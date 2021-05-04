package Buffer
/*
 * This code is a minimal hardware described in Chisel.
 *
 * Buffer to load picture from memory
 */

//import Chisel._
import patmos.Constants._
import chisel3._
import chisel3.util._
import ocp._
import chisel3.util.experimental.loadMemoryFromFile


//need to be a modulo, to use the mem blocks instead of FF
class EmbMem extends Module{
    val io = IO(new Bundle() {
    val writeEn = Input(Bool()) 
    val writeAddr = Input(UInt(10.W)) 
    val rd_addr = Input(UInt(10.W)) 
    val writeData =Input (UInt (32.W))
    val readData =Output (UInt (8.W))
  })
  val LINE_WIDTH = 640
  val memory = SyncReadMem(LINE_WIDTH/4, UInt(32.W))
  when(io.writeEn){
    memory.write(io.writeAddr, io.writeData)
  }

  //OCP sends the lowest address as highest byte
  io.readData:=0.U
  switch (io.rd_addr(1,0)) {
    is(3.U){
      io.readData:= memory.read(io.rd_addr>>2)(7, 0)
    }
    is(2.U){
      io.readData:= memory.read(io.rd_addr>>2)(15, 8)
    }
    is(1.U){
      io.readData:= memory.read(io.rd_addr>>2)(23, 16)
    }
    is(0.U){
      io.readData:= memory.read(io.rd_addr>>2)(31, 24)
    }
  }
}


class Buffer extends Module {
  val LINE_WIDTH = 640
  val DISPLAY_HEIGTH = 480

  val VGA_MEM_BASE_ADDR = 400000.U

  val request :: read :: done :: Nil = Enum(3)

  

  val io = IO(new Bundle() {
    //VGA controller IO
    val red = Output(UInt(8.W))
    val green = Output(UInt(8.W))
    val blue = Output(UInt(8.W))

    val line_cnt = Input(UInt(10.W))
    val rd_addr = Input(UInt(10.W)) //log(LINE_WIDTH)

    val memPort = new OcpBurstMasterPort(EXTMEM_ADDR_WIDTH, DATA_WIDTH, BURST_LENGTH)
  })

  val memory = Module (new EmbMem())
  val pixel_reg = RegInit(0.U(8.W))
  val ocpState = RegInit(request)
  val ocpWordCnt = RegInit(0.U(3.W))
  val ocpBuffAddr = RegInit(0.U(11.W))
  val ocpLineCnt = RegInit(0.U(10.W))


  
  //VGA controller read
    memory.io.rd_addr:=io.rd_addr
    pixel_reg := memory.io.readData

    io.red := pixel_reg(1,0)  <<   (pixel_reg(7,6)<<1)
    io.green := pixel_reg(3,2)<< (pixel_reg(7,6)<<1)
    io.blue := pixel_reg(5,4) <<  (pixel_reg(7,6)<<1)



  //Read from Memory via OCP

  io.memPort.M.Cmd := OcpCmd.IDLE
  io.memPort.M.Data := 0.U(21.W)
  io.memPort.M.DataValid := 0.U
  io.memPort.M.DataByteEn := 0.U(1.W)

  io.memPort.M.Addr := VGA_MEM_BASE_ADDR + ocpLineCnt * LINE_WIDTH.U + ocpBuffAddr

  memory.io.writeEn:= false.B
  memory.io.writeData:= 0.U
  memory.io.writeAddr := 0.U
  switch(ocpState) {
    is(request) {
      io.memPort.M.Cmd := OcpCmd.RD

      when(io.memPort.S.CmdAccept === true.B) {
        ocpState := read
      }
    }

    is(read) {
      ocpWordCnt := 0.U;
      when(io.memPort.S.Resp === OcpResp.DVA) {

        when(ocpBuffAddr === (LINE_WIDTH.U - 4.U)) { //reset the counter (-4 because we read 4 byte on one burst)
          ocpState := done
        }
          .otherwise {
            when((ocpWordCnt) === 3.U) { //ocpWordCnt % 4
              ocpState := request
            }
          }

        ocpWordCnt := ocpWordCnt + 1.U
        memory.io.writeEn:= true.B
        memory.io.writeData:=io.memPort.S.Data
        memory.io.writeAddr:=ocpBuffAddr>>2

        ocpBuffAddr := ocpBuffAddr + 4.U
      }

      when(io.memPort.S.Resp === OcpResp.ERR || io.memPort.S.Resp ===OcpResp.FAIL) {
        ocpState := done
      }
    }

    is(done) { //wait until next line is ready to buffer
      ocpWordCnt := 0.U

      ocpBuffAddr := 0.U 

      when(io.rd_addr === LINE_WIDTH.U) {
        ocpState := request
        ocpLineCnt := io.line_cnt + 1.U
        when(ocpLineCnt === DISPLAY_HEIGTH.U) {
          ocpLineCnt := 0.U
        }
      }
    }
  }
}

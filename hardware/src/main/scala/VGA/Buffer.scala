package Buffer
/*
 * This code is a minimal hardware described in Chisel.
 *
 * Buffer to load picture from memory
 */

//import Chisel._
import patmos.Constants._
import chisel3._
import chisel3.Driver
import chisel3.util._
import ocp._

class Buffer extends Module {
  val LINE_WIDTH = 640
  val DISPLAY_HEIGTH = 480

  val VGA_MEM_BASE_ADDR = 400000.U
  //MCmd Values
  // val OCP_IDLE = 0.U(3.W)
  // val OCP_WR = 1.U(3.W)
  // val OCP_RD = 2.U(3.W)
  // SResp values
  // val NULL = 0.U(2.W)
  // val DVA = 1.U(2.W)
  // val FAIL = 2.U(2.W)
  // val ERR = 3.U(2.W)

  val request :: read :: done :: Nil = Enum(3)

  val memory = SyncReadMem(LINE_WIDTH, UInt(8.W))

  val io = IO(new Bundle() {
    //VGA controller IO
    val red = Output(UInt(8.W))
    val green = Output(UInt(8.W))
    val blue = Output(UInt(8.W))

    val line_cnt = Input(UInt(10.W))
    val rd_addr = Input(UInt(10.W)) //log(LINE_WIDTH)

    //Memory IO

    //val memPort = new OcpBurstMasterPort(32, 32, BURST)
    val memPort = new OcpBurstMasterPort(EXTMEM_ADDR_WIDTH, DATA_WIDTH, BURST_LENGTH)
  })

  //fill memory with random colors

  val color = 48.U(8.W)
  val cnt = RegInit(0.U(11.W))

  val ocpState = RegInit(request)
  val ocpWordCnt = RegInit(0.U(3.W))

  val ocpBuffAddr = RegInit(0.U(11.W))

  val ocpLineCnt = RegInit(0.U(10.W))

  when(cnt < LINE_WIDTH.U) {

    when((cnt >> 3) % 2.U === 0.U) {
      memory.write(cnt, color)
    }
      .otherwise {
        memory.write(cnt, color >> 2)
      }
    cnt := cnt + 1.U
  }

  //Read for VGA controller
    io.red := memory.read(io.rd_addr)(1, 0)<<6
    io.green := memory.read(io.rd_addr)(3, 2)<<6
    io.blue := memory.read(io.rd_addr)(5, 4)<<6



  //Read from Memory via OCP

  io.memPort.M.Cmd := OcpCmd.IDLE
  io.memPort.M.Data := 0.U(21.W)
  io.memPort.M.DataValid := 0.U
  io.memPort.M.DataByteEn := 0.U(1.W)

  io.memPort.M.Addr := VGA_MEM_BASE_ADDR + ocpLineCnt * LINE_WIDTH.U + ocpBuffAddr

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

        when(ocpBuffAddr === (LINE_WIDTH.U - 4.U)) { //reset the counter (-2 because in the last step we write two adresses and we start with 0)
          ocpState := done
        }
          .otherwise {
            when((ocpWordCnt) === 3.U) { //ocpWordCnt % 4
              ocpState := request
            }
          }

        ocpWordCnt := ocpWordCnt + 1.U
        //split write from OCP to Buffer in 3 states
        memory.write(ocpBuffAddr + 3.U, io.memPort.S.Data(7, 0))
        memory.write(ocpBuffAddr + 2.U, io.memPort.S.Data(15, 8))
        memory.write(ocpBuffAddr + 1.U, io.memPort.S.Data(23, 16))
        memory.write(ocpBuffAddr + 0.U, io.memPort.S.Data(31, 24))
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

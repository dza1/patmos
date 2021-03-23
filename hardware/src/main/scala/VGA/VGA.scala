package io
/*
 * This code is a minimal hardware described in Chisel.
 * 
 * VGA controller: just show a static picture
 */

import patmos.Constants._
import chisel3._
import chisel3.Driver


class VGA extends Module {
  val io = IO(new Bundle() with patmos.HasPins {
    val pins = new Bundle() {
    val red = Output(UInt(8.W))
    val green = Output(UInt(8.W))
    val blue = Output(UInt(8.W))
    val hs = Output(UInt(1.W))
    val vs = Output(UInt(1.W))
    val blank = Output(UInt(1.W))
    val sync = Output(UInt(1.W))
    val clock = Output(UInt(1.W))
    }
  })
  val CNT_MAX = (50000000 / 2 - 1).U;
  val HS_COUNT = (800-1).U;
  val VS_COUNT = (521-1).U;
  val VS_LEN = (96-1).U;
  val HS_LEN = (2-1).U;

  val HS_START = (144-1).U;
  val HS_STOP = (784-1).U;

  val VS_START = (31-1).U;
  val VS_STOP = (511-1).U;

  io.pins.red := 139.U(8.W)
  io.pins.green :=0.U(8.W)
  io.pins.blue := 139.U(8.W)
  io.pins.sync:=0.U


  val cntReg = RegInit(0.U(32.W))


  val hsReg = RegInit(0.U(1.W))
  val vsReg = RegInit(0.U(1.W))
  val blankReg = RegInit(1.U(1.W))

  val clkDev =RegInit(0.U(2.W))
  val lineCountReg =RegInit(0.U(10.W))
  val pixelCountReg =RegInit(0.U(10.W))

  clkDev := clkDev+ 1.U
  when (clkDev===2.U) {
    pixelCountReg:=pixelCountReg+1.U

    //Horizontal
    when(pixelCountReg === VS_LEN && lineCountReg >= 2.U) {
      hsReg := 1.U
    }
    when(pixelCountReg === HS_COUNT) {
      pixelCountReg := 0.U
      hsReg := 0.U
      lineCountReg := lineCountReg + 1.U
      //Vertical
      when(lineCountReg === VS_COUNT) {
        lineCountReg := 0.U
        vsReg := 0.U
      }
      when(lineCountReg === HS_LEN) {
        vsReg := 1.U
      }
    }
    when(pixelCountReg===HS_START && lineCountReg>VS_START && lineCountReg<=VS_STOP){ //disable blank
      blankReg:=1.U
    }
    when(pixelCountReg===HS_STOP) { //enable blank
      blankReg:=0.U
    }

    clkDev:=0.U


  }
  cntReg := cntReg + 1.U
  when(cntReg === CNT_MAX) {
    cntReg := 0.U
  }
  io.pins.hs := hsReg
  io.pins.vs := vsReg
  io.pins.clock:= cntReg
  io.pins.blank:= blankReg
}

// /**
//  * An object extending App to generate the Verilog code.
//  */
// object VGA extends App {
//   (new chisel3.stage.ChiselStage).emitVerilog(new VGA())
// }




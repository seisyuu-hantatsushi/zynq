`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 02/12/2026 05:23:49 PM
// Design Name: 
// Module Name: SIM_LED_COUNTER
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module SIM_LED_COUNTER(
  );

  logic clk;
  logic	rst;
  logic	cnt_up;
  logic	[1:0] o_led;
  
  initial begin
    clk = 1'b0;
    forever #10 clk = ~clk;
  end

  initial begin
    rst = 1'b0;
    #100;
    rst = 1'b1;
  end

  initial begin
    cnt_up = 1'b0;
    forever begin
      cnt_up = ~cnt_up;
      #1000;
      cnt_up = ~cnt_up;
      #1000;
    end 
  end
  
  // DUT
  LED_COUNTER# () 
  LED_COUNTER_U0 (
		  .i_clk(clk),
		  .i_rst(rst),
		  .i_cnt_up(cnt_up),
		  .o_led(o_led)
  );

endmodule

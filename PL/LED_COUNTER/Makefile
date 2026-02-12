recreate: led_conter/led_counter.xpr

led_conter/led_counter.xpr: LED_COUNTER.tcl
	vivado -mode batch -source LED_COUNTER.tcl

clean:
	rm -rf .Xil led_counter *.log *.jou

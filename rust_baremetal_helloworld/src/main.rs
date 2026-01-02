#![no_std]
#![no_main]

extern crate panic_halt;

const UART0_BASE_ADDRESS: usize=0xE000_0000;

mod start;

#[unsafe(no_mangle)]
pub extern "C" fn main() -> ! {
    loop {
	 core::hint::spin_loop();
    }
}

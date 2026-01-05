#![no_std]
#![no_main]

// KEY1 MIO0, KEY2 MIO12
// LED1 MIO8, LED2 MIO7

extern crate panic_halt;

const UART0_BASE_ADDRESS: u32 = 0xE000_0000;

const GPIO_BASE_ADDRESS: u32 = 0xE000_A000;
const GPIO_DATA0_OFFSET: u32 = 0x0000_0040;
const GPIO_DATA1_OFFSET: u32 = 0x0000_0044;
const GPIO_DIRM0_OFFSET: u32 = 0x0000_0204;
const GPIO_OEN0_OFFSET:  u32 = 0x0000_0208;
const GPIO_DIRM1_OFFSET: u32 = 0x0000_0244;
const GPIO_OEN1_OFFSET:  u32 = 0x0000_0248;

const SLCR_BASE_ADDRESS: u32  = 0xF800_0000;
const SLCR_APER_CLK_CTRL_OFFSET: u32 = 0x012C;
const SLCR_GPIO_RST_CTRL_OFFSET: u32 = 0x022C;

const SLCR_APER_CLK_CTRL_GPIO_CPU_1XCLKACT:u32  = 0x1 << 22;
const SLCR_APER_CLK_CTRL_UART1_CPU_1XCLKACT:u32 = 0x1 << 21;
const SLCR_APER_CLK_CTRL_UART0_CPU_1XCLKACT:u32 = 0x1 << 20;

mod start;

fn regbase_offset_bit_or(base:u32, offset:u32, val:u32){
    let reg = (base + offset) as *mut u32;
    unsafe {
	let reg_val = core::ptr::read_volatile(reg);
	core::ptr::write_volatile(reg, reg_val | val);
    }
}

fn regbase_offset_bit_and(base:u32, offset:u32, val:u32){
    let reg = (base + offset) as *mut u32;
    unsafe {
	let reg_val = core::ptr::read_volatile(reg);
	core::ptr::write_volatile(reg, reg_val & val);
    }
}

fn register_bit_or(addr:u32, val:u32){
    let reg = addr as *mut u32;
    unsafe {
	let reg_val = core::ptr::read_volatile(reg);
	core::ptr::write_volatile(reg, reg_val | val);
    }
}

fn register_bit_and(addr:u32, val:u32){
    let reg = addr as *mut u32;
    unsafe {
	let reg_val = core::ptr::read_volatile(reg);
	core::ptr::write_volatile(reg, reg_val & val);
    }
}

fn regisiter_read(addr:u32) -> u32 {
    let reg = addr as *mut u32;
    let val = unsafe {
	core::ptr::read_volatile(reg)
    };
    return val;
}

fn gpio_ctrl_init() {
    // reset gpio ctrl
    regbase_offset_bit_or(SLCR_BASE_ADDRESS, SLCR_GPIO_RST_CTRL_OFFSET, 0x0000_0001);
    // enable GPIO AMBA Clock Control
    regbase_offset_bit_or(SLCR_BASE_ADDRESS, SLCR_APER_CLK_CTRL_OFFSET, SLCR_APER_CLK_CTRL_GPIO_CPU_1XCLKACT);
}

// dir: true -> output, false -> input
fn gpio_cfg_pin(pin_code:u32, dir:bool) -> Result<(), u32>{

    let (dirm_reg, oen_reg, shift) = if pin_code < 32 {
	let dirm_reg = GPIO_BASE_ADDRESS + GPIO_DIRM0_OFFSET;
	let oen_reg  = GPIO_BASE_ADDRESS + GPIO_OEN0_OFFSET;
	let shift    = pin_code;
	(dirm_reg, oen_reg, shift)
    } else if pin_code < 54 {
	let dirm_reg = GPIO_BASE_ADDRESS + GPIO_DIRM1_OFFSET;
	let oen_reg  = GPIO_BASE_ADDRESS + GPIO_OEN1_OFFSET;
	let shift    = pin_code-32;
	(dirm_reg, oen_reg, shift)
    } else {
	return Err(1);
    };

    if dir {
	// output
	register_bit_or(dirm_reg, 0x01 << shift);
	register_bit_or(oen_reg,  0x01 << shift);
    } else {
	// input
	register_bit_and(dirm_reg, !(0x01 << shift));
	register_bit_and(oen_reg,  !(0x01 << shift));
    };
    
    return Ok(())
}

fn gpio_write(pin_code:u32, data:bool) -> Result<(), u32> {
    let (data_reg, shift) = if pin_code < 32 {
	let data_reg = GPIO_BASE_ADDRESS + GPIO_DATA0_OFFSET;
	let shift    = pin_code;
	(data_reg, shift)
    } else if pin_code < 54 {
	let data_reg = GPIO_BASE_ADDRESS + GPIO_DATA1_OFFSET;
	let shift    = pin_code-32;
	(data_reg, shift)
    } else {
	return Err(1);
    };

    if data {
	// set
	register_bit_or(data_reg, 0x01 << shift);
    } else {
	// clear
	register_bit_and(data_reg, !(0x01 << shift));
    }
    
    return Ok(())
}

fn gpio_read(pin_code:u32) -> Result<bool, u32> {
    let (data_reg, shift) = if pin_code < 32 {
	let data_reg = GPIO_BASE_ADDRESS + GPIO_DATA0_OFFSET;
	let shift    = pin_code;
	(data_reg, shift)
    } else {
	let data_reg = GPIO_BASE_ADDRESS + GPIO_DATA1_OFFSET;
	let shift    = pin_code-32;
	(data_reg, shift)
    };

    Ok((regisiter_read(data_reg) & (0x01 << shift)) != 0x00)
}

#[unsafe(no_mangle)]
pub extern "C" fn main() -> ! {

    gpio_ctrl_init();
    if let Err(_) = gpio_cfg_pin(7, true) {
	panic!();
    };
    if let Err(_) = gpio_cfg_pin(8, true) {
	panic!();
    };

    if let Err(_) = gpio_cfg_pin(0, false) {
	panic!();
    };
    if let Err(_) = gpio_cfg_pin(12, false) {
	panic!();
    };
    
    
    if let Err(_) = gpio_write(7, true) {
	panic!();
    };

    let mut prev = gpio_read(0).unwrap();
    let mut on = false;
    let _ =  gpio_write(8, on);
    loop {
	if let Ok(val) = gpio_read(12) {
	    let _ = gpio_write(7, val);
	};
	if let Ok(now) = gpio_read(0) {
	    if now != prev && now {
		on = !on;
		let _ =  gpio_write(8, on);
	    }
	    prev = now;
	}
	core::hint::spin_loop();
    }
}

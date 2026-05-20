use esp_idf_hal::i2c::*;
use esp_idf_hal::peripherals::Peripherals; // Prelude is gone, import directly
use esp_idf_hal::units::FromValueType;     // This helps with .kHz()
use esp_idf_hal::delay::BLOCK;

fn main() -> anyhow::Result<()> {
    esp_idf_svc::sys::link_patches();
    esp_idf_svc::log::EspLogger::initialize_default();

    let peripherals = Peripherals::take()?;

    // 1. Fixed the numeric type (100u32) and ensured units are in scope
    let config = I2cConfig::new().baudrate(100_u32.kHz().into());

    let mut i2c = I2cDriver::new(
        peripherals.i2c0,
        peripherals.pins.gpio8,
        peripherals.pins.gpio9,
        &config,
    )?;

    log::info!("Starting I2C Bus Scan...");

    for addr in 1..128 {
        // Attempt a 0-byte write to the address
        let res = i2c.write(addr, &[], BLOCK);

        match res {
            Ok(_) => {
                log::info!("Found device at address: 0x{:02x}", addr);
            }
            Err(_) => {
                // Ignore errors (no device found)
            }
        }
    }

    log::info!("Scan Complete.");
    loop {
        esp_idf_hal::delay::FreeRtos::delay_ms(1000);
    }
}
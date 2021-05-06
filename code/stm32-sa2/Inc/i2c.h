// This code sucks.
// I have NO idea how to properly handle I2C errors.
// And literally hours of googling did not get me anywhere.
// Just use HAL. Even stable LL examples are unavailable.
// Please don't use this I2C library ever, just use HAL
// Author: sx107

#include "main.h"

#ifndef I2C_H_
#define I2C_H_

#define I2C_TMO_VAL 50000

#define I2C_ERR_BERR 	(0b1 << 0)
#define I2C_ERR_ARLO 	(0b1 << 1)
#define I2C_ERR_AF 		(0b1 << 2)
#define I2C_ERR_OVR 	(0b1 << 3)
#define I2C_ERR_PEC 	(0b1 << 4)
#define I2C_ERR_TMO		(0b1 << 5) // This is NOT bit 14 SMBus timeout error!

void i2c_init();
void i2c_deinit();

// Error handling
void I2C1_ER_IRQHandler(void);
void i2c_reset_error();
uint8_t i2c_get_error();
uint8_t i2c_waitBit(uint32_t bit);

// Main write-read functions
uint8_t i2c_write(uint8_t addr, uint16_t sz, uint8_t* buf);
uint8_t i2c_read(uint8_t addr, uint16_t sz, uint8_t* buf);

// Other read-write functions
uint8_t i2c_write8(uint8_t addr, uint8_t byte);
uint8_t i2c_read8(uint8_t addr);
uint8_t i2c_readRegister(uint8_t addr, uint8_t reg, uint16_t sz, uint8_t* buf);
uint8_t i2c_readRegister8(uint8_t addr, uint8_t reg);
uint8_t i2c_writeRegister8(uint8_t addr, uint8_t reg, uint8_t val);

#endif


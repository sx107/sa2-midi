// This code sucks.
// I have NO idea how to properly handle I2C errors.
// And literally hours of googling did not get me anywhere.
// Just use HAL. Even stable LL examples are unavailable.
// Please don't use this I2C library ever, just use HAL
// Author: sx107

#include "i2c.h"
#include "main.h"

volatile uint8_t _i2c_error = 0;

// I2C init
void i2c_init() {
	// I2C @ pins PB6, PB7.
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
	MODIFY_REG(GPIOB->CRL, GPIO_CRL_CNF6_Msk | GPIO_CRL_MODE6_Msk, (3 << GPIO_CRL_CNF6_Pos) | (3 << GPIO_CRL_MODE6_Pos));
	MODIFY_REG(GPIOB->CRL, GPIO_CRL_CNF7_Msk | GPIO_CRL_MODE7_Msk, (3 << GPIO_CRL_CNF7_Pos) | (3 << GPIO_CRL_MODE7_Pos));

	NVIC_SetPriority(I2C1_ER_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
	NVIC_EnableIRQ(I2C1_ER_IRQn);

	// Enable I2C clock
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
	I2C1->OAR2 &= ~I2C_OAR2_ENDUAL; // Disable dual address
	// No global call, enable clock stretch, disable I2C
	MODIFY_REG(I2C1->CR1, I2C_CR1_ENGC | I2C_CR1_NOSTRETCH | I2C_CR1_PE, 0);

	MODIFY_REG(I2C1->CR2, I2C_CR2_FREQ, 36);  // 36 MHz APB1 Clock
	MODIFY_REG(I2C1->TRISE, I2C_TRISE_TRISE, 11); // 300 nS TRise time, (1/36MHz)*10
	MODIFY_REG(I2C1->CCR, (I2C_CCR_FS | I2C_CCR_DUTY | I2C_CCR_CCR), 30UL | I2C_CCR_FS); // 400kHz speed

	MODIFY_REG(I2C1->CR1, I2C_CR1_SMBUS | I2C_CR1_SMBTYPE | I2C_CR1_ENARP, 0); // No SMBUS
	MODIFY_REG(I2C1->OAR1, 0x3FF | I2C_OAR1_ADDMODE_Msk, 1 << 14); // Bit 14 has to be always kept 1. WHY, STM, W H Y?! ADDR = 0, 7-bit

	I2C1->CR1 |= I2C_CR1_PE;
	I2C1->CR1 |= I2C_CR1_ACK;
	//MODIFY_REG(I2C1->OAR2, I2C_OAR2_ADD2_Msk, 0); // ENDUAL already disabled

	I2C1->CR2 |= I2C_CR2_ITERREN;
	for(uint8_t i = 0; i < 0xFE; i++) {}
}

void i2c_deinit() {
	I2C1->SR1 &= ~(0b11011111 << 8);
	I2C1->CR2 &= ~I2C_CR2_ITERREN;
	I2C1->CR1 &= ~I2C_CR1_PE;
}

// I2C Error IRQ
void I2C1_ER_IRQHandler(void)
{
	_i2c_error = ((I2C1->SR1) >> 8) & 0b11111;
	// SmBus errors are just ignored
	I2C1->SR1 &= ~(0b11011111 << 8);
}

// Resets I2C, Sends stop condition, etc.
void i2c_handle_error() {
	if(
		(_i2c_error & I2C_ERR_AF) == I2C_ERR_AF ||
		(_i2c_error & I2C_ERR_TMO) == I2C_ERR_TMO) {
		I2C1->CR1 |= I2C_CR1_STOP;
		(void) I2C1->SR1; (void) I2C1->SR2;
		return;
	}
	return;
}

// Waits for a certain bit in I2C1->SR1 with a timeout
inline uint8_t i2c_waitBit(uint32_t bit) {
	_i2c_error = 0;
	uint16_t timeOut = 0;
	while(((I2C1->SR1) & bit) != bit) {
		if((timeOut++ > I2C_TMO_VAL) && (I2C_TMO_VAL != 0)) {
			_i2c_error |= I2C_ERR_TMO;
		}
		if(_i2c_error != 0) {
			i2c_handle_error();
			return _i2c_error;
		}
	}
	return 0;
}

// Functions to give access to _i2c_error from outside
void i2c_reset_error() {
	_i2c_error = 0;
}

uint8_t i2c_get_error() {
	return _i2c_error;
}


// Main write function
uint8_t i2c_write(uint8_t addr, uint16_t sz, uint8_t* buf) {
	// Return if any error occurred earlier immediately, wait while I2C is busy
	if(_i2c_error != 0) {i2c_handle_error(); return _i2c_error;}
	WAITNBIT(I2C1->SR2, I2C_SR2_BUSY);
	// Enable ACK
	MODIFY_REG(I2C1->CR1, I2C_CR1_POS | I2C_CR1_ACK, I2C_CR1_ACK);

	// Send start condition
	I2C1->CR1 |= I2C_CR1_START;
	_i2c_error = 0;
	WAITBIT(I2C1->SR1, I2C_SR1_SB);
	(void) I2C1->SR1;

	// Send address
	MODIFY_REG(I2C1->DR, I2C_DR_DR, addr << 1);
	RETERR(i2c_waitBit(I2C_SR1_ADDR), _i2c_error);
	(void) I2C1->SR1; (void) I2C1->SR2;

	// Send data
	for(uint16_t i = 0; i < sz; i++) {
		MODIFY_REG(I2C1->DR, I2C_DR_DR, buf[i]);
		RETERR(i2c_waitBit(I2C_SR1_TXE), _i2c_error);
	}

	// Send stop condition
	I2C1->CR1 |= I2C_CR1_STOP;
	//WAITNBIT(I2C1->SR1, I2C_SR1_STOPF);
	return 0;
}

// Main read function
uint8_t i2c_read(uint8_t addr, uint16_t sz, uint8_t* buf) {
	if(_i2c_error != 0) {i2c_handle_error(); return _i2c_error;}
	WAITNBIT(I2C1->SR2, I2C_SR2_BUSY);
	// TODO: Rewrite according to datasheet
	// Enable ACK
	MODIFY_REG(I2C1->CR1, I2C_CR1_POS | I2C_CR1_ACK, I2C_CR1_ACK);

	// Send start condition
	I2C1->CR1 |= I2C_CR1_START;
	WAITBIT(I2C1->SR1, I2C_SR1_SB);
	(void) I2C1->SR1;

	// Send address
	MODIFY_REG(I2C1->DR, I2C_DR_DR, (addr << 1) | 0b1);
	//WAITBIT(I2C1->SR1, I2C_SR1_ADDR);
	RETERR(i2c_waitBit(I2C_SR1_ADDR), _i2c_error);

	(void) I2C1->SR1; (void) I2C1->SR2;

	// Send data
	for(uint16_t i = 0; i < (sz-1); i++) {
		WAITBIT(I2C1->SR1, I2C_SR1_RXNE);
		buf[i] = I2C1->DR & I2C_DR_DR;
	}

	// Disable ACK, send stop immedeately after this byte
	I2C1->CR1 &= ~I2C_CR1_ACK;
	I2C1->CR1 |= I2C_CR1_STOP;
	WAITBIT(I2C1->SR1, I2C_SR1_RXNE);

	buf[sz-1] = I2C1->DR & I2C_DR_DR;
	return 0;
}

// Alternative read-write functions
uint8_t i2c_write8(uint8_t addr, uint8_t byte) {
	return i2c_write(addr, 1, &byte);
}

uint8_t i2c_read8(uint8_t addr) {
	uint8_t retVal;
	i2c_read(addr, 1, &retVal);
	return retVal;
}

uint8_t i2c_readRegister(uint8_t addr, uint8_t reg, uint16_t sz, uint8_t* buf) {
	RETERR(i2c_write8(addr, reg), _i2c_error);
	//for(uint8_t i = 0; i < 0xFF; i++) {}
	return i2c_read(addr, sz, buf);
}

uint8_t i2c_readRegister8(uint8_t addr, uint8_t reg) {
	RETERR(i2c_write8(addr, reg), _i2c_error);
	//for(uint8_t i = 0; i < 0xFF; i++) {}
	return i2c_read8(addr);
}

uint8_t i2c_writeRegister8(uint8_t addr, uint8_t reg, uint8_t val) {
	uint8_t sndBuf[] = {reg, val};
	return i2c_write(addr, 2, sndBuf);
}


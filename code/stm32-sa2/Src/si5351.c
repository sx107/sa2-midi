/*
 * si5351.h
 *
 *  Created on: Mar 19, 2021
 *      Author: Sx107
 *      Origin: Si5351 library by adafruit (https://github.com/adafruit/Adafruit_Si5351_Library),
 *      rewritten for STM32 in C and made as lightweight as possible
 *      Additional options also are added
 *      Supports only 3 outputs (TSSOP-10 package)
 */

// This code uses i2c.h, i2c.c - probably the worst I2C library ever.
// Just rewrite all i2c_xxx functions to HAL to use it

#include "si5351.h"
#include "i2c.h"
#include "si5351_regmap.h"

uint8_t _si5351_initialized = 0;

si5351_err_t si5351_init(si5351CrystalLoad_t crystalLoad) {
	i2c_writeRegister8(SI5351_ADDR, SI5351_REG_OUTPUT_ENABLE_CONTROL, 0xFF); // Disable all clocks outputs
	uint8_t sndBuf[9] = {SI5351_REG_CLK0_CONTROL, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80};
	i2c_write(SI5351_ADDR, 9, sndBuf); // Powerdown all clocks
	i2c_writeRegister8(SI5351_ADDR, SI5351_REG_CRYSTAL_INTERNAL_LOAD_CAPACITANCE, crystalLoad);

	SI5351_ASSERT(i2c_get_error() == 0, SI5351_ERR_I2C);

	_si5351_initialized = 1;
	return SI5351_ERR_NONE;
};

si5351_err_t si5351_setupPLL(si5351PLL_t pll, uint8_t mult, uint32_t num, uint32_t denom) {
	  /*
	   * where: a = mult, b = num and c = denom
	   * P1 register is an 18-bit value using following formula:
	   * 	P1[17:0] = 128 * mult + floor(128*(num/denom)) - 512
	   * P2 register is a 20-bit value using the following formula:
	   * 	P2[19:0] = 128 * num - denom * floor(128*(num/denom))
	   * P3 register is a 20-bit value using the following formula:
	   * 	P3[19:0] = denom
	   */
	SI5351_ASSERT(_si5351_initialized != 0, SI5351_ERR_NOTINIT);
	SI5351_ASSERT((mult > 14) && (mult < 91), SI5351_ERR_INVALIDPARR);
	SI5351_ASSERT(denom != 0, SI5351_ERR_INVALIDPARR);
	SI5351_ASSERT(num <= 0xFFFFF, SI5351_ERR_INVALIDPARR);
	SI5351_ASSERT(denom <= 0xFFFFF, SI5351_ERR_INVALIDPARR);

	uint32_t P1; /* PLL config register P1 */
	uint32_t P2; /* PLL config register P2 */
	uint32_t P3; /* PLL config register P3 */

	if(num == 0) {
		P1 = 128 * mult - 512;
		P2 = 0;
		P3 = 1;
	} else if (denom == 1) {
		P1 = 128 * mult + 128 * num - 512;
		P2 = 128 * num - 128;
		P3 = 1;
	} else {
		P1 = (uint32_t)(128 * mult + (128 * num) / denom - 512);
		P2 = (uint32_t)(128 * num - denom * ((num * 128) / denom));
		P3 = denom;
	}

	uint8_t sndBuf[9] = {((pll == SI5351_PLL_A) ? SI5351_REG_PLLA_P0 : SI5351_REG_PLLB_P0),
		(P3 & 0x0000FF00) >> 8,
		(P3 & 0x000000FF),
		(P1 & 0x00030000) >> 16,
		(P1 & 0x0000FF00) >> 8,
		(P1 & 0x000000FF),
		((P3 & 0x000F0000) >> 12) | ((P2 & 0x000F0000) >> 16),
		(P2 & 0x0000FF00) >> 8,
		(P2 & 0x000000FF)
	};

	i2c_write(SI5351_ADDR, 9, sndBuf);
	for(uint16_t i = 0; i < 0x200; i++) {}
	i2c_writeRegister8(SI5351_ADDR, SI5351_REG_PLL_RESET, (pll == SI5351_PLL_A) ? (1 << 5) : (1 << 7));
	SI5351_ASSERT(i2c_get_error() == 0, SI5351_ERR_I2C);
	return SI5351_ERR_NONE;
};

si5351_err_t si5351_setMultisynthControl(uint8_t output, si5351MultisynthEn_t en, si5351PLL_t pllsource, si5351MultisynthInv_t inv, si5351MultisynthMode_t mode, si5351OutputDrive_t drv) {
	SI5351_ASSERT(_si5351_initialized != 0, SI5351_ERR_NOTINIT);
	SI5351_ASSERT(output < 3, SI5351_ERR_INVALIDPARR);
	i2c_writeRegister8(SI5351_ADDR, SI5351_REG_CLK0_CONTROL + output, 0b00001100 | en | pllsource | inv | mode | drv);
	SI5351_ASSERT(i2c_get_error() == 0, SI5351_ERR_I2C);
	return SI5351_ERR_NONE;
}

si5351_err_t si5351_setupMultisynth(uint8_t output, uint32_t div, uint32_t num, uint32_t denom, si5351RDiv_t rdiv) {
	  /* Output Multisynth Divider Equations
	   *
	   * where: a = div, b = num and c = denom
	   * P1 register is an 18-bit value using following formula:
	   * 	P1[17:0] = 128 * a + floor(128*(b/c)) - 512
	   * P2 register is a 20-bit value using the following formula:
	   * 	P2[19:0] = 128 * b - c * floor(128*(b/c))
	   * P3 register is a 20-bit value using the following formula:
	   * 	P3[19:0] = c
	   */

	SI5351_ASSERT(_si5351_initialized != 0, SI5351_ERR_NOTINIT);
	SI5351_ASSERT(div > 3, SI5351_ERR_INVALIDPARR);
	SI5351_ASSERT(div < 901, SI5351_ERR_INVALIDPARR);
	SI5351_ASSERT(denom != 0, SI5351_ERR_INVALIDPARR);
	SI5351_ASSERT(num <= 0xFFFFF, SI5351_ERR_INVALIDPARR);
	SI5351_ASSERT(denom <= 0xFFFFF, SI5351_ERR_INVALIDPARR);
	SI5351_ASSERT(output < 3, SI5351_ERR_INVALIDPARR);

	uint32_t P1; /* PLL config register P1 */
	uint32_t P2; /* PLL config register P2 */
	uint32_t P3; /* PLL config register P3 */

	if(num == 0) {
		P1 = 128 * div - 512;
		P2 = 0;
		P3 = 1;
	} else if (denom == 1) {
		P1 = 128 * div + 128 * num - 512;
		P2 = 128 * num - 128;
		P3 = 1;
	} else {
		P1 = (uint32_t)(128 * div + (128 * num) / denom - 512);
		P2 = (uint32_t)(128 * num - denom * ((num * 128) / denom));
		P3 = denom;
	}

	uint8_t sndBuf[9] = {SI5351_REG_MULTISYNTH0_P0 + 8 * output,
		(P3 & 0x0000FF00) >> 8,
		(P3 & 0x000000FF),
		((P1 & 0x00030000) >> 16) | rdiv,
		(P1 & 0x0000FF00) >> 8,
		(P1 & 0x000000FF),
		((P3 & 0x000F0000) >> 12) | ((P2 & 0x000F0000) >> 16),
		(P2 & 0x0000FF00) >> 8,
		(P2 & 0x000000FF)
	};

	i2c_write(SI5351_ADDR, 9, sndBuf);
	SI5351_ASSERT(i2c_get_error() == 0, SI5351_ERR_I2C);
	return SI5351_ERR_NONE;
}

si5351_err_t si5351_enableOutput(uint8_t output) {
	SI5351_ASSERT(_si5351_initialized != 0, SI5351_ERR_NOTINIT);
	SI5351_ASSERT(output < 3, SI5351_ERR_INVALIDPARR);
	uint8_t reg = i2c_readRegister8(SI5351_ADDR, 3);
	reg &= ~(1 << output);
	i2c_writeRegister8(SI5351_ADDR, 3, reg);
	SI5351_ASSERT(i2c_get_error() == 0, SI5351_ERR_I2C);
	return SI5351_ERR_NONE;
}

si5351_err_t si5351_disableOutput(uint8_t output) {
	SI5351_ASSERT(_si5351_initialized != 0, SI5351_ERR_NOTINIT);
	SI5351_ASSERT(output < 3, SI5351_ERR_INVALIDPARR);
	uint8_t reg = i2c_readRegister8(SI5351_ADDR, 3);
	reg |= (1 << output);
	i2c_writeRegister8(SI5351_ADDR, 3, reg);
	SI5351_ASSERT(i2c_get_error() == 0, SI5351_ERR_I2C);
	return SI5351_ERR_NONE;
}

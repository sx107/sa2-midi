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

#ifndef SI5351_H_
#define SI5351_H_

#include "main.h"

#define SI5351_ADDR 0x60 // Not 0xC0, not shifted to the left.

typedef enum {
	SI5351_ERR_NONE,
	SI5351_ERR_NOTINIT,
	SI5351_ERR_INVALIDPARR,
	SI5351_ERR_I2C
} si5351_err_t;

#define SI5351_ASSERT(X, ERR) if(!(X)) {return ERR;}

typedef enum {
  SI5351_PLL_A = (0 << 5),
  SI5351_PLL_B = (1 << 5),
} si5351PLL_t;

typedef enum {
  SI5351_CRYSTAL_LOAD_6PF = (1 << 6),
  SI5351_CRYSTAL_LOAD_8PF = (2 << 6),
  SI5351_CRYSTAL_LOAD_10PF = (3 << 6)
} si5351CrystalLoad_t;

typedef enum {
	SI5351_OUTPUT_2MA = 0b00,
	SI5351_OUTPUT_4MA = 0b01,
	SI5351_OUTPUT_6MA = 0b10,
	SI5351_OUTPUT_8MA = 0b11
} si5351OutputDrive_t;

typedef enum {
	SI5351_MS_MODE_INT = (0b0 << 6),
	SI5351_MS_MODE_FRAC = (0b1 << 6)
} si5351MultisynthMode_t;

typedef enum {
	SI5351_MS_NINV = (0b0 << 4),
	SI5351_MS_INV = (0b1 << 4)
} si5351MultisynthInv_t;

typedef enum {
	SI5351_MS_EN = (0b0 << 7),
	SI5351_MS_DIS = (0b1 << 7)
} si5351MultisynthEn_t;

typedef enum {
  SI5351_R_DIV_1 = 0 << 4,
  SI5351_R_DIV_2 = 1 << 4,
  SI5351_R_DIV_4 = 2 << 4,
  SI5351_R_DIV_8 = 3 << 4,
  SI5351_R_DIV_16 = 4 << 4,
  SI5351_R_DIV_32 = 5 << 4,
  SI5351_R_DIV_64 = 6 << 4,
  SI5351_R_DIV_128 = 7 << 4,
} si5351RDiv_t;

si5351_err_t si5351_init(si5351CrystalLoad_t crystalLoad);

si5351_err_t si5351_setupPLL(si5351PLL_t pll, uint8_t mult, uint32_t num, uint32_t denom);
si5351_err_t si5351_setMultisynthControl(uint8_t output, si5351MultisynthEn_t en, si5351PLL_t pllsource, si5351MultisynthInv_t inv, si5351MultisynthMode_t mode, si5351OutputDrive_t drv);
si5351_err_t si5351_setupMultisynth(uint8_t output, uint32_t div, uint32_t num, uint32_t denom, si5351RDiv_t rdiv);
si5351_err_t si5351_enableOutput(uint8_t output);
si5351_err_t si5351_disableOutput(uint8_t output);

#endif /* SI5351_H_ */

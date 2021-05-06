/*
 * sa2.c
 *
 *  Created on: 22 мар. 2021 г.
 *      Author: Sx107
 */


#include "sa2.h"
#include "sa2_lut.h"
#include "si5351.h"
#include "i2c.h"

/*
 * SA-2 key matrix map
 * KI	KO>	0		1		2		3		4		5		6		7
 * 0		F0		C#1		A1		F2		d0		d5		drum BD	drum OH
 * 1		F#0		D1		A#1		F#2		d1		d6		drum SN	drum CB
 * 2		G0		D#1		B1		G2		d2		d7		drum HH	---
 * 3		G#0		E1		C2		G#2		d3		d8		demo0	---
 * 4		A0		F1		C#2		A2		d4		d9		demo1	---
 * 5		A#0		F#1		D2		A#2		temp_u	stop	demo2	---
 * 6		B0		G1		D#2		B2		vol_u	temp_d	demo3	---
 * 7		C1		G#1		E2		C3		select	vol_d	demo4	polyphony
 */

// Current key matrix inputs
volatile uint8_t outState[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
volatile uint8_t outState_reset[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // outState reset mask (for SA2_NOTE_TRIGGER)
volatile uint8_t outState_trigger[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // outState trigger (for SA2_NOTE_TRIGGER)
// outState_trigger bit is set in sa2_key_set if Sa2_NOTE_TRIGGER state is selected
// outState_reset bit is set in sa2_handle_pins when just one output is active
// On the next condition, when only one key matrix output is active, outState bits will be reset according to outState_reset
// That way, SA2_NOTE_TRIGGER presses the key for at least 2 periods of key scanning (one period is way too unreliable)


// Clock regulation
volatile uint8_t  _sa2_current_octave = 2;				// Current octave, 2 is the standard octave (21.7MHz)
volatile uint16_t _sa2_pitchbend = 8192;				// Current pitchbend value (14-bit)
volatile uint8_t  _sa2_modulation = 0;					// Current modulation value (7-bit)
volatile sa2_pb_range_t _sa2_pbrange = SA2_PB_FULLTONE;	// Current pitchbend range
volatile sa2_mod_type_t _sa2_modtype = SA2_MOD_VIBRATO; // Current modulation type
volatile uint8_t _sa2_glitchm1m2 = 0; // M1, M2 (direct clock pin) glitches, executed once in sa2_update_clock(), M1 = 1, M2 = 2, M2 is more important than M1
volatile uint8_t _sa2_glitchclk = 0; // Current glitch M3 LSB 2 bits control the 1.5x or 2x overclock, 3rd bit controls the trigger mode

// Number of LFO bits in modulation fm and fm-high modes
#define SA2_MODBITS_FM 7
#define SA2_MODBITS_FM_HIGH 4


// Initialization functions

void sa2_gpio_init() {
	// Turn on port A, B
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN;

	// Configure PB8...PB15 as EXTI Input; these are key matrix outputs (from the SA-2 side)
	MODIFY_REG(AFIO->EXTICR[2],
			AFIO_EXTICR3_EXTI8 | AFIO_EXTICR3_EXTI9 | AFIO_EXTICR3_EXTI10 | AFIO_EXTICR3_EXTI11,
			AFIO_EXTICR3_EXTI8_PB | AFIO_EXTICR3_EXTI9_PB | AFIO_EXTICR3_EXTI10_PB | AFIO_EXTICR3_EXTI11_PB);
	MODIFY_REG(AFIO->EXTICR[3],
			AFIO_EXTICR4_EXTI12 | AFIO_EXTICR4_EXTI13 | AFIO_EXTICR4_EXTI14 | AFIO_EXTICR4_EXTI15,
			AFIO_EXTICR4_EXTI12_PB | AFIO_EXTICR4_EXTI13_PB | AFIO_EXTICR4_EXTI14_PB | AFIO_EXTICR4_EXTI15_PB);

	// PB8..PB15 EXTI, both rising edge and falling edge
	EXTI->IMR |=  0xFF << 8;
	EXTI->RTSR |= 0xFF << 8;
	EXTI->FTSR |= 0xFF << 8;

	// EXTI IRQ
	NVIC_SetPriority(EXTI9_5_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
	NVIC_EnableIRQ(EXTI9_5_IRQn);
	NVIC_SetPriority(EXTI15_10_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
	NVIC_EnableIRQ(EXTI15_10_IRQn);

	// Configure PA0...PA7 as outputs; these are key matrix inputs (from the SA-2 side)
	MODIFY_REG(GPIOA->CRL, 0x0000FFFF, GPIO_CRL_MODE0 | GPIO_CRL_MODE1 | GPIO_CRL_MODE2 | GPIO_CRL_MODE3);
	MODIFY_REG(GPIOA->CRL, 0xFFFF0000, GPIO_CRL_MODE4 | GPIO_CRL_MODE5 | GPIO_CRL_MODE6 | GPIO_CRL_MODE7);

	//PB4: Glitch; low = no glitch, high - clock pause
	MODIFY_REG(GPIOB->CRL, GPIO_CRL_CNF4_Msk | GPIO_CRL_MODE4_Msk, (0 << GPIO_CRL_CNF4_Pos) | (3 << GPIO_CRL_MODE4_Pos));
	GPIOB->BSRR |= GPIO_BSRR_BR4;

	//PB3: Reset; low = reset, high = no effect
	MODIFY_REG(GPIOB->CRL, GPIO_CRL_CNF3_Msk | GPIO_CRL_MODE3_Msk, (1 << GPIO_CRL_CNF3_Pos) | (3 << GPIO_CRL_MODE3_Pos));
	GPIOB->BSRR |= GPIO_BSRR_BS3;
}

void sa2_clock_init() {
	// Assuming si5351_init() has already been called.
	// Without the 1ms delays it works unstable for some reason;
	// Probably due to the adafruit si5351 pullup resistors being
	// too high? (10kOhm instead of 1k-4.7k)

	while(1) {
		i2c_reset_error();
		si5351_setupPLL(SI5351_PLL_A, 27, 97, 125);
		_delayMs(1);
		si5351_setupMultisynth(0, 16, 0, 1, SI5351_R_DIV_2);
		_delayMs(1);
		si5351_setMultisynthControl(0, SI5351_MS_EN, SI5351_PLL_A, SI5351_MS_NINV, SI5351_MS_MODE_FRAC, SI5351_OUTPUT_8MA);
		_delayMs(1);
		si5351_enableOutput(0);
		if(i2c_get_error() == 0) {
			break;
		} else {
			GPIOC->ODR ^= GPIO_ODR_ODR13;
		}
	}
}

// This function initializes the TIM2 timer which is used for the modulation LFO
// Raw counter register is used as the current LFO state
void sa2_timer_init() {
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	// Default values of SMCR and CR2 registers are okay (Internal clock)
	// Up-down (center-aligned) mode, no clock division, ARPE set, everything else at default
	MODIFY_REG(TIM2->CR1, TIM_CR1_CMS_Msk | TIM_CR1_CKD_Msk, (3 << TIM_CR1_CMS_Pos) | (0 << TIM_CR1_CKD_Pos) | TIM_CR1_ARPE);
	// ARR Changes the LFO bits (14 bits), PSC changes the LFO frequency
	TIM2->ARR = 16384;
	TIM2->PSC = _sa2_modrate_lut[0];
	TIM2->EGR |= TIM_EGR_UG; // Update the ARR and PSC
	// Finally, turn on the timer
	TIM2->CR1 |= TIM_CR1_CEN;
}

void sa2_init() {
	sa2_gpio_init();
	sa2_clock_init();
	sa2_timer_init();
}

// Handling functions

void sa2_handle_pins() {
	// To be called in EXTI interrupts
	uint8_t pinState = ((GPIOB->IDR) >> 8) & 0xFF; //Input PB8...PB15

	if(pinState == 0x00) { // No key matrix pins active
		//GPIOA->BRR |= 0xFF; // Turn all key matrix inputs off
		return;
	}

	uint8_t pinFirst = __CLZ(pinState) - 24;	// Number of trailing zeros in an 8-bit number
	pinFirst = 7 - pinFirst;					// Reverse this number, get the first 1 bit position from the right

	uint8_t nBits = __builtin_popcount(pinState); // Number of active bits

	// To prevent key matrix bugs, if nBits >= 2 ignore everything
	// Though nBits = 2 during switching
	if(nBits > 2) { // You may want to replace this with "nBits >= 2".
		GPIOA->BRR |= 0xFF;
		return;
	}

	// Edge case when key matrix switches from the last output to the first, during the transition both are active
	// Previous code would treat this situation as "last output active", not "first output active"
	// If in previous test >= 2 is coded, not > 2, then essentially this test will never occur.
	// In my case, when I try writing ">= 2", some keys (like digits 3 and 4) don't work stable.
	if(pinState == 0b10000001) {pinFirst = 0;}

	// Finally, set the proper key matrix inputs on
	uint8_t p = outState[pinFirst];
	GPIOA->BSRR |= ((~(uint32_t)p) << 16) | p; //Now isn't that obvious? :)

	// Process the SA2_NOTE_TRIGGER conditions
	if(nBits == 1) {
		outState[pinFirst] &= ~(outState_reset[pinFirst]);
		outState_reset[pinFirst] = outState_trigger[pinFirst];
		outState_trigger[pinFirst] = 0;
	}
}

void sa2_update_clock() {
	if (_sa2_glitchclk != 0) {
		if (_sa2_glitchclk == 1) {si5351_setupMultisynth(0, 12, 0, 1, 0);}
		else {si5351_setupMultisynth(0, 8, 0, 1, 0);}
		if ((_sa2_glitchclk & (1 << 3)) != 0) {
			_sa2_glitchclk = 0;
		}
		return;
	}

	if (_sa2_glitchm1m2 == 1) {
		// Mode 1 glitch. Push some impulses into the clock pin
		for(uint8_t i = 0; i < 0xFE; i++) {
			GPIOB->ODR ^= GPIO_ODR_ODR4;
			for(uint8_t j = 0; j < 32; j++) {};
		}
		GPIOB->BSRR |= GPIO_BSRR_BR4;
		_sa2_glitchm1m2 = 0;
	} else if (_sa2_glitchm1m2 == 2) {
		static uint16_t noise = 0xABCD;
		// Push some noise into the clock pin
		for(uint8_t i = 0; i < 0xFE; i++) {
			noise = (noise >> 0x1U) ^ (-(noise & 0x1U) & 0xB400U);
			if((noise & 0b1) == 0) {
				GPIOB->BSRR |= GPIO_BSRR_BR4;
			} else {
				GPIOB->BSRR |= GPIO_BSRR_BS4;
			}
			for(uint8_t j = 0; j < 32; j++) {};
		}
		GPIOB->BSRR |= GPIO_BSRR_BR4;
		_sa2_glitchm1m2 = 0;
	}


	uint16_t lfoValue = TIM2->CNT;

	if(_sa2_modtype == SA2_MOD_FM) {lfoValue = 0x3FFF & (lfoValue * (((1 << 14) - 1) / ((1 << SA2_MODBITS_FM) - 1)));} 					// lfoValue * (16383 / 127) in case of SA_MODBITS_FM = 7
	else if(_sa2_modtype == SA2_MOD_FM_HIGH) {lfoValue = 0x3FFF & (lfoValue * (((1 << 14) - 1) / ((1 << SA2_MODBITS_FM_HIGH) - 1)));}	// Just shifting bits is wrong

	uint16_t modValue = _sa2_modulation;
	lfoValue = 0x3FFF & (((int32_t)(lfoValue) - 0x2000L) * ((int32_t)modValue) / 127 + 0x2000L); // Convert to signed int, multiply, back to unsigned, make sure it's 14-bit


	_sa2_pitchbend &= 0x3FFF;
	_sa2_current_octave &= 3;

	uint8_t rDivValue = (3 - _sa2_current_octave) << 4; // See Si5351.h
														// A switch-case will be more obvious, but
														// I am not sure that optimizer will optimize it to this

	uint16_t lut_low, lut_high;
	uint32_t full_pb = 0x7FFF & (lfoValue + _sa2_pitchbend); // uint32_t, because full range pitchbend requires it
	uint32_t pb_final;

	switch(_sa2_pbrange) {
		case SA2_PB_FULLTONE:
			// First SA2_FULLTONE_LUT_BITS of the 14-bit number represent the
			// Current interval of the LUT
			// LUT order is inverted, so we invert the lut_low and lut_high here
			lut_low = _sa2_pb_lut_fulltone[SA2_FULLTONE_LUT_INT(full_pb) + 1];
			lut_high = _sa2_pb_lut_fulltone[SA2_FULLTONE_LUT_INT(full_pb)];

			// Last (14 - SA2_FULLTONE_LUT_BITS) of the 14-bit pitchbend represent the
			// current position in the LUT interval
			pb_final = SA2_FULLTONE_LUT_SUBTRACT + lut_low + (uint32_t)SA2_FULLTONE_LUT_POS(full_pb) * (uint32_t)(lut_high - lut_low) / (uint32_t)SA2_FULLTONE_LUT_INTLEN;

			// 16 is the midpoint of the pitchbend. Check if we are over it on octaved 3, pitchbend/mod up is disabled on it
			// Try commenting the next check, maybe your sa-2 wants to overclock.
			if((SA2_FULLTONE_LUT_MINVAL + (pb_final >> SA2_FULLTONE_LUT_PRECISION) < 16) && (_sa2_current_octave == 3)) {
				si5351_setupMultisynth(0, 16, 0, 1, rDivValue);
				return;
			}

			// My SA-2 Does not want to underclock even one bit under x4 underclocking, so disable the pitchbend down on the lowest octave
			if((SA2_FULLTONE_LUT_MINVAL + (pb_final >> SA2_FULLTONE_LUT_PRECISION) > 16) && (_sa2_current_octave == 0)) {
				si5351_setupMultisynth(0, 16, 0, 1, rDivValue);
				return;
			}

			si5351_setupMultisynth(0,
					SA2_FULLTONE_LUT_MINVAL + (pb_final >> SA2_FULLTONE_LUT_PRECISION), // Integer part
					pb_final & ((1 << SA2_FULLTONE_LUT_PRECISION) - 1),					// Numerator
					1 << SA2_FULLTONE_LUT_PRECISION,									// Denominator
					rDivValue);															// Additional division controlled by the current octave
			break;
		case SA2_PB_OCTAVE:
			// All LUT calculations are similar to the SA2_PB_FULLTONE calculations.
			lut_low = _sa2_pb_lut_octave[SA2_OCTAVE_LUT_INT(full_pb) + 1];
			lut_high = _sa2_pb_lut_octave[SA2_OCTAVE_LUT_INT(full_pb)];
			pb_final = SA2_OCTAVE_LUT_SUBTRACT + lut_low + (uint32_t)SA2_OCTAVE_LUT_POS(full_pb) * (uint32_t)(lut_high - lut_low) / (uint32_t)SA2_OCTAVE_LUT_INTLEN;

			// See these two checks above.
			if((SA2_OCTAVE_LUT_MINVAL + (pb_final >> SA2_OCTAVE_LUT_PRECISION) < 16) && (_sa2_current_octave == 3)) {
				si5351_setupMultisynth(0, 16, 0, 1, rDivValue);
				return;
			}
			if((SA2_OCTAVE_LUT_MINVAL + (pb_final >> SA2_OCTAVE_LUT_PRECISION) > 16) && (_sa2_current_octave == 0)) {
				si5351_setupMultisynth(0, 16, 0, 1, rDivValue);
				return;
			}

			si5351_setupMultisynth(0,
					SA2_OCTAVE_LUT_MINVAL + (pb_final >> SA2_OCTAVE_LUT_PRECISION), // Integer part
					pb_final & ((1 << SA2_OCTAVE_LUT_PRECISION) - 1),				// Numerator
					1 << SA2_OCTAVE_LUT_PRECISION,									// Denominator
					rDivValue);
			break;
		case SA2_PB_FULL:
			// All LUT calculations are similar to the SA2_PB_FULLTONE calculations.
			// But! LUT input here is 14-bit, and modulation is truncated (full_pb is converted)

			if(full_pb < 0x2000) {full_pb = 0x2000;}
			else if (full_pb >= 0x6000) {full_pb = 0x6000 - 1;}
			else {full_pb = 0x3FFF & (full_pb - 0x2000);} // Bitwise & added just in case

			lut_low = _sa2_pb_lut_full[SA2_FULL_LUT_INT(full_pb) + 1];
			lut_high = _sa2_pb_lut_full[SA2_FULL_LUT_INT(full_pb)];
			pb_final = SA2_FULL_LUT_SUBTRACT + lut_low + (uint32_t)SA2_FULL_LUT_POS(full_pb) * (uint32_t)(lut_high - lut_low) / (uint32_t)SA2_FULL_LUT_INTLEN;

			si5351_setupMultisynth(0,
					SA2_FULL_LUT_MINVAL + (pb_final >> SA2_FULL_LUT_PRECISION), // Integer part
					pb_final & ((1 << SA2_FULL_LUT_PRECISION) - 1),				// Numerator
					1 << SA2_FULL_LUT_PRECISION,								// Denominator
					SI5351_R_DIV_1);
			break;
	}
}

// Pitchbend, mod, octave setters
void sa2_set_pitchbend(uint16_t pb) {
	_sa2_pitchbend = pb & 0x3FFF;
}

void sa2_set_octave(uint8_t oct) {
	_sa2_current_octave = oct & 0x3;
}

void sa2_set_pitchbend_range(sa2_pb_range_t range) {
	_sa2_pbrange = range;
}

void sa2_set_modulation(uint8_t mod) {
	_sa2_modulation = mod & 0x7F; // 7-bit
}

void sa2_set_modulation_rate(uint8_t mod) {
	TIM2->PSC = _sa2_modrate_lut[mod & 0x7F]; // See sa2_lut.h
}

void sa2_set_modulation_type(sa2_mod_type_t modtype) {
	if (_sa2_modtype == modtype) {return;}
	if(modtype == SA2_MOD_VIBRATO) {TIM2->ARR = 0b1UL << 14;}
	else if (modtype == SA2_MOD_FM) {TIM2->ARR = 0b1UL << SA2_MODBITS_FM;}
	else if (modtype == SA2_MOD_FM_HIGH) {TIM2->ARR = 0b1 << SA2_MODBITS_FM_HIGH;}
	TIM2->EGR |= TIM_EGR_UG;
	_sa2_modtype = modtype;
}

void sa2_modulation_sync(void) {
	TIM2->EGR |= TIM_EGR_UG;
}

// Glitch, reset, polyphony

void sa2_glitch_m1() {
	if(_sa2_glitchm1m2 != 2) {_sa2_glitchm1m2 = 1;}
}

void sa2_glitch_m2() {
	_sa2_glitchm1m2 = 2;
}

void sa2_glitch_m3(sa2_glitchm3_type_t type, sa2_note_state_t st) {
	if(st == SA2_NOTE_STOP) {
		_sa2_glitchclk = 0;
	} else {
		_sa2_glitchclk = type;
		if(st == SA2_NOTE_TRIGGER) {
			_sa2_glitchclk |= 1 << 3;
		}
	}
}

void sa2_issue_reset() {
	GPIOB->BSRR |= GPIO_BSRR_BR3;
	for(uint16_t i = 0; i < 0xFFE; i++) {}
	GPIOB->BSRR |= GPIO_BSRR_BS3;
}

void sa2_force_clock() {
	GPIOB->BSRR |= GPIO_BSRR_BS4;
	_delayMs(1);
	GPIOB->BSRR |= GPIO_BSRR_BR4;
	_sa2_glitchm1m2 = 0;
	_delayMs(10);
	sa2_issue_reset();
}

void sa2_set_poly(sa2_poly_t p) {
	if(p == SA2_POLY_2) {
		outState[7] &= ~(1 << 7);
	} else {
		outState[7] |= (1 << 7);
	}
}

// Basic key functions

void sa2_set_keystate(uint8_t kO, uint8_t kI) {
	outState[kO] = (outState[kO] & 0xFF00) | kI;
}

void sa2_key_set(uint8_t kO, uint8_t bitMask, sa2_note_state_t st) {
	if(st == SA2_NOTE_STOP) {
		outState[kO] &= ~bitMask;
	} else {
		outState[kO] |=  bitMask;
		if (st == SA2_NOTE_TRIGGER) {
			outState_trigger[kO] |= bitMask;
		}
	}
}

void sa2_press_keys(uint8_t kO, uint8_t kI, sa2_note_state_t st) {
	sa2_key_set(kO, kI, st);
}

void sa2_reset_all_keys() {
	for(uint8_t i = 0; i < 7; i++) {outState[i] = 0x0000U;}
	outState[7] &= 0b1 << 7; // Don't touch the polyphony bit
}

// All the boring functions...

void sa2_press_note(uint8_t note, sa2_note_state_t st) {
	if(note > 31) {note = 31;}
	sa2_key_set(0 + (note >> 3), 1 << (note & 0b111), st);
}

void sa2_press_digit(uint8_t digit, sa2_note_state_t st) {
	if(digit > 9) {digit = 9;}
	sa2_key_set(4 + digit / 5,  1 << (digit % 5), st);
}

void sa2_press_drum(uint8_t drum, sa2_note_state_t st) {
	if (drum > 4) {drum = 4;}
	// BD, SN, HH are on kO 6
	// OH, CB are on kO 7
	sa2_key_set(6 + drum / 3, 1 << (drum % 3), st);
}

void sa2_press_demo(uint8_t demo, sa2_note_state_t st) {
	if (demo > 4) {demo = 4;}
	sa2_key_set(6, 1 << (demo + 3), st);
}

void sa2_press_tempoUp(sa2_note_state_t st) {
	sa2_key_set(4, 1 << 5, st);
}

void sa2_press_tempoDown(sa2_note_state_t st) {
	sa2_key_set(5, 1 << 6, st);
}

void sa2_press_volUp(sa2_note_state_t st) {
	sa2_key_set(4, 1 << 6, st);
}

void sa2_press_volDown(sa2_note_state_t st) {
	sa2_key_set(5, 1 << 7, st);
}

void sa2_press_select(sa2_note_state_t st) {
	sa2_key_set(4, 1 << 7, st);
}

void sa2_press_stop(sa2_note_state_t st) {
	sa2_key_set(5, 1 << 5, st);
}

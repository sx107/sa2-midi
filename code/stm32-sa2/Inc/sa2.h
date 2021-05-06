/*
 * sa2.h
 *
 *  Created on: 22 мар. 2021 г.
 *      Author: Sx107
 */

#include "main.h"

#ifndef SA2_H_
#define SA2_H_

typedef enum {
	SA2_POLY_2,
	SA2_POLY_4
} sa2_poly_t;

typedef enum {
	SA2_NOTE_PLAY,
	SA2_NOTE_STOP,
	SA2_NOTE_TRIGGER 	// Plays once and releases after the key is physically pressed
						// Useful for functional keys (digits, volume, tempo)
} sa2_note_state_t;

typedef enum {
	SA2_PB_FULLTONE,
	SA2_PB_OCTAVE,
	SA2_PB_FULL		// From x4 underclocking to x2 overclocking
} sa2_pb_range_t;

typedef enum {
	SA2_MOD_VIBRATO, 	// Cuts down the LFO precision
	SA2_MOD_FM,			// I know it is not a perfect solution, but a better one would require multiple LUTs
	SA2_MOD_FM_HIGH		// And precision loss is unavoidable anyway, yet it is not that important at high frequencies, frequency tracking is way more important which remains (again, same LUT)
} sa2_mod_type_t;

typedef enum {
	SA2_GLITCHM3_1_5_x = 1,
	SA2_GLITCHM3_2_0_x = 2
} sa2_glitchm3_type_t;

void sa2_init();
void sa2_clock_init(); // Can be called even without sa2_init()
void sa2_force_clock(); // Sometimes it is needed during initialization if your SA-2 CPU is undervolted (~3V)

// To be called in EXTI9_5_IRQHandler, EXTI15_10_IRQHandler
void sa2_handle_pins();

// Glitches
void sa2_glitch_m1(); // 2 impulses are pushed into clock
void sa2_glitch_m2(); // Noisy clock
void sa2_glitch_m3(sa2_glitchm3_type_t type, sa2_note_state_t st); // Overclock 1.5x

// Reset and polyphony
void sa2_issue_reset();
void sa2_set_poly(sa2_poly_t p); // Issue a reset after this one.

// Raw key presses
void sa2_set_keystate(uint8_t kO, uint8_t kI);
void sa2_press_keys(uint8_t kO, uint8_t kI, sa2_note_state_t st);
void sa2_reset_all_keys(); // Does not affect the polyphony.

// Notes, drums, etc
void sa2_press_note(uint8_t note, sa2_note_state_t st); 	// From F0 to C2, 32 notes
void sa2_press_drum(uint8_t drum, sa2_note_state_t st);	// 0 = Bass drum, 1 = Snare, 2 = Hihat, 3 = Open hat, 4 = Cowbell
void sa2_press_demo(uint8_t demo, sa2_note_state_t st);	// Demo from 0 to 4
void sa2_press_tempoUp(sa2_note_state_t st);
void sa2_press_tempoDown(sa2_note_state_t st);
void sa2_press_volUp(sa2_note_state_t st);
void sa2_press_volDown(sa2_note_state_t st);
void sa2_press_select(sa2_note_state_t st);
void sa2_press_stop(sa2_note_state_t st);
void sa2_press_digit(uint8_t digit, sa2_note_state_t st);

void sa2_set_pitchbend(uint16_t pb);					// 14-bit MIDI pitchbend
void sa2_set_pitchbend_range(sa2_pb_range_t range);		// Set pitchbend range
void sa2_set_modulation(uint8_t mod); 					// 7-bit MIDI modulation
void sa2_set_modulation_rate(uint8_t modrate);			// 7-bit modulation rate
void sa2_set_modulation_type(sa2_mod_type_t modtype); 	// Vibrato (low freq) or FM (high freq) or FM-high (very high freq mod, essentially noise due to sa2_update_clock() low call rate)
void sa2_modulation_sync(void);							// Resets the modulation LFO
void sa2_set_octave(uint8_t oct); 						// Octave: from 0 to 3, 2 is the standard octave
														// On octave 0 pitchbend/mod down won't work
														// On octave 3 pitchbend/mod up won't work - my SA-2 REALLY dislikes overclocking much more than x2
														// But you can change the sa2_update_clock code to change this behavior
void sa2_update_clock();

#endif /* SA2_H_ */

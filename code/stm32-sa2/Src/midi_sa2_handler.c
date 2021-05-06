/*
 * midi_sa2_handler.c
 *
 *  Created on: Mar 27, 2021
 *      Author: Sx107
 */

#include "sa2.h"
#include "midi.h"
#include "midi_sa2_handler.h"

void handle_midi_message(uint8_t cmd, uint8_t fb, uint8_t sb) {
	// Note that implicit note off is already dealt with and passed as a 0x80 note off command
	switch(MIDI_CMD(cmd)) {
	case 0xE0:
		sa2_set_pitchbend((uint16_t)fb + ((uint16_t)sb << 7));
		break;
	case 0xB0:
		if (fb == 1) {sa2_set_modulation(sb);}				// CC1: Modulation
		else if (fb == 3) {sa2_set_modulation_rate(sb);}	// CC3: ModRate
		else if (fb == 16) {								// CC 16: Polyphony (4/2)
			if (sb >= 64) {sa2_set_poly(SA2_POLY_4);}
			else {sa2_set_poly(SA2_POLY_2);}
		}
		else if (fb == 17) {								// CC 17: ModRate range
			if (sb >= 84) {sa2_set_modulation_type(SA2_MOD_FM_HIGH);}
			else if (sb >= 42) {sa2_set_modulation_type(SA2_MOD_FM);}
			else {sa2_set_modulation_type(SA2_MOD_VIBRATO);}
		}
		else if(fb == 123) {sa2_reset_all_keys();}			// CC123: All notes off
		break;
	case 0x90:
		// Handle all the functional/triggered only keys here
		if (fb == 24) {sa2_press_select(SA2_NOTE_TRIGGER); return;}									// C0 : select
		else if (fb == 25) {sa2_press_stop(SA2_NOTE_TRIGGER); return;}								// C#0 : stop
		else if (fb == 26) {sa2_press_tempoDown(SA2_NOTE_TRIGGER); return;}							// D0 : tempo down
		else if (fb == 27) {sa2_press_tempoUp(SA2_NOTE_TRIGGER); return;}							// D#0 : tempo up
		else if (fb == 28) {sa2_press_volDown(SA2_NOTE_TRIGGER); return;}							// E0 : volume down
		else if (fb == 29) {sa2_press_volUp(SA2_NOTE_TRIGGER); return;}								// F0 : volume up
		else if (fb >= 31 && fb < (31 + 5)) {sa2_press_demo(fb - 30, SA2_NOTE_TRIGGER); return;}	// G0 - B0: demos
		else if ((fb >= 36) && (fb < 36+10)) {sa2_press_digit(fb - 36, SA2_NOTE_TRIGGER); return;}	// C1 - A1: digits
		else if ((fb >= 48) && (fb < 52)) {sa2_set_octave((fb - 48)); return;} 						// C2 - D#2: set octave
		else if (fb == 52) {sa2_set_pitchbend_range(SA2_PB_FULLTONE); return;} 						// E2 - F#2: set pitchbend range
		else if (fb == 53) {sa2_set_pitchbend_range(SA2_PB_OCTAVE); return;}						// E2 - F#2: set pitchbend range
		else if (fb == 54) {sa2_set_pitchbend_range(SA2_PB_FULL); return;}							// E2 - F#2: set pitchbend range
		else if (fb == 55) {sa2_glitch_m1(); return;}												// G2: mode 1 glitch
		else if (fb == 56) {sa2_glitch_m2(); return;}												// G#2: mode 2 glitch
		else if (fb == 58) {sa2_issue_reset(); return;}												// A#2: reset
		else if (fb == 59) {sa2_modulation_sync(); return;}											// B2: modulation reset
		else if (fb >= 60 && fb < 65) {sa2_press_drum(fb - 60, SA2_NOTE_TRIGGER); return;}			// C3 - E3: drums
	case 0x80:
		// Handle all keys here
		if(fb >= 65 && fb < (65+32)) { // Normal keys from F3 to C6
			sa2_press_note(fb - 65, (cmd == 0x90) ? SA2_NOTE_PLAY : SA2_NOTE_STOP);
		}
		else if (fb == 57) {sa2_glitch_m3(SA2_GLITCHM3_1_5_x, (cmd == 0x90) ? SA2_NOTE_PLAY : SA2_NOTE_STOP); return;}			// A2: mode 3 glitch
		break;
	}
}

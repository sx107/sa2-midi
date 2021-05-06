/*
 * midi.c
 *
 *  Created on: Mar 27, 2021
 *      Author: Sx107
 */

#include "midi.h"
#include "main.h"
#include "midi_sa2_handler.h"

volatile uint8_t _midi_status = 0x00;
volatile midi_mode_t _midi_mode = MIDI_MODE_RESET;
volatile uint8_t _midi_firstbyte = 0x00;
volatile uint8_t _midi_own_channel = MIDI_ALLCHAN;

#define MIDI_IS_STAT(X) ((X & 0x80) == 0x80)
#define MIDI_IS_DATA(X) ((X & 0x80) == 0)

// See midi_sa2_handler.c
/*
void handle_midi_message(uint8_t cmd, uint8_t fb, uint8_t sb) {
}
*/

void USART1_IRQHandler(void) {
	uint8_t rb = USART1->DR;

	if(((USART1->SR) & USART_SR_ORE) != 0) {
		_midi_mode = MIDI_MODE_RESET;
		return;
	}

	// Ignore all the realtime bytes, they must not affect the parsing.
	if(rb == 0xF8 || rb == 0xFA || rb == 0xFB || rb == 0xFC || rb == 0xFE || rb == 0xFF) {
		return;
	}

	if(((_midi_mode == MIDI_MODE_RESET) || (_midi_mode == MIDI_MODE_SYSEX)) && MIDI_IS_DATA(rb)) {
		// Simply wait. We don't know the running status at this point anyways
		return;
	}

	if MIDI_IS_STAT(rb) {
		// Immedeately change the running status
		_midi_status = rb;
		uint8_t cmd = MIDI_CMD(rb);
		if(rb == 0xF0) {_midi_mode = MIDI_MODE_SYSEX;}
		else if(cmd == 0xF0) {_midi_mode = MIDI_MODE_RESET;} // Any other 0xFx message, including sysex end. Start ignoring everything except status bytes
		else if(cmd == 0xC0 || cmd == 0xD0) {_midi_mode = MIDI_MODE_SINGLE_WAIT;} // Channel and pressure, only non-0xFx messages with one byte
		else {_midi_mode = MIDI_MODE_1BYTE_WAIT;}
	} else {
		if (MIDI_CMD(_midi_status) == 0xF0) {return;} // Just to be sure.
		if ((_midi_own_channel != MIDI_ALLCHAN) && (MIDI_CHAN(rb) != _midi_own_channel)) {return;} // MIDI Channel check
		if(_midi_mode == MIDI_MODE_1BYTE_WAIT) {
			// Store the first byte, wait for second
			_midi_firstbyte = rb;
			_midi_mode = MIDI_MODE_2BYTE_WAIT;
		} else if (_midi_mode == MIDI_MODE_2BYTE_WAIT) {
			// Handle 2-byte midi message
			if ((MIDI_CMD(_midi_status) == 0x90) && (rb == 0)) { // Implicit note off
				handle_midi_message(0x80 | MIDI_CHAN(_midi_status), _midi_firstbyte, 0);
			} else {
				handle_midi_message(_midi_status, _midi_firstbyte, rb);
			}
			_midi_mode = MIDI_MODE_1BYTE_WAIT;
		} else if (_midi_mode == MIDI_MODE_SINGLE_WAIT) {
			handle_midi_message(_midi_status, rb, 0);
		}
	}
}

void midi_init(void) {
	// 1. Init the clocks
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_USART1EN;

	//2. Init the PA9 (USART TX), PA10 (USART RX) pins
	//MODIFY_REG(GPIOA->CRH, GPIO_CRH_CNF9 | GPIO_CRH_MODE9, (3 << GPIO_CRH_CNF9) | (3 << GPIO_CRH_MODE9_Pos)); // Don't need that.
	MODIFY_REG(GPIOA->CRH, GPIO_CRH_CNF10 | GPIO_CRH_MODE10, (1 << GPIO_CRH_CNF10_Pos) | (0 << GPIO_CRH_MODE10_Pos));
	//MODIFY_REG(GPIOA->CRH, GPIO_CRH_CNF10 | GPIO_CRH_MODE10, (2 << GPIO_CRH_CNF10_Pos) | (0 << GPIO_CRH_MODE10_Pos));
	//MODIFY_REG(GPIOA->ODR, 0, GPIO_ODR_ODR10);

	// 3. Config the NVIC for the USART IRQ
	NVIC_SetPriority(USART1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),1, 0));
	NVIC_EnableIRQ(USART1_IRQn);

	// 4. Init the USART
	// Midi: 31250kbps, 8bit, no parity, 1 stopbit

	MODIFY_REG(USART1->CR1, USART_CR1_M | USART_CR1_PCE | USART_CR1_TE, (0 << USART_CR1_M_Pos) | (0 << USART_CR1_PS_Pos) | USART_CR1_RE);
	MODIFY_REG(USART1->CR2, USART_CR2_STOP, 0 << USART_CR2_STOP_Pos);
	// CR3 is just fine.

	// 31250kbps, so USARTDIV = 72MHz / (16 * 31250) = 144
	USART1->BRR = 144UL << USART_BRR_DIV_Mantissa_Pos;

	// Enable the RX interrupt and USART
	USART1->CR1 |= USART_CR1_UE | USART_CR1_RXNEIE;
}

void midi_enable(void) {
	USART1->CR1 |= USART_CR1_RXNEIE;
}

void midi_disable(void) {
	USART1->CR1 &= ~USART_CR1_RXNEIE;
}

void midi_set_channel(uint8_t chan) {
	if(chan == MIDI_ALLCHAN) {_midi_own_channel = MIDI_ALLCHAN;}
	else {_midi_own_channel = 0x0F & chan;}
}

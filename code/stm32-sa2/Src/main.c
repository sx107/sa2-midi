/**
 *
 * Author: Aleksandr Kurganov aka sx107
 *
 * This software component is licensed under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 */

#include <stdint.h>
#include "main.h"
#include "i2c.h"
#include "si5351.h"
#include "sa2.h"
#include "midi.h"

#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif


// For the sa2.c
void EXTI9_5_IRQHandler(void)
{
	EXTI->PR |= EXTI_PR_PR8 | EXTI_PR_PR9;
	sa2_handle_pins();
}

void EXTI15_10_IRQHandler(void)
{
	EXTI->PR |= EXTI_PR_PR10 | EXTI_PR_PR11 | EXTI_PR_PR12 | EXTI_PR_PR13 | EXTI_PR_PR14 | EXTI_PR_PR15;
	sa2_handle_pins();
}

void rcc_configure() {
	// Flash latency: 78 MHz -> Two wait states (0b010)
	FLASH->ACR |= FLASH_ACR_LATENCY_1;
	WAITBIT(FLASH->ACR, FLASH_ACR_LATENCY_1);

	//Enable HSE
	RCC->CR |= RCC_CR_HSEON;
	WAITBIT(RCC->CR, RCC_CR_HSERDY);

	// HSE, x9, no prediv
	MODIFY_REG(RCC->CFGR, RCC_CFGR_PLLMULL | RCC_CFGR_PLLSRC, RCC_CFGR_PLLMULL9 | RCC_CFGR_PLLSRC);

	//Enable PLL
	RCC->CR |= RCC_CR_PLLON;
	WAITBIT(RCC->CR, RCC_CR_PLLRDY);

	//Set prescalers: AHB = /1, APB1 = /2, APB2 = /1
	MODIFY_REG(RCC->CFGR,
			RCC_CFGR_HPRE | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2,
			RCC_CFGR_HPRE_DIV1 | RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_PPRE2_DIV1);

	MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, RCC_CFGR_SW_PLL);
	WAITVAL(RCC->CFGR, RCC_CFGR_SWS, RCC_CFGR_SWS_PLL);

	__disable_irq();
	SysTick->LOAD = (uint32_t)(72000 + 1);
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
	SysTick->VAL = 0UL;
	__enable_irq();
}

void gpio_configure() {
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN | RCC_APB2ENR_IOPDEN;

	// Configure LED as output
	MODIFY_REG(GPIOC->CRH,
			GPIO_CRH_MODE13 | GPIO_CRH_CNF13,
			(2 << GPIO_CRH_MODE13_Pos) | (0 << GPIO_CRH_CNF13_Pos));
	GPIOC->BSRR |= GPIO_BSRR_BS13;

}

void _delayMs(uint32_t del) {
	SysTick->VAL = 0;
	while (del) {
		if ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) != 0U)
		{
			del--;
	    }
	}
}

uint8_t s6[16] = {0b001, 0b100, 0b010, 0b100, 0b100, 0b001, 0b010, 0b100, 0b001, 0b100, 0b010, 0b100, 0b100, 0b001, 0b010, 0b100};
uint8_t s7[16] = {0b0, 0b0, 0b10, 0b0, 0b0, 0b0, 0b0, 0b0, 0b10, 0b0, 0b0, 0b0, 0b10, 0b0, 0b0, 0b0};

int main(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
	RCC->APB1ENR |= RCC_APB1ENR_PWREN;
	NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

	AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_JTAGDISABLE;
	DBGMCU->CR |= DBGMCU_CR_DBG_STOP | DBGMCU_CR_DBG_STANDBY | DBGMCU_CR_DBG_SLEEP;

	rcc_configure();
	gpio_configure();


	i2c_init();


	// Attempt to init the si5351, flash the led if we fail
	while(1) {
		i2c_reset_error();
		si5351_init(SI5351_CRYSTAL_LOAD_10PF);
		_delayMs(100);
		if(i2c_get_error() == 0) {break;}
		else {GPIOC->ODR ^= GPIO_ODR_ODR13;}
	}
	GPIOC->BSRR |= GPIO_BSRR_BS13;

	// TODO: Find a way to detect that MIDI is connected, and, if not, call this routine to turn off everything to minimize the power consumption
	/*
	sa2_clock_init();
	i2c_deinit();
	PWR->CR |= PWR_CR_PDDS;
	__WFI();
	*/

	sa2_init();
	midi_init();
	_delayMs(10);
	sa2_set_poly(SA2_POLY_4);
	sa2_issue_reset();
    /* Loop forever */
	for(;;) {
		sa2_update_clock();
	}

}

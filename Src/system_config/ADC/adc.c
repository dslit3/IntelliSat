/*
 * adc.c
 *
 *  Created on: Feb 10, 2024
 *      Author: Chandrark Muddana
 */

#include "adc.h"

//USES ADC1

void adc_init(){
	RCC->AHB2ENR |= RCC_AHB2ENR_ADCEN; //enables ADC clock
	//ADC1->CR |= ADC_CR_ADDIS; //Disables ADC
	RCC->CCIPR   &= ~( RCC_CCIPR_ADCSEL ); //enables peripheral clock
	RCC->CCIPR   |=  ( 3 << RCC_CCIPR_ADCSEL_Pos ); //Sets ADC clock to system clock
	ADC1->CR    &= ~( ADC_CR_DEEPPWD );//makes sure ADC isn't in deep power down mode
	ADC1->CR |= ADC_CR_ADVREGEN; //enables ADC voltage regulator
	//Test what happens if voltage regulator is gone
	nop(10000); //waits a bit
	ADC1->CR &= ~(ADC_CR_ADCALDIF); //Sets it to single ended mode
	//ADC1->CR |= ADC_CR_ADCAL; //Calibrates ADC
	//while ((ADC1->CR & ADC_CR_ADCAL) != 0) { } //Waits until ADC is calibrated

}

void adc_enable(){
	ADC1->ISR |= ADC_ISR_ADRDY; // Checks if it is ready
	ADC1->CR |= ADC_CR_ADEN; //Enables ADC
	while ((ADC1->ISR & ADC_ISR_ADRDY) == 0) { } //Waits until ADRDY is reset
}
void adc_configGPIO(){
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN; //enables GPIO C
	while (GPIOC->OTYPER == 0xFFFFFFFF); //Waits until its done enabling

	GPIOC->OTYPER       &= ~( GPIO_OTYPER_OT0 ); //Reset C0 pin
	GPIOC->PUPDR        &= ~( GPIO_OSPEEDR_OSPEED0 ); //Reset C0 pin
	GPIOC->OSPEEDR      &= ~( GPIO_PUPDR_PUPD0 ); //Reset C0 pin
	GPIOC->MODER        &= ~( GPIO_MODER_MODE0 ); //Reset C0 pin
	GPIOC->ASCR 		|=  ( GPIO_ASCR_ASC0 ); //Connects analog switch to ADC channel for C0
	GPIOC->MODER        |=  ( 0x3 << GPIO_MODER_MODE0_Pos ); //Sets mode to analog

}

void adc_setConstantGPIOValue(){
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN; //enables GPIO B
	while (GPIOB->OTYPER == 0xFFFFFFFF); //Waits until its done enabling

	GPIOB->MODER &= ~(
		  GPIO_MODER_MODE3_Msk
		| GPIO_MODER_MODE4_Msk
		| GPIO_MODER_MODE5_Msk
		| GPIO_MODER_MODE6_Msk); //Resets config of B3 to B6

	GPIOB->MODER |=
	   	  GPIO_MODER_MODE3_0	// B3
		| GPIO_MODER_MODE4_0	// B4
		| GPIO_MODER_MODE5_0	// B5
		| GPIO_MODER_MODE6_0;	// B6 Sets to output mode

	gpio_set(GPIOB, 3, 0); //3-3
	gpio_set(GPIOB, 4, 0); //3-2
	gpio_set(GPIOB, 5, 1); //2-2
	gpio_set(GPIOB, 6, 1); //1-3
}

void adc_setChannel(){
	//leave sampling time at default
	ADC1->SQR1 &= ~( ADC_SQR1_L ); //Sets number of channels in the sequence of 1
	ADC1->SQR1 &= ~(ADC_SQR1_SQ1); //Resets the sequence
	ADC1->SQR1 |= (1 << ADC_SQR1_SQ1_Pos); //Sets the sequence to just channel 1 (PIN C0)
	ADC1->SMPR1 &= ~( ADC_SMPR1_SMP1 ); //Resets the sampling time of channel 1
	ADC1->SMPR1 |=  ( 0x7 << ADC_SMPR1_SMP1_Pos ); //Sets the sampling time to max: 640.5 cycles per sample
}

// Perform a single ADC conversion.
// (Assumes that there is only one channel per sequence)
uint16_t adc_singleConversion() {
  // Start the ADC conversion.
  ADC1->CR  |=  ( ADC_CR_ADSTART );
  // Wait for the 'End Of Conversion' flag.
  while ( !( ADC1->ISR & ADC_ISR_EOC ) ) {};
  // Read the converted value (this also clears the EOC flag).
  uint16_t adc_val = ADC1->DR;
  // Wait for the 'End Of Sequence' flag and clear it.
  while ( !( ADC1->ISR & ADC_ISR_EOS ) ) {};
  ADC1->ISR |=  ( ADC_ISR_EOS );
  // Return the ADC value.
  return adc_val;
}

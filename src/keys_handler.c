/**
 * Brief:
 **/

#include <stdint.h>
#include <stdbool.h>

#define TAG "KEYS"

#include "utils/log.h"

#include "hal/hal_mcu.h"

void KEYS_Initialize(void)
{
    /* 1. setup timers for encoders */
    /* 2. setup ADC for keys control */

    
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;

    /* -------------------------------------------------------------- */
    TIM1->CCMR1 = TIM_CCMR1_CC1S_0 | TIM_CCMR1_CC2S_1;
    TIM1->CCER = 0;

    TIM1->PSC = 0;
    TIM1->ARR = 255;
    TIM1->CNT = 0;

    /* Use Encoder mode 2 */
    TIM1->SMCR |= TIM_SMCR_SMS_1;

    /* -------------------------------------------------------------- */
    TIM4->CCMR1 = TIM_CCMR1_CC1S_0 | TIM_CCMR1_CC2S_1;
    TIM4->CCER = 0;

    TIM4->PSC = 0;
    TIM4->ARR = 255;
    TIM4->CNT = 0;

    /* Use Encoder mode 2 */
    TIM4->SMCR |= TIM_SMCR_SMS_1;

    /* -------------------------------------------------------------- */

    TIM1->CR1 = TIM_CR1_CEN;
    TIM4->CR1 = TIM_CR1_CEN;

//    ADC4->
}

__attribute__((used))
    void HAL_IRQ_ADC4_Handler(void)
{

}

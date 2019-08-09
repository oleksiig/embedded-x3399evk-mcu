/**
 * Brief:
 **/

#include <stdint.h>

#define TAG "I2S-CLK"

#include "utils/log.h"

#include "hal/hal_mcu.h"
#include "hal/hal_gpio.h"

/* */
void I2S_ClockGatingInitialize(void)
{
    /* Configure TIM2 and TIM15 for I2S clock division */

    /* */
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    RCC->APB2ENR |= RCC_APB2ENR_TIM15EN;

    /* */
    TIM2->CCMR1 = TIM_CCMR1_IC2F_0 | TIM_CCMR1_IC2F_1; /* toggle OCxREF */
    TIM2->CCER = TIM_CCER_CC2E;
    TIM2->PSC = 0;
    TIM2->ARR = 2 -1; /* DIV / 4 */
   // TIM2->CR2 = TIM_CR2_MMS_2; /* Compare 1 */
    TIM2->CR2 = TIM_CR2_MMS_2 | TIM_CR2_MMS_0; /* Compare 2 */

    TIM2->SMCR |= TIM_SMCR_MSM; /* Master mode */

    /* Use TIM2_ETR for ext clock */
    TIM2->SMCR |= TIM_SMCR_SMS_0 | TIM_SMCR_SMS_1 | TIM_SMCR_SMS_2;     /* Ext clock mode */
    TIM2->SMCR |= TIM_SMCR_TS_0 | TIM_SMCR_TS_1 | TIM_SMCR_TS_2;        /* ETRF */

    /* */
    //TIM15->CCMR1 = TIM_CCMR1_IC1F_0 | TIM_CCMR1_IC1F_1; /* toggle OCxREF */
    TIM15->CCER = TIM_CCER_CC1E;

    TIM15->SMCR = TIM_SMCR_SMS_0 | TIM_SMCR_SMS_1 | TIM_SMCR_SMS_2; /* Ext clock mode */
    TIM15->SMCR &= ~TIM_SMCR_TS;                                    /* TIM2 */

    TIM15->PSC = 0;
    TIM15->ARR = 32 -1; /* DIV TIM2 / 64 */

    TIM15->CCMR1 &= ~(TIM_CCMR1_CC1S);                       /* CH1 as output */
    TIM15->CCMR1 |= (TIM_CCMR1_OC1M_0 | TIM_CCMR1_OC1M_1);   /* Toggle - OC1REF toggles when TIMx_CNT=TIMx_CCR1 */

    TIM15->BDTR = TIM_BDTR_MOE | TIM_BDTR_AOE | TIM_BDTR_OSSR;

    /* */
    TIM2->CR1 = TIM_CR1_CEN;
    TIM15->CR1 = TIM_CR1_CEN;
}

void I2S_RefClkStart(void)
{
    HAL_GPIO_SetBit(GPIOA, GPIO_BIT3);
}

void I2S_RefClkStop(void)
{
    HAL_GPIO_ClrBit(GPIOA, GPIO_BIT3);
}

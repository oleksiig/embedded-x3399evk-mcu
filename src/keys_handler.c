/**
 * Brief:
 **/

#include <stdint.h>
#include <stdbool.h>

#define TAG "KEYS"

#include "utils/log.h"

#include "hal/hal_mcu.h"
#include "hal/hal_irq.h"
#include "hal/hal_systick.h"
#include "hal/hal_dma.h"

static const HAL_DMA_Config_t g_pDmaCfg =
{
    .Direction          = e_HAL_DMA_Periph2Mem,
    .Priority           = e_HAL_DMA_PRIO_High,
    .bCircularMode      = true,
    .Periph.bIncrement  = false,
    .Periph.Step        = e_HAL_DMA_STEP_16,
    .Memory.bIncrement  = true,
    .Memory.Step        = e_HAL_DMA_STEP_16
};

/* */
static uint16_t g_pAdcRawSamples[2] = {0};

/* */
void KEYS_VolumeStepDown(void);
void KEYS_VolumeStepUp(void);
void KEYS_TuneStepBack(void);
void KEYS_TuneStepForward(void);

/* */
void KEYS_Initialize(void)
{
    /* 1. setup timers for encoders */
    /* 2. setup ADC for keys control */

    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;

    /* -------------------------------------------------------------- */
    TIM1->CCMR1 = TIM_CCMR1_CC1S_0 | TIM_CCMR1_CC2S_1;
    TIM1->CCER = 0;
    TIM1->DIER = TIM_DIER_UIE;

    TIM1->PSC = 0;
    TIM1->ARR = 1;
    TIM1->CNT = 0;

    /* Use Encoder mode 2 */
    TIM1->SMCR = TIM_SMCR_SMS_0;

    /* -------------------------------------------------------------- */
    TIM4->CCMR1 = TIM_CCMR1_CC1S_0 | TIM_CCMR1_CC2S_1;
    TIM4->CCER = 0;
    TIM4->DIER = TIM_DIER_UIE;

    TIM4->PSC = 0;
    TIM4->ARR = 1;
    TIM4->CNT = 0;

    /* Use Encoder mode 2 */
    TIM4->SMCR = TIM_SMCR_SMS_0;

    /* -------------------------------------------------------------- */
    HAL_IRQ_Enable(TIM1_UP_TIM16_IRQn);
    HAL_IRQ_Enable(TIM4_IRQn);

    TIM1->CR1 = TIM_CR1_CEN;
    TIM4->CR1 = TIM_CR1_CEN;

    /* -------------------------------------------------------------- */
    // ADC4_IN4, ADC4_IN5
    RCC->CFGR2 &= ~(RCC_CFGR2_ADCPRE34);
    RCC->CFGR2 |= RCC_CFGR2_ADCPRE34_DIV2;
    RCC->AHBENR |= RCC_AHBENR_ADC34EN;

    /* Enable ADC voltage regulator and wait */
    ADC4->CR &= ~ADC_CR_ADVREGEN;   /* Exit from deep power down */
    ADC4->CR |= ADC_CR_ADVREGEN_0;  /* Pwr On */
    HAL_SysTick_Delay(10);  /* 10uS */

    /* Ensure that ADC is off */
    if (ADC4->CR & ADC_CR_ADEN)
        ADC4->CR &= ~ADC_CR_ADEN;

    ADC4->CR &= ~ADC_CR_ADCALDIF; /* Single-ended input */

    /* Start calibration process */
    ADC4->CR |= ADC_CR_ADCAL;
    while(ADC4->CR & ADC_CR_ADCAL);

    /* Enable ADC and wait for ready */
    ADC4->CR |= ADC_CR_ADEN;
    while ((ADC4->ISR & ADC_ISR_ADRDY) == 0);

    ADC4->CFGR |= ADC_CFGR_OVRMOD; /* Overwrite old data when OVR occurs */
    ADC4->CFGR |= ADC_CFGR_CONT; /* Continuous conversion mode */
    ADC4->CFGR |= ADC_CFGR_AUTDLY; /* Delayed conversion mode */

    ADC4->SQR1  = ADC_SQR1_L_0; /* 2 conversions */
    ADC4->SQR1 |= (4 << ADC_SQR1_SQ1_Pos) & ADC_SQR1_SQ1_Msk;
    ADC4->SQR1 |= (5 << ADC_SQR1_SQ2_Pos) & ADC_SQR1_SQ2_Msk;

    /* */
    const HAL_DMA_Map_t * lpDmaMap = HAL_DMA_GetMapping(ADC4, e_HAL_DMA_Periph2Mem);
    if (lpDmaMap != NULL)
    {
        DMA_Channel_t *lpDmaChan = (DMA_Channel_t*)lpDmaMap->pStreamBase;

        int nDmaChanIdx = HAL_DMA_Config(lpDmaMap, &g_pDmaCfg);
        if (nDmaChanIdx >= 0)
        {
            lpDmaChan->CCR &= ~DMA_CCR_EN;
            lpDmaChan->CPAR = (uint32_t)&ADC4->DR;
            lpDmaChan->CMAR = (uint32_t)&g_pAdcRawSamples;
            lpDmaChan->CNDTR = ARRAY_SIZE(g_pAdcRawSamples);
            lpDmaChan->CCR |= DMA_CCR_EN;

            ADC4->CFGR |= (ADC_CFGR_DMAEN | ADC_CFGR_DMACFG);
        }
        else
            LOGW("ADC4 config DMA channel %ld failed", lpDmaMap->nChannel);
    }
    else
        LOGW("ADC4 unable to find proper DMA mapping");

    /* */
    HAL_IRQ_Enable(DMA2_Channel2_IRQn);
    /* */
    ADC4->CR |= ADC_CR_ADSTART;
}

__attribute__((used))
    void HAL_IRQ_TIM1_Handler(void)
{
    if (TIM1->CR1 & TIM_CR1_DIR) {
        KEYS_VolumeStepDown();
    } else {
        KEYS_VolumeStepUp();
    }

    TIM1->SR &= ~(TIM_SR_UIF);
}

__attribute__((used))
    void HAL_IRQ_TIM4_Handler(void)
{
    if (TIM4->CR1 & TIM_CR1_DIR) {
        KEYS_TuneStepBack();
    } else {
        KEYS_TuneStepForward();
    }

    TIM4->SR &= ~(TIM_SR_UIF);
}

/* ------------------------------------------------------------------- */
bool KEYS_NextIsPressed(void)
{
    const uint16_t val = (g_pAdcRawSamples[0] >> 8);
    return (val > 6 && val <= 8);
}

bool KEYS_PrevIsPressed(void)
{
    const uint16_t val = (g_pAdcRawSamples[0] >> 8);
    return (val >= 4 && val < 6);
}

bool KEYS_PowerIsPressed(void) /* Encoder VOL/POWER */
{
    const uint16_t val = (g_pAdcRawSamples[0] >> 8);
    return (val > 9 && val <= 11);
}

/* ------------------------------------------------------------------- */
bool KEYS_BandIsPressed(void)
{
    const uint16_t val = (g_pAdcRawSamples[1] >> 8);
    return (val > 8 && val <= 10);
}

bool KEYS_NaviIsPressed(void)
{
    const uint16_t val = (g_pAdcRawSamples[1] >> 8);
    return (val > 6 && val <= 8);
}

bool KEYS_HomeIsPressed(void)
{
    const uint16_t val = (g_pAdcRawSamples[1] >> 8);
    return (val > 6 && val <= 8);
}

bool KEYS_EqIsPressed(void) /* Encoder TUNE/EQ */
{
    const uint16_t val = (g_pAdcRawSamples[1] >> 8);
    return (val > 9 && val <= 11);
}

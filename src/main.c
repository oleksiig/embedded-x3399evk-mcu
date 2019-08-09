/**
 * Brief:
 **/

#include <stdint.h>
#include <stdbool.h>

#define TAG "Main"

#include "utils/log.h"
#include "utils/intrinsics.h"

#include "hal/hal_mcu.h"
#include "hal/hal_systick.h"
#include "hal/hal_gpio.h"


/* -------------------------------------------------------------------- */
void I2S_ClockGatingInitialize(void);
void I2S_RefClkStart(void);
void I2S_RefClkStop(void);
void KEYS_Initialize(void);
void HCI_Initialize(void);

/* -------------------------------------------------------------------- */
int EntryPoint(void)
{
    HAL_SysTick_Start(SYSTICK_RATE_1MS);

    I2S_ClockGatingInitialize();

    /* ---------------------------------------------------------------- */
    while(1)
    {

    }
}

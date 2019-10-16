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

/* -------------------------------------------------------------------- */
void KEYS_Initialize(void);
bool KEYS_NextIsPressed(void);
bool KEYS_PrevIsPressed(void);
bool KEYS_BandIsPressed(void);
bool KEYS_NaviIsPressed(void);
bool KEYS_HomeIsPressed(void);

bool KEYS_PowerIsPressed(void); /* Encoder VOL/POWER */
void KEYS_VolumeStepDown(void) {}
void KEYS_VolumeStepUp(void) {}

bool KEYS_EqIsPressed(void); /* Encoder TUNE/EQ */
void KEYS_TuneStepBack(void) {}
void KEYS_TuneStepForward(void) {}
/* -------------------------------------------------------------------- */

void HCI_Initialize(void);

/* -------------------------------------------------------------------- */
int EntryPoint(void)
{
    I2S_ClockGatingInitialize();
    KEYS_Initialize();

    /* ---------------------------------------------------------------- */
    while(1)
    {
        KEYS_NextIsPressed();
    }
}

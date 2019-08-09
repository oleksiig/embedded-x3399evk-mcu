/**
 * Brief:
 **/

#include <stdint.h>

#include "utils/memory/heap.h"

#include "hal/hal_mcu.h"
#include "hal/hal_clock.h"
#include "hal/hal_gpio.h"
#include "hal/hal_dma.h"
#include "hal/hal_uart.h"

#include "drivers/console/console_serial.h"

void IRLinkIRQHandle(void);
void LockIRQHandle(void);

/* -------------------------------------------------------------------- */
/* OVERDRIVE OFF!! */
const HAL_ClkDesc_t g_pClkList[] =
{
    {  8000000,  (CLK_TYPE_XTAL  | CLK_MAIN_OSC)  },
    { 72000000,  (CLK_TYPE_PLL0  | CLK_SRC_XTAL)  },
    { 72000000,  (CLK_TYPE_AHB   | CLK_SRC_PLL0)  },
    { 36000000,  (CLK_TYPE_APB1)                  },
    { 72000000,  (CLK_TYPE_APB2)                  },
    { 0, 0 } /* End of list */
};

/* */
void ExtIRQ_Radio(void) { }
void ExtIRQ_Amplifier(void) { }

/* -------------------------------------------------------------------- */
const HAL_GpioPin_t g_pPinList[] =
{
    /* I2S ref clock generation  */
    GPIO_CONFIG(GPIOA, GPIO_BIT0,  GPIO_AF1,         GPIO_IN_FLOAT),             // TIM2_ETR - REF_12288MHZ
    GPIO_CONFIG(GPIOA, GPIO_BIT1,  GPIO_AF1,         GPIO_OUT_PUSH_PULL),        // TIM2_CH2 - I2S_REF_BCLK
    GPIO_CONFIG(GPIOA, GPIO_BIT2,  GPIO_AF9,         GPIO_OUT_PUSH_PULL),        // TIM15_CH1 - I2S_REF_LRCK
    GPIO_CONFIG(GPIOA, GPIO_BIT3,  GPIO_AF_NONE,     GPIO_OUT_PUSH_PULL_H),      // GPIO_OUT - REF_CLK_EN

    /* SPI host interface */
    GPIO_CONFIG(GPIOA, GPIO_BIT4,  GPIO_AF5,         GPIO_IN_PULL_UP),           // SPI1_NSS - SPI_SOC_CS#
    GPIO_CONFIG(GPIOA, GPIO_BIT5,  GPIO_AF5,         GPIO_IN_PULL_DOWN),         // SPI1_SCK - SPI_SOC_CLK
    GPIO_CONFIG(GPIOA, GPIO_BIT6,  GPIO_AF5,         GPIO_OUT_PUSH_PULL),        // SPI1_MISO - SPI_SOC_RX
    GPIO_CONFIG(GPIOA, GPIO_BIT7,  GPIO_AF5,         GPIO_IN_PULL_DOWN),         // SPI1_MOSI - SPI_SOC_TX
    GPIO_CONFIG(GPIOB, GPIO_BIT0,  GPIO_AF_NONE,     GPIO_OUT_PUSH_PULL_H),      // GPIO_OUT - SPI_SOC_INT#

    /* ENC_VOL */
    GPIO_CONFIG(GPIOA, GPIO_BIT8,  GPIO_AF6,         GPIO_IN_PULL_UP),           // TIM1_CH1 - ENC_VOL_A
    GPIO_CONFIG(GPIOA, GPIO_BIT9,  GPIO_AF6,         GPIO_IN_PULL_UP),           // TIM1_CH2 - ENC_VOL_B

    /* ENC _TUNE */
    GPIO_CONFIG(GPIOA, GPIO_BIT11, GPIO_AF10,        GPIO_IN_PULL_UP),           // TIM4_CH1 - ENC_TUNE_A
    GPIO_CONFIG(GPIOA, GPIO_BIT12, GPIO_AF10,        GPIO_IN_PULL_UP),           // TIM4_CH2 - ENC_TUNE_B

    /* KEYS */
    GPIO_CONFIG(GPIOB, GPIO_BIT14, GPIO_AF0,         GPIO_IN_ANALOG),            // ADC4_IN4 - KEY1
    GPIO_CONFIG(GPIOB, GPIO_BIT15, GPIO_AF0,         GPIO_IN_ANALOG),            // ADC4_IN5 - KEY2

    /* I2C1 */
    GPIO_CONFIG(GPIOB, GPIO_BIT6, GPIO_AF4,          GPIO_OUT_OPEN_DRAIN),       // SCL
    GPIO_CONFIG(GPIOB, GPIO_BIT7, GPIO_AF4,          GPIO_OUT_OPEN_DRAIN),       // SDA

    /* CAN1 */
    GPIO_CONFIG(GPIOB, GPIO_BIT8, GPIO_AF9,          GPIO_IN_PULL_DOWN),         // RX
    GPIO_CONFIG(GPIOB, GPIO_BIT9, GPIO_AF9,          GPIO_OUT_PUSH_PULL),        // TX

    /* USART3 */
    GPIO_CONFIG(GPIOB, GPIO_BIT10, GPIO_AF7,         GPIO_OUT_PUSH_PULL),        // TX
    GPIO_CONFIG(GPIOB, GPIO_BIT11, GPIO_AF7,         GPIO_IN_PULL_DOWN),         // RX

    /* Radio controls */
    GPIO_CONFIG(GPIOB, GPIO_BIT3,  GPIO_AF_NONE,     GPIO_OUT_PUSH_PULL_H),      // GPIO_OUT - RADIO_PWEN
    GPIO_CONFIG(GPIOB, GPIO_BIT5,  GPIO_AF_NONE,     GPIO_OUT_PUSH_PULL),        // GPIO_OUT - RADIO_RST#

    GPIO_CONFIG_IRQ(GPIOB,  GPIO_BIT4, GPIO_AF_NONE, GPIO_IN_PULL_UP |           // EXTI - RADIO_INT#
        GPIO_IRQ_FALLING_EDGE, ExtIRQ_Radio),

    /* AMP controls */
    GPIO_CONFIG(GPIOA, GPIO_BIT10, GPIO_AF_NONE,     GPIO_OUT_PUSH_PULL),        // GPIO_OUT - AMP_MUTE#
    GPIO_CONFIG(GPIOB, GPIO_BIT13, GPIO_AF_NONE,     GPIO_OUT_PUSH_PULL),        // GPIO_OUT - AMP_PWEN

    GPIO_CONFIG_IRQ(GPIOB,  GPIO_BIT12, GPIO_AF_NONE, GPIO_IN_PULL_UP |          // EXTI - AMP_INT#
        GPIO_IRQ_FALLING_EDGE, ExtIRQ_Amplifier),

    /* ADSP controls */
    GPIO_CONFIG(GPIOA, GPIO_BIT15,  GPIO_AF_NONE,    GPIO_OUT_PUSH_PULL_H),        // GPIO_OUT - ADSP_PWEN

    /* AUX */
    GPIO_CONFIG(GPIOB, GPIO_BIT1,  GPIO_AF_NONE,     GPIO_IN_PULL_UP),           // GPIO_OUT - AUX_GPIO0
    GPIO_CONFIG(GPIOB, GPIO_BIT2,  GPIO_AF_NONE,     GPIO_IN_PULL_UP),           // GPIO_OUT - AUX_GPIO1

    /* End of list */
    GPIO_CONFIG(NULL, 0, 0, 0)
};

/* ------------------------------------------------------------------ */
const HAL_DMA_Map_t g_pDMA_MapTable[] =
{
    /* End of list */
    DMA_MAP(NULL, NULL, NULL, 0, 0)
};

/* ------------------------------------------------------------------ */
const DRV_SerialConsoleDesc_t g_pSerialConsole =
{
    .pPort          = USART3,
    .nBaudrate      = 115200
};

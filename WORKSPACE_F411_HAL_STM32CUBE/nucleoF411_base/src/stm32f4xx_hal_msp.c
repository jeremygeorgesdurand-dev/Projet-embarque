// MCU Support Package

#include "stm32f4xx_hal_msp.h"

#include "stm32f4xx_hal_gpio_ex.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_hal_gpio.h"
#include "board.h"

#define I2C1_ER_IRQ_PRIO    2
#define I2C1_EV_IRQ_PRIO    3
#define TIM5_IRQ_PRIO       8
#define EXTI15_10_IRQ_PRIO  5
#define EXTI4_IRQ_PRIO      6
#define USART2_IRQ_PRIO     7
#define SPI1_IRQ_PRIO		4

void HAL_GPIO_LEDS_MspInit(void);
void HAL_TIM2_MspInit(void);
void HAL_TIM5_MspInit(void);
void HAL_GPIO_SWITCHS_MspInit(void);
void HAL_UART2_MspInit(void);
void HAL_I2C1_MspInit(void);
void HAL_EXTI_MspInit(void);
void HAL_GPIO_SWITCH_UP_EXTI_MspInit(void);
void HAL_LCD_MspInit(void);

//===========================================================
void HAL_MspInit(void)
{
	  __GPIOA_CLK_ENABLE();
	  __GPIOB_CLK_ENABLE();
	  __GPIOC_CLK_ENABLE();

	  HAL_GPIO_LEDS_MspInit();
	  HAL_GPIO_SWITCHS_MspInit();
	  HAL_TIM2_MspInit();
	  HAL_TIM5_MspInit();
	  HAL_UART2_MspInit();
	  HAL_LCD_MspInit();

}
//===========================================================
// LED_RED   --> PB4
// LED_GREEN --> PC7
// LED_BLUE  --> PA9
void HAL_GPIO_LEDS_MspInit(void)
{
	  GPIO_InitTypeDef  GPIO_InitStruct;
}
//===========================================================
// SW_UP     --> PA4
// SW_DOWN   --> PB0
// SW_CENTER --> PB5
// SW_RIGHT  --> PC0
// SW_LEFT   --> PC1
void HAL_GPIO_SWITCHS_MspInit()
{

}

//===========================================================
void  HAL_TIM2_MspInit(void)
{
	__TIM2_CLK_ENABLE();
}

void  HAL_TIM5_MspInit(void)
{
	__TIM5_CLK_ENABLE();
}

//===========================================================
//			USART2 : ST-LINK virtual Port
// TX --> PA2
// RX --> PA3
//===========================================================
void HAL_UART2_MspInit(void)
{
	// A COMPLETER
}
//===========================================================

/* PINS
 *
 *  LCD_A0    --> D7  --> PA8 : GPIO
 *  LCD_CS_N  --> D10 --> PB6 : GPIO
 *  LCD_RESET --> D12 --> PA6 : GPIO
 *
 *  LCD_MOSI  --> D11 --> PA7 : SPI1_MOSI
 *  LCD_SCK   --> D13 --> PA5 : SPI1_SCK
 */
void HAL_LCD_MspInit()
{
	  GPIO_InitTypeDef  GPIO_InitStruct;

	  GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_8;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_MEDIUM;
	  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	  GPIO_InitStruct.Pin = GPIO_PIN_6;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_MEDIUM;
	  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	__SPI1_CLK_ENABLE();

	GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP ;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStruct.Alternate =   GPIO_AF5_SPI1 ; // hal_gpio_ex.h

	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);


	  NVIC_SetPriority(SPI1_IRQn, SPI1_IRQ_PRIO);
	  NVIC_EnableIRQ(SPI1_IRQn);


}


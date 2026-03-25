#include "stm32f4xx_hal_msp.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_tim.h"
#include "stm32f4xx_hal_uart.h"
#include "stm32f4xx_hal_i2c.h"
#include "leds.h"
#include "sw.h"
#include "lm75.h"
#include "stm32f4xx_hal.h"

TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim5;
UART_HandleTypeDef huart2;
I2C_HandleTypeDef hi2c1;

//===========================================================
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	switch(GPIO_Pin)
	{
	}
}
//============================================================
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart == &huart2)
	{
	}
}
//============================================================
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{

}
//============================================================
int main()
{
	HAL_Init();
	HAL_MspInit(); // Initialisation des Broches


	while(1)
	{


		HAL_Delay(100);
	}

	return 0;
}
//============================================================


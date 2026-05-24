#include "stm32f4xx_hal_msp.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_tim.h"
#include "stm32f4xx_hal_uart.h"
#include "stm32f4xx_hal_i2c.h"
#include "leds.h"
#include "sw.h"
#include "lm75.h"
#include "stm32f4xx_hal.h"
#include "bmp280.h"
#include <stdio.h> 

TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim5;
UART_HandleTypeDef huart2;
I2C_HandleTypeDef hi2c1;

BMP280_HandleTypedef bmp280;

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
int main(void) {
    HAL_Init();
    HAL_MspInit();

	
	huart2.Instance          = USART2;
	huart2.Init.BaudRate     = 115200;
	huart2.Init.WordLength   = UART_WORDLENGTH_8B;
	huart2.Init.StopBits     = UART_STOPBITS_1;
	huart2.Init.Parity       = UART_PARITY_NONE;
	huart2.Init.Mode         = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;

	HAL_UART_Init(&huart2);

	char *test = "UART OK\r\n";
	HAL_UART_Transmit(&huart2, (uint8_t*)test, 9);
	HAL_Delay(100);

	hi2c1.Instance             = I2C1;
	hi2c1.Init.ClockSpeed      = 100000;  // 100 kHz
	hi2c1.Init.DutyCycle       = 0;       // I2C_DUTYCYCLE_2 = 0 dans le registre CCR
	hi2c1.Init.OwnAddress1     = 0;
	hi2c1.Init.AddressingMode  = 0;       // 7-bit = 0 dans le registre OAR1
	hi2c1.Init.DualAddressMode = 0;       // désactivé = 0
	hi2c1.Init.OwnAddress2     = 0;
	hi2c1.Init.GeneralCallMode = 0;       // désactivé = 0
	hi2c1.Init.NoStretchMode   = 0;       // stretch activé = 0
	HAL_I2C_Init(&hi2c1);

	char scan_msg[32];
	for (uint8_t addr = 0x75; addr < 0x78; addr++) {
    uint8_t dummy = 0xD0;
    if (HAL_I2C_Master_Transmit_IT(&hi2c1, addr, &dummy, 1, 1000) == I2C_OK) {
        int l = snprintf(scan_msg, sizeof(scan_msg), "Found: 0x%02X\r\n", addr);
        HAL_UART_Transmit(&huart2, (uint8_t*)scan_msg, l);
    }
    HAL_Delay(10);
}

	char *test2 = "I2C INIT OK\r\n";
	HAL_UART_Transmit(&huart2, (uint8_t*)test2, 13);
	HAL_Delay(100);

    // Init BMP280
    bmp280_params_t params;
    bmp280_init_default_params(&params);
    bmp280.i2c  = &hi2c1;
    bmp280.addr = BMP280_I2C_ADDRESS_1;  // 0x77 

    if (!bmp280_init(&bmp280, &params)) {
        // Erreur : blink LED d'erreur
        while(1) { HAL_Delay(500); }
    }

	if (!bmp280_init(&bmp280, &params)) {
    char *err = "BMP280 FAIL\r\n";
    HAL_UART_Transmit(&huart2, (uint8_t*)err, 13);
    while(1) { HAL_Delay(500); }
	}

	char *ok = "BMP280 OK\r\n";
	HAL_UART_Transmit(&huart2, (uint8_t*)ok, 11);

float pressure = 0.0f, temperature = 0.0f;
char msg[64];

while (1) {
    bool ok = bmp280_read_float(&bmp280, &temperature, &pressure, NULL);

    if (ok) {
        int32_t press_int = (int32_t)(pressure / 100.0f);
        int32_t press_dec = (int32_t)(pressure / 1.0f) % 100;
        int32_t temp_int  = (int32_t)(temperature);
        int32_t temp_dec  = (int32_t)(temperature * 100) % 100;
        if (temp_dec < 0) temp_dec = -temp_dec;

        int len = snprintf(msg, sizeof(msg), "%lu;%ld.%02ld;%ld.%02ld\r\n",
                           HAL_GetTick(), press_int, press_dec, temp_int, temp_dec);
        HAL_UART_Transmit(&huart2, (uint8_t*)msg, len);
    } else {
        char *err = "READ FAIL\r\n";
        HAL_UART_Transmit(&huart2, (uint8_t*)err, 10);
    }

    HAL_Delay(500);
}
}
//============================================================


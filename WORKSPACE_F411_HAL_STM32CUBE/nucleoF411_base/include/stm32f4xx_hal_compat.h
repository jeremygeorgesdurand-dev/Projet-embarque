// include/stm32f4xx_hal_compat.h
#ifndef STM32F4XX_HAL_COMPAT_H
#define STM32F4XX_HAL_COMPAT_H

#include "stm32f4xx_hal_i2c.h"

#define HAL_OK  I2C_OK

// Signature réelle : (hi2c, DevAddress, pData, nwr, nrd, Timeout)
// pData sert à la fois pour l'envoi (nwr octets) ET la réception (nrd octets)

static inline int HAL_I2C_Mem_Read(I2C_HandleTypeDef *hi2c,
        uint16_t DevAddr, uint16_t MemAddr, uint16_t MemAddrSize,
        uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
    uint8_t buf[32];  // taille fixe suffisante pour BMP280 (max 26 octets calib)
    buf[0] = (uint8_t)MemAddr;
    
    int ret = HAL_I2C_Master_Transmit_Receive_IT(hi2c, DevAddr >> 1,
                  buf, 1, Size, Timeout);
    
    for (uint16_t i = 0; i < Size; i++) pData[i] = buf[i];
    return ret;
}

static inline int HAL_I2C_Mem_Write(I2C_HandleTypeDef *hi2c,
        uint16_t DevAddr, uint16_t MemAddr, uint16_t MemAddrSize,
        uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
    uint8_t buf[Size + 1];
    buf[0] = (uint8_t)MemAddr;
    for (uint16_t i = 0; i < Size; i++) buf[i + 1] = pData[i];
    return HAL_I2C_Master_Transmit_IT(hi2c, DevAddr >> 1, buf, Size + 1, Timeout);
}

#endif
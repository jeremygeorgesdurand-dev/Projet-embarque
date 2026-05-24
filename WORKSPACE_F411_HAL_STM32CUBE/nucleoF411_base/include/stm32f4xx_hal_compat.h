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
    // On place l'adresse registre dans pData[0], la réponse écrasera ensuite
    pData[0] = (uint8_t)MemAddr;
    return HAL_I2C_Master_Transmit_Receive_IT(hi2c, DevAddr,
               pData, 1, Size, Timeout);
}

static inline int HAL_I2C_Mem_Write(I2C_HandleTypeDef *hi2c,
        uint16_t DevAddr, uint16_t MemAddr, uint16_t MemAddrSize,
        uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
    // Construit [registre, valeur] dans un buffer temporaire
    uint8_t buf[Size + 1];
    buf[0] = (uint8_t)MemAddr;
    for (uint16_t i = 0; i < Size; i++) buf[i + 1] = pData[i];
    return HAL_I2C_Master_Transmit_IT(hi2c, DevAddr, buf, Size + 1, Timeout);
}

#endif
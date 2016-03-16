/*
 * ws2812b.h
 *
 *  Created on: 05 апр. 2014 г.
 *      Author: Kreyl
 */

#pragma once

#include "ch.h"
#include "hal.h"
#include "kl_lib.h"
#include "color.h"
#include "uart.h"

#define LED_DMA_MODE    DMA_PRIORITY_VERYHIGH \
                        | STM32_DMA_CR_MSIZE_HWORD \
                        | STM32_DMA_CR_PSIZE_HWORD \
                        | STM32_DMA_CR_MINC     /* Memory pointer increase */ \
                        | STM32_DMA_CR_DIR_M2P  /* Direction is memory to peripheral */ \
                        | STM32_DMA_CR_TCIE     /* Enable Transmission Complete IRQ */

// Tx timings: bit cnt
#define SEQ_1               0b110
#define SEQ_0               0b100
#define SEQ_LEN             3
#define RST_W_CNT_4MHz      9 // zero words to produce reset

// SPI16 Buffer (no tuning required)
#define DATA_BIT_CNT        (LED_CNT * 3 * 8 * SEQ_LEN)   // Each led has 3 channels 8 bit each
#define DATA_W_CNT          ((DATA_BIT_CNT + 15) / 16)
#define TOTAL_W_CNT         (RST_W_CNT_4MHz + DATA_W_CNT + 1)

class LedWs_t {
private:
    Spi_t ISpi;
    uint16_t IBuf[TOTAL_W_CNT];
    uint16_t *PBuf, BitsLeft;
    void AppendBitsMadeOfByte(uint8_t Byte);
    void AppendOnes();
public:
    void Init();
    void OnAHBFreqChange();
    // Inner use
    Color_t ICurrentClr[LED_CNT];
    void ISetCurrentColors();
//    void IStopTx() { TxTmr.Set(0); TxTmr.Disable(); }
    void ITmrHandlerI();
};

extern LedWs_t LedWs;

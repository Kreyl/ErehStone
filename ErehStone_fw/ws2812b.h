/*
 * ws2812b.h
 *
 *  Created on: 05 ���. 2014 �.
 *      Author: Kreyl
 */

#pragma once

#include "ch.h"
#include "hal.h"
#include "kl_lib.h"
#include "color.h"
#include "uart.h"

#define LED_DMA_MODE    DMA_PRIORITY_VERYHIGH \
                        | STM32_DMA_CR_MSIZE_BYTE \
                        | STM32_DMA_CR_PSIZE_HWORD \
                        | STM32_DMA_CR_MINC     /* Memory pointer increase */ \
                        | STM32_DMA_CR_DIR_M2P  /* Direction is memory to peripheral */ \
                        | STM32_DMA_CR_TCIE     /* Enable Transmission Complete IRQ */

// Bit Buffer (no tuning required)
#define RST_BIT_CNT         27 // zero bits to produce reset
#define DATA_BIT_CNT        (LED_CNT * 3 * 8)   // Each led has 3 channels 8 bit each
#define TOTAL_BIT_CNT       (DATA_BIT_CNT + RST_BIT_CNT)

// Tx timings
#define T0H_N_4MHz          1
#define T1H_N_4MHz          3
#define T_TOTAL_N_4MHz      18

#define T0H_N_16MHz         4
#define T1H_N_16MHz         12
#define T_TOTAL_N_16MHz     27

#define T0H_N_24MHz         10
#define T1H_N_24MHz         19
#define T_TOTAL_N_24MHz     27

#if 1 // ============================= LED Params ==============================
class LedParams_t {
public:
    union {
        struct {
            int32_t Indx;
            int32_t R, G, B;
            int32_t Smooth;
        };
        int32_t Arr[5];
    };
    uint8_t Check(Shell_t *PShell) {
        uint8_t Rslt = OK;
        if(Indx < 1 or Indx > 3) {
            PShell->Printf("Bad indx: %d\r\n", Indx);
            Rslt = FAILURE;
        }
        if(R < 0 or R > 255) {
            PShell->Printf("Bad R: %d\r\n", R);
            Rslt = FAILURE;
        }
        if(G < 0 or G > 255) {
            PShell->Printf("Bad G: %d\r\n", G);
            Rslt = FAILURE;
        }
        if(B < 0 or B > 255) {
            PShell->Printf("Bad B: %d\r\n", B);
            Rslt = FAILURE;
        }
        if(Smooth < 0) {
            PShell->Printf("Bad Smooth: %d\r\n", Smooth);
            Rslt = FAILURE;
        }
        return Rslt;
    }
    void Print() { Uart.Printf("%u  %u %u %u   %u\r", Indx, R, G, B, Smooth); }
};
#endif

class LedWs_t {
private:
    PinOutputPWM_t<T_TOTAL_N_4MHz, invNotInverted, omOpenDrain> TxTmr{LEDWS_GPIO, LEDWS_PIN, LEDWS_TMR, LEDWS_TMR_CH};
    uint8_t T0H_N = T0H_N_4MHz, T1H_N = T1H_N_4MHz;
    uint8_t BitBuf[TOTAL_BIT_CNT];
    uint8_t *PBit;
    void AppendBitsMadeOfByte(uint8_t Byte);
public:
    void Init();
    void OnAHBFreqChange();
    // Inner use
    Color_t ICurrentClr[LED_CNT];
    void ISetCurrentColors();
    void IStopTx() { TxTmr.Set(0); TxTmr.Disable(); }
    void ITmrHandlerI();
};

extern LedWs_t LedWs;

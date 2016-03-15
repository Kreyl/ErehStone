/*
 * ws2812b.cpp
 *
 *  Created on: 05 апр. 2014 г.
 *      Author: Kreyl
 */

#include "ws2812b.h"
#include "evt_mask.h"
#include "main.h"

LedWs_t LedWs;

extern "C" {
// Wrapper for Tx Completed IRQ
void LedTxcIrq(void *p, uint32_t flags) {
    dmaStreamDisable(LEDWS_DMA);
    LedWs.IStopTx();
//    Uart.PrintfI("Irq\r");
}
static inline void LedTmrCallback(void *p) {
    chSysLockFromISR();
    LedWs.ITmrHandlerI();
    chSysUnlockFromISR();
}
} // "C"

void LedWs_t::Init() {
    // ==== Timer ====
#if defined LED_REMAP_TIM15 // Remap T15CH2 to PB15
    bool AfioWasEnabled = (RCC->APB2ENR & RCC_APB2ENR_AFIOEN);
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;     // Enable AFIO
    AFIO->MAPR2 |= AFIO_MAPR2_TIM15_REMAP;
    if (!AfioWasEnabled) RCC->APB2ENR &= ~RCC_APB2ENR_AFIOEN;
#endif
    // Init tmr in PWM mode
    TxTmr.Init();
    TxTmr.Set(0);
    TxTmr.EnableDmaOnUpdate();
    OnAHBFreqChange();
    TxTmr.Disable();

//    TIM3->PSC = 9;

    // Zero buffer
    for(uint32_t i=DATA_BIT_CNT; i<TOTAL_BIT_CNT; i++) BitBuf[i] = 0;

    // ==== DMA ====
    dmaStreamAllocate     (LEDWS_DMA, IRQ_PRIO_LOW, LedTxcIrq, NULL);
    dmaStreamSetPeripheral(LEDWS_DMA, TMR_PCCR(LEDWS_TMR, LEDWS_TMR_CH));
    dmaStreamSetMemory0   (LEDWS_DMA, BitBuf);
    dmaStreamSetMode      (LEDWS_DMA, LED_DMA_MODE);
}

void LedWs_t::OnAHBFreqChange() {
    TxTmr.Disable();
    uint32_t Freq = Clk.GetTmrClkFreq(LEDWS_TMR);
    if(Freq == 4000000) {
        TxTmr.SetTopValue(T_TOTAL_N_4MHz);
        T0H_N = T0H_N_4MHz;
        T1H_N = T1H_N_4MHz;
    }
    else if(Freq == 16000000) {
        TxTmr.SetTopValue(T_TOTAL_N_16MHz);
        T0H_N = T0H_N_16MHz;
        T1H_N = T1H_N_16MHz;
    }
    else if(Freq == 24000000) {
        TxTmr.SetTopValue(T_TOTAL_N_24MHz);
        T0H_N = T0H_N_24MHz;
        T1H_N = T1H_N_24MHz;
    }
    TxTmr.GenerateUpdateEvt();
    TxTmr.Enable();
}
/*
void LedWs_t::SetCommonColorSmoothly(Color_t Clr, uint32_t Smooth, ClrSetupMode_t AMode) {
    chVTReset(&ITmr);
    IMode = AMode;
    for(uint32_t i=0; i<LED_CNT; i++) {
        DesiredClr[i] = Clr;
        SmoothValue[i] = Smooth;
    }
    Indx = 0;   // Start with first LED
    chSysLock();
    ITmrHandlerI();
    chSysUnlock();
}

void LedWs_t::StartProcess(ClrSetupMode_t AMode) {
    IMode = AMode;
    Indx = 0;   // Start with first LED
    chSysLock();
    ITmrHandlerI();
    chSysUnlock();
}

void LedWs_t::ITmrHandlerI() {
    while(IClr[Indx] == DesiredClr[Indx]) {
        Indx++;
        if(Indx >= LED_CNT) {
            App.SignalEvtI(EVT_LEDS_DONE);
            return; // Setup completed
        }
    }
    // Adjust color(s) and calculate delay depending on mode
    uint32_t Delay = 0;
    if(IMode == csmOneByOne) {
        while(SmoothValue[Indx] == 0) {
            IClr[Indx] = DesiredClr[Indx];
            Indx++;
            if(Indx >= LED_CNT) {
                App.SignalEvtI(EVT_LEDS_DONE);
                return; // Setup completed
            }
        } // while
        Delay = ICalcDelayByIndx(Indx);
        IClr[Indx].Adjust(&DesiredClr[Indx]);
    }
    // Simultaneously
    else {
        uint32_t tmp;
        for(uint8_t i=Indx; i<LED_CNT; i++) {
            if(SmoothValue[i] == 0) IClr[i] = DesiredClr[i];
            else {
                tmp = ICalcDelayByIndx(i);      // }
                if(tmp > Delay) Delay = tmp;    // } Calculate Delay
                IClr[i].Adjust(&DesiredClr[i]); // Adjust color
            }
        } // for
//        Uart.PrintfI("D=%u\r", Delay);
        ISetCurrentColors();
        if(Delay == 0) {
            App.SignalEvtI(EVT_LEDS_DONE);
            return; // Setup completed
        }
    }
//    Uart.Printf("I=%u; D=%u\r", Indx, Delay);
    chVTSetI(&ITmr, MS2ST(Delay), LedTmrCallback, NULL);
}
*/

void LedWs_t::AppendBitsMadeOfByte(uint8_t Byte) {
    for(uint8_t i=0; i<8; i++) {
        if(Byte & 0x80) *PBit = T1H_N;
        else *PBit = T0H_N;
        PBit++;
        Byte <<= 1;
    }
}

void LedWs_t::ISetCurrentColors() {
    TxTmr.Disable();
    TxTmr.SetCounter(0);
    TxTmr.GenerateUpdateEvt();

    //    for(uint8_t i=0; i<LED_CNT; i++) Uart.PrintfI("* %u %u %u\r", IClr[i].R, IClr[i].G, IClr[i].B);
//    Uart.PrintfI("\r");
    // Fill bit buffer
    PBit = &BitBuf[RST_BIT_CNT];
//    PBit = BitBuf;  // debug
    for(uint32_t i=0; i<LED_CNT; i++) {
        AppendBitsMadeOfByte(ICurrentClr[i].G);
        AppendBitsMadeOfByte(ICurrentClr[i].R);
        AppendBitsMadeOfByte(ICurrentClr[i].B);
    }

    // Change first 1 duration
    BitBuf[0] -= 2;

//    for(uint8_t i=0; i<TOTAL_BIT_CNT; i++) Uart.PrintfI("%u ", BitBuf[i]);
//    Uart.PrintfI("\r");

    // Start transmission
    dmaStreamSetTransactionSize(LEDWS_DMA, TOTAL_BIT_CNT);
    dmaStreamSetMode(LEDWS_DMA, LED_DMA_MODE);
    dmaStreamEnable(LEDWS_DMA);

//    chSysLockFromIsr();
    TxTmr.Enable();
}

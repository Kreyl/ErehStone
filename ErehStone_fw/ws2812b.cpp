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
//    Uart.PrintfI("Irq\r");
}
} // "C"

void LedWs_t::Init() {
    PinSetupAlterFunc(LEDWS_GPIO, LEDWS_PIN, omOpenDrain, pudNone, AF5, psHigh);
    ISpi.Setup(LEDWS_SPI, boMSB, cpolIdleLow, cphaFirstEdge, sbFdiv2, bitn16);
    ISpi.Enable();
    ISpi.EnableTxDma();

//    OnAHBFreqChange();

    // Zero buffer
    for(uint32_t i=0; i<RST_W_CNT; i++) IBuf[i] = 0;

    // ==== DMA ====
    dmaStreamAllocate     (LEDWS_DMA, IRQ_PRIO_LOW, LedTxcIrq, NULL);
    dmaStreamSetPeripheral(LEDWS_DMA, &LEDWS_SPI->DR);
    dmaStreamSetMode      (LEDWS_DMA, LED_DMA_MODE);
}

void LedWs_t::OnAHBFreqChange() {
//    TxTmr.Disable();
//    uint32_t Freq = Clk.GetTmrClkFreq(LEDWS_TMR);
//    if(Freq == 4000000) {
//        TxTmr.SetTopValue(T_TOTAL_N_4MHz);
//        T0H_N = T0H_N_4MHz;
//        T1H_N = T1H_N_4MHz;
//    }
//    else if(Freq == 16000000) {
//        TxTmr.SetTopValue(T_TOTAL_N_16MHz);
//        T0H_N = T0H_N_16MHz;
//        T1H_N = T1H_N_16MHz;
//    }
//    else if(Freq == 24000000) {
//        TxTmr.SetTopValue(T_TOTAL_N_24MHz);
//        T0H_N = T0H_N_24MHz;
//        T1H_N = T1H_N_24MHz;
//    }
//    TxTmr.GenerateUpdateEvt();
//    TxTmr.Enable();
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
    uint16_t seq;
    for(uint8_t i=0; i<8; i++) {
        if(Byte & 0x80) seq = SEQ_1;
        else seq = SEQ_0;
        Byte <<= 1;
        // Append sequence
        uint16_t w = *PBuf;
        if(BitsLeft == 0) {
            PBuf++;
            BitsLeft = 16;
            w = 0;   // reset all bits
        }
        else if(BitsLeft == 16) w = 0;   // reset all bits

        if(BitsLeft >= SEQ_LEN) {
            uint32_t Shift = BitsLeft - SEQ_LEN;
            w |= seq << Shift;
            BitsLeft -= SEQ_LEN;
            *PBuf = w;
        }
        else {
            uint32_t Shift = SEQ_LEN - BitsLeft;
            w |= seq >> Shift;
            *PBuf++ = w;
            Shift = 16 - Shift;
            w = 0;
            w |= seq << Shift;
            *PBuf = w;
            BitsLeft = Shift;
        }
    } // for
}

void LedWs_t::AppendOnes() {
    if(BitsLeft != 0) {
        uint16_t w = 0;
        for(uint8_t i=0; i<BitsLeft; i++) {
            w <<= 1;
            w |= 1;
        }
        *PBuf |= w;
    }
    PBuf++;
    while(PBuf < &IBuf[TOTAL_W_CNT]) *PBuf++ = 0xFFFF;
}

void LedWs_t::ISetCurrentColors() {
    // Fill bit buffer
    PBuf = &IBuf[RST_W_CNT];
    BitsLeft = 16;
    for(uint32_t i=0; i<LED_CNT; i++) {
        AppendBitsMadeOfByte(ICurrentClr[i].G);
        AppendBitsMadeOfByte(ICurrentClr[i].R);
        AppendBitsMadeOfByte(ICurrentClr[i].B);
    }
    AppendOnes();

    // Start transmission
    dmaStreamSetMemory0(LEDWS_DMA, IBuf);
    dmaStreamSetTransactionSize(LEDWS_DMA, TOTAL_W_CNT);
    dmaStreamSetMode(LEDWS_DMA, LED_DMA_MODE);
    dmaStreamEnable(LEDWS_DMA);
}

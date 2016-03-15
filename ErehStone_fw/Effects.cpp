/*
 * Effects.cpp
 *
 *  Created on: 15 марта 2016 г.
 *      Author: Kreyl
 */

#include "Effects.h"
#include "ws2812b.h"
#include "main.h"

Effects_t Effects;

static THD_WORKING_AREA(waEffectsThread, 256);
__noreturn
static void EffectsThread(void *arg) {
    chRegSetThreadName("Effects");
    Effects.ITask();
}

__noreturn
void Effects_t::ITask() {
    while(true) {
        switch(IState) {
            case effIdle: chThdSleep(TIME_INFINITE); break;

            case effAllSmoothly: {
                uint32_t Delay = 0;
                for(uint8_t i=0; i<LED_CNT; i++) {
                    uint32_t tmp = ICalcDelayN(i);  // }
                    if(tmp > Delay) Delay = tmp;    // } Calculate Delay
                    LedWs.ICurrentClr[i].Adjust(&DesiredClr[i]); // Adjust current color
                } // for
                LedWs.ISetCurrentColors();
                if(Delay == 0) {    // Setup completed
                    App.SignalEvt(EVT_LEDS_DONE);
                    IState = effIdle;
                }
                else chThdSleepMilliseconds(Delay);
            } break;
        } // switch
    } // while true
}

void Effects_t::Init() {
    LedWs.Init();
    // Thread
    PThd = chThdCreateStatic(waEffectsThread, sizeof(waEffectsThread), HIGHPRIO, (tfunc_t)EffectsThread, NULL);
}


void Effects_t::AllTogetherNow(Color_t Color) {
    IState = effIdle;
    for(uint32_t i=0; i<LED_CNT; i++) LedWs.ICurrentClr[i] = Color;
    LedWs.ISetCurrentColors();
    App.SignalEvt(EVT_LEDS_DONE);
}

void Effects_t::AllTogetherSmoothly(Color_t Color, uint32_t ASmoothValue) {
    if(ASmoothValue == 0) AllTogetherNow(Color);
    else {
        chSysLock();
        for(uint32_t i=0; i<LED_CNT; i++) {
            DesiredClr[i] = Color;
            SmoothValue[i] = ASmoothValue;
        }
        IState = effAllSmoothly;
        chSchWakeupS(PThd, MSG_OK);
        chSysUnlock();
    }
}

uint32_t Effects_t::ICalcDelayN(uint32_t n) {
    uint32_t DelayR = (LedWs.ICurrentClr[n].R == DesiredClr[n].R)? 0 : CalcDelay(LedWs.ICurrentClr[n].R, SmoothValue[n]);
    uint32_t DelayG = (LedWs.ICurrentClr[n].G == DesiredClr[n].G)? 0 : CalcDelay(LedWs.ICurrentClr[n].G, SmoothValue[n]);
    uint32_t DelayB = (LedWs.ICurrentClr[n].B == DesiredClr[n].B)? 0 : CalcDelay(LedWs.ICurrentClr[n].B, SmoothValue[n]);
//    Uart.Printf("I=%u; R=%u/%u; G=%u/%u; B=%u/%u\r",
//            Indx,
//            IClr[Indx].Red,   DesiredClr[Indx].Red,
//            IClr[Indx].Green, DesiredClr[Indx].Green,
//            IClr[Indx].Blue,  DesiredClr[Indx].Blue);
//    Uart.Printf("DR=%u; DG=%u; DB=%u\r", DelayR, DelayG, DelayB);
    uint32_t Rslt = DelayR;
    if(DelayG > Rslt) Rslt = DelayG;
    if(DelayB > Rslt) Rslt = DelayB;
    return Rslt;
}

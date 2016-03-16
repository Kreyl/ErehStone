/*
 * Effects.cpp
 *
 *  Created on: 15 марта 2016 г.
 *      Author: Kreyl
 */

#include "Effects.h"
#include "main.h"

Effects_t Effects;

#define CHUNK_CNT   3
static LedChunk_t Chunk[CHUNK_CNT] = {
        {5, 0},
        {6, 10},
        {11, 16}
};

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

            case effChunkRunningRandom: IProcessChunkRandom(); break;
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

void Effects_t::ChunkRunningRandom(Color_t Color, uint32_t NLeds, uint32_t ASmoothValue) {
    chSysLock();
    for(uint32_t i=0; i<CHUNK_CNT; i++) {
        Chunk[i].Color = Color;
        Chunk[i].NLeds = NLeds;
        Chunk[i].StartOver();
    }
    for(uint32_t i=0; i<LED_CNT; i++) {
        SmoothValue[i] = ASmoothValue;
    }
    IState = effChunkRunningRandom;
    chSchWakeupS(PThd, MSG_OK);
    chSysUnlock();
}

uint32_t t=0;

void Effects_t::IProcessChunkRandom() {
    uint32_t Delay = 0;
    for(uint32_t i=0; i<CHUNK_CNT; i++) {
        uint32_t ChunkDelay = Chunk[i].ProcessAndGetDelay();
        if(ChunkDelay > Delay) Delay = ChunkDelay;
    }
    LedWs.ISetCurrentColors();
    chThdSleepMilliseconds(Delay);
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

#if 1 // ============================== LedChunk ===============================
uint32_t LedChunk_t::ProcessAndGetDelay() {
    if(LedWs.ICurrentClr[Head] == Color) {   // Go on if done with current
        GetNext(&Head);
        GetNext(&Tail);
        Effects.DesiredClr[Head] = Color;
        Effects.DesiredClr[Tail] = clBlack;
    }
    // Iterate Leds
    uint32_t Delay = 0;
    int n = Start;
    do {
        uint32_t tmp = Effects.ICalcDelayN(n);  // }
        if(tmp > Delay) Delay = tmp;            // } Calculate Delay
        if(Delay!= 0) LedWs.ICurrentClr[n].Adjust(&Effects.DesiredClr[n]); // Adjust current color
    } while(GetNext(&n) == OK);
    return Delay;
}

void LedChunk_t::StartOver() {
    Head = Start; //Random(Start, End);
    Tail = GetPrevN(Head, NLeds);
    Effects.DesiredClr[Head] = Color;
    Effects.DesiredClr[Tail] = clBlack;
}

uint8_t LedChunk_t::GetNext(int *PCurrent) {
    int curr = *PCurrent;
    if(curr == End) {
        *PCurrent = Start;
        return OVERFLOW;
    }
    else {
        if(End > Start) *PCurrent = curr + 1;
        else *PCurrent = curr - 1;
        return OK;
    }
}

int LedChunk_t::GetPrevN(int Current, int N) {
    int Rslt;
    if(End > Start) {
        Rslt = Current - N;
        if(Rslt < Start) Rslt += 1 + End - Start;
    }
    else {
        Rslt = Current + N;
        if(Rslt > Start) Rslt -= Start - End + 1;
    }
    return Rslt;
}

#endif

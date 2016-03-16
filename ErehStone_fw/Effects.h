/*
 * Effects.h
 *
 *  Created on: 15 марта 2016 г.
 *      Author: Kreyl
 */

#pragma once

#include "board.h"
#include "ch.h"
#include "color.h"
#include "ws2812b.h"

enum EffState_t {effIdle, effAllSmoothly, effChunkRunningRandom};

class LedChunk_t {
private:
    int Head, Tail;
    uint8_t GetNext(int *PCurrent);
    int GetPrevN(int Current, int N);
public:
    int Start, End, NLeds;
    Color_t Color;
    LedChunk_t(int AStart, int AEnd) {
        Start = AStart;
        End = AEnd;
        NLeds = 1;
        Head = AStart;
        Tail = AEnd;
        Color = clBlack;
    }
    uint32_t ProcessAndGetDelay();
    void StartOver();
};

class Effects_t {
private:
    thread_t *PThd;
    EffState_t IState;
    uint32_t SmoothValue[LED_CNT];
    void IProcessChunkRandom();
public:
    void Init();
    // Effects
    void AllTogetherNow(Color_t Color);
    void AllTogetherSmoothly(Color_t Color, uint32_t ASmoothValue);
    void ChunkRunningRandom(Color_t Color, uint32_t NLeds, uint32_t ASmoothValue);
    // Inner use
    uint32_t ICalcDelayN(uint32_t n);
    Color_t DesiredClr[LED_CNT];
    void ITask();
};

extern Effects_t Effects;

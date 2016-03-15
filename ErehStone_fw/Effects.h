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

enum EffState_t {effIdle, effAllSmoothly};

class Effects_t {
private:
    thread_t *PThd;
    EffState_t IState;
    Color_t DesiredClr[LED_CNT];
    uint32_t SmoothValue[LED_CNT];
    uint32_t ICalcDelayN(uint32_t n);
public:
    void Init();
    // Effects
    void AllTogetherNow(Color_t Color);
    void AllTogetherSmoothly(Color_t Color, uint32_t ASmoothValue);
    // Inner use
    void ITask();
};

extern Effects_t Effects;

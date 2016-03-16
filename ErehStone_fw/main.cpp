/*
 * main.cpp
 *
 *  Created on: 20 ����. 2014 �.
 *      Author: g.kruglov
 */

#include "main.h"
#include "board.h"
#include "led.h"
#include "Sequences.h"
#include "radio_lvl1.h"
#include "Sequences.h"
#include "Effects.h"

#include "ws2812b.h"

App_t App;

int main(void) {
    // ==== Init Vcore & clock system ====
    SetupVCore(vcore1V5);
//    Clk.SetMSI4MHz();
    Clk.SetupFlashLatency(16);
    Clk.SwitchToHSI();
    Clk.UpdateFreqValues();

    // Init OS
    halInit();
    chSysInit();
    App.InitThread();

    // ==== Init hardware ====
    Uart.Init(115200, UART_GPIO, UART_TX_PIN);//, UART_GPIO, UART_RX_PIN);
    Uart.Printf("\r%S %S\r", APP_NAME , BUILD_TIME);
    Clk.PrintFreqs();

    Effects.Init();

    if(Radio.Init() == OK) {

    }
    else {
        chThdSleepMilliseconds(2700);
    }

    Effects.ChunkRunningRandom(clGreen, 3, 45);

    // Main cycle
    App.ITask();
}

__noreturn
void App_t::ITask() {
//    Effects.AllTogetherSmoothly(clGreen, 360);
    while(true) {
//        chThdSleepMilliseconds(2700);
//        Effects.AllTogetherNow(clGreen);
//        chThdSleepMilliseconds(270);
//        Effects.AllTogetherNow(clRed);
//        chThdSleepMilliseconds(270);
//        Effects.AllTogetherNow(clBlue);
//        chThdSleepMilliseconds(270);

//        Effects.AllTogetherSmoothly(clGreen, 180);
//        chEvtWaitAny(EVT_LEDS_DONE);
//        Effects.AllTogetherSmoothly(clRed, 180);
//        chEvtWaitAny(EVT_LEDS_DONE);
//        Effects.AllTogetherSmoothly(clBlue, 180);
//        chEvtWaitAny(EVT_LEDS_DONE);
//        Effects.AllTogetherSmoothly(clYellow, 180);
//        chEvtWaitAny(EVT_LEDS_DONE);
//        Effects.AllTogetherSmoothly(clCyan, 180);
//        chEvtWaitAny(EVT_LEDS_DONE);
//        Effects.AllTogetherSmoothly(clMagenta, 180);
//        chEvtWaitAny(EVT_LEDS_DONE);
//        Effects.AllTogetherSmoothly(clWhite, 180);
//        chEvtWaitAny(EVT_LEDS_DONE);
//        Effects.AllTogetherSmoothly(clBlack, 180);
//        chEvtWaitAny(EVT_LEDS_DONE);

//        __unused eventmask_t Evt = chEvtWaitAny(ALL_EVENTS);

//        if(Evt & EVT_LEDS_DONE) {
//
//        }

#if UART_RX_ENABLED
        if(EvtMsk & EVTMSK_UART_NEW_CMD) {
            OnCmd((Shell_t*)&Uart);
            Uart.SignalCmdProcessed();
        }
#endif

#if 0 // ==== Radio cmd ====
        if(Evt & EVT_RADIO_NEW_CMD) {

        }
#endif
    } // while true
}

#if UART_RX_ENABLED // ======================= Command processing ============================
void App_t::OnCmd(Shell_t *PShell) {
	Cmd_t *PCmd = &PShell->Cmd;
    __attribute__((unused)) int32_t dw32 = 0;  // May be unused in some configurations
    Uart.Printf("%S\r", PCmd->Name);
    // Handle command
    if(PCmd->NameIs("Ping")) {
        PShell->Ack(OK);
    }

    else PShell->Ack(CMD_UNKNOWN);
}
#endif

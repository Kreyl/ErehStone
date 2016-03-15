/*
 * main.cpp
 *
 *  Created on: 20 февр. 2014 г.
 *      Author: g.kruglov
 */

#include "main.h"
#include "board.h"
#include "led.h"
#include "Sequences.h"
#include "radio_lvl1.h"
#include "Sequences.h"
#include "ws2812b.h"

App_t App;

int main(void) {
    // ==== Init Vcore & clock system ====
    SetupVCore(vcore1V5);
    Clk.SetMSI4MHz();
    Clk.UpdateFreqValues();

    // Init OS
    halInit();
    chSysInit();
    App.InitThread();

    // ==== Init hardware ====
    Uart.Init(115200, UART_GPIO, UART_TX_PIN);//, UART_GPIO, UART_RX_PIN);
    Uart.Printf("\r%S %S\r", APP_NAME , BUILD_TIME);
    Clk.PrintFreqs();

    LedWs.Init();

    if(Radio.Init() == OK) {

    }
    else {
        chThdSleepMilliseconds(2700);
    }

    // Main cycle
    App.ITask();
}

__noreturn
void App_t::ITask() {
    while(true) {
        chThdSleepMilliseconds(360);
        LedWs.SetCommonColor((Color_t){0,255,0});
        chThdSleepMilliseconds(360);
        LedWs.SetCommonColor((Color_t){255,0,0});
        chThdSleepMilliseconds(360);
        LedWs.SetCommonColor((Color_t){0,0,255});

//        __unused eventmask_t Evt = chEvtWaitAny(ALL_EVENTS);
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

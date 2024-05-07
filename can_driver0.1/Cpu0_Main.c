#include "Ifx_Types.h"
#include "IfxCpu.h"
#include "IfxScuWdt.h"
#include "can_init.h"
#include "can_driver.h"
#include "leds.h"

IFX_ALIGN(4) IfxCpu_syncEvent g_cpuSyncEvent = 0; //alligs to multiple of 4byte

void core0_main(void)
{
    IfxCpu_enableInterrupts();

    IfxScuWdt_disableCpuWatchdog(IfxScuWdt_getCpuWatchdogPassword());
       IfxScuWdt_disableSafetyWatchdog(IfxScuWdt_getSafetyWatchdogPassword());
    IfxCpu_emitEvent(&g_cpuSyncEvent); // sets the CPU-specific event flag into the synchronization variable event.
    IfxCpu_waitEvent(&g_cpuSyncEvent, 1); //waits until all the involved CPUs set their event flag
    
    canInitDriver();
    initLeds();
    canTransmitMessage(CAN_DEBUG_ID,CAN_DEBUG_DATA, 1);

    while(1)
    {
    }
}

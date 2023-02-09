#include <RTE_Components.h>
#include CMSIS_device_header

#include "UART_stdio.h"

#include "lvgl.h"
#include "demos/lv_demos.h"
extern void lv_port_disp_init(void);

#define DISABLE_PRINTF
#ifdef DISABLE_PRINTF
#define printf(fmt, ...) (0)
__asm(".global __use_no_semihosting");
void _sys_exit(int return_code) { while(1); }
void _ttywrch(int ch) { }
#endif

volatile uint32_t ms_ticks = 0;
void SysTick_Handler (void) { ms_ticks++; lv_tick_inc(1); }
void delay(uint32_t nticks) { nticks += ms_ticks; while(ms_ticks < nticks); }
#define TICKS_PER_SECOND    1000

int main (void)
{
	SysTick_Config(SystemCoreClock/TICKS_PER_SECOND);
#ifdef RTE_Compiler_IO_STDOUT_User
	stdout_init();
#endif

	lv_port_disp_init();

#if LV_USE_DEMO_WIDGETS
	lv_demo_widgets();
#endif

#if LV_USE_DEMO_BENCHMARK
	lv_demo_benchmark(LV_DEMO_BENCHMARK_MODE_RENDER_AND_DRIVER);
#endif

	while (1) {
		lv_task_handler();
	}

	return 0;
}

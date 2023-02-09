/*
 * Auto generated Run-Time-Environment Component Configuration File
 *      *** Do not modify ! ***
 *
 * Project: LVGLv9 Demos Clang
 * RTE configuration: LVGLv9 Demos Clang.rteconfig
*/
#ifndef RTE_COMPONENTS_H
#define RTE_COMPONENTS_H

/*
 * Define the Device Header File:
*/
#define CMSIS_device_header "M55_HP.h"

/* ARM::Acceleration.Arm-2D.Core */
#define __RTE_ACCELERATION_ARM_2D__
/* ARM::Acceleration.Arm-2D.Transform */
#define __RTE_ACCELERATION_ARM_2D_TRANSFORM__
/* AlifSemiconductor::BSP.External peripherals.ILI9806E LCD panel */
#define RTE_Drivers_MIPI_DSI_ILI9806E_PANEL 1   /* Driver ILI9806E LCD panel*/
/* AlifSemiconductor::Device.SOC Peripherals.CDC200 */
#define RTE_Drivers_CDC200	1   /* Driver CDC200*/
/* AlifSemiconductor::Device.SOC Peripherals.GPIO */
#define RTE_Drivers_GPIO   1           /* Driver GPIO */
/* AlifSemiconductor::Device.SOC Peripherals.I3C */
#define RTE_Drivers_I3C0			1           /* Driver I3C */
/* AlifSemiconductor::Device.SOC Peripherals.MIPI DSI */
#define RTE_Drivers_MIPI_DSI			1   /* Driver MIPI DSI */
/* AlifSemiconductor::Device.SOC Peripherals.PINCONF */
#define RTE_Drivers_PINCONF   1           /* Driver PinPAD and PinMux */
/* AlifSemiconductor::Device.SOC Peripherals.USART */
#define RTE_Drivers_USART0   1           /* Driver UART0 */
        #define RTE_Drivers_USART1   1           /* Driver UART1 */
	#define RTE_Drivers_USART2   1           /* Driver UART1 */
	#define RTE_Drivers_USART3   1           /* Driver UART1 */
	#define RTE_Drivers_USART4   1           /* Driver UART1 */
	#define RTE_Drivers_USART5   1           /* Driver UART1 */
	#define RTE_Drivers_USART6   1           /* Driver UART1 */
	#define RTE_Drivers_USART7   1           /* Driver UART1 */
/* Keil.ARM Compiler::Compiler.I/O.STDOUT.User */
#define RTE_Compiler_IO_STDOUT          /* Compiler I/O: STDOUT */
          #define RTE_Compiler_IO_STDOUT_User     /* Compiler I/O: STDOUT User */
/* LVGL.LVGL::LVGL.lvgl.Benchmark */
/*! \brief enable demo:bencharmk */
#define LV_USE_DEMO_BENCHMARK         1
/* LVGL.LVGL::LVGL.lvgl.Essential */
/*! \brief Enable LVGL */
#define RTE_GRAPHICS_LVGL
/* LVGL.LVGL::LVGL.lvgl.Extra Themes */
/*! \brief use extra themes, widgets and layouts */
#define RTE_GRAPHICS_LVGL_USE_EXTRA_THEMES
/* LVGL.LVGL::LVGL.lvgl.GPU Arm-2D */
/*! \brief enable Arm-2D support*/
#define LV_USE_GPU_ARM2D 1

#endif /* RTE_COMPONENTS_H */

/* Copyright (c) 2022 ALIF SEMICONDUCTOR

   All rights reserved.
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
   - Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   - Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
   - Neither the name of ALIF SEMICONDUCTOR nor the names of its contributors
     may be used to endorse or promote products derived from this software
     without specific prior written permission.
   *
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
   ---------------------------------------------------------------------------*/

/**************************************************************************//**
 * @file     lv_port_disp.c
 * @author   Ahmad Rashed
 * @email    ahmad.rashed@alifsemi.com
 * @version  V1.0.0
 * @date     28-March-2022
 * @brief    for lvgl library display init with touch input
 * @bug      None.
 * @Note     None
 ******************************************************************************/
#include <stdio.h>

#include <RTE_Device.h>
#include <RTE_Components.h>
#include CMSIS_device_header

#include "lvgl.h"

#include "Driver_CDC200.h"
#include "CDC200_dev.h"

//#define DISABLE_I2C_TOUCH

#if ((LV_COLOR_DEPTH == 16) && (RTE_CDC200_PIXEL_FORMAT != 2)) || \
    ((LV_COLOR_DEPTH == 32) && (RTE_CDC200_PIXEL_FORMAT != 0))
#error "The LV_COLOR_DEPTH and RTE_CDC200_PIXEL_FORMAT must match."
#endif

#if RTE_CDC200_PIXEL_FORMAT == 0
#define PIXEL_BYTES    (4)
#elif RTE_CDC200_PIXEL_FORMAT == 2
#define PIXEL_BYTES    (2)
#elif RTE_CDC200_PIXEL_FORMAT == 1
#error "Using 24-bit image buffer is not supported by LVGL, use 16-bit or 32-bit only."
#endif

#define DIMAGE_X       (RTE_PANEL_HACTIVE_TIME)
#define DIMAGE_Y       (RTE_PANEL_VACTIVE_LINE)
static uint8_t lcd_image[DIMAGE_Y][DIMAGE_X][PIXEL_BYTES] __attribute__((section("lcd_frame_buf")));
static uint8_t lcd_image2[DIMAGE_Y][DIMAGE_X][PIXEL_BYTES] __attribute__((section("lcd_frame_buf")));

extern ARM_DRIVER_CDC200 Driver_CDC200;
static ARM_DRIVER_CDC200 *CDCdrv = &Driver_CDC200;

void hw_disp_cb(uint32_t event)
{
	if(event & ARM_CDC_DSI_ERROR_EVENT)
	{
		/* Transfer Error: Received Hardware error, Wake-up Thread. */
		while(1);
	}
}

static void hw_disp_init(void)
{
	CDCdrv->Initialize(hw_disp_cb);
	CDCdrv->PowerControl(ARM_POWER_FULL);
	CDCdrv->Control(CDC200_CONFIGURE_DISPLAY, (uint32_t)lcd_image);
	CDCdrv->Start();
}
#ifndef DISABLE_I2C_TOUCH
#include "GT911_touch_driver.h"
static void lv_touch_get(lv_indev_drv_t * drv, lv_indev_data_t * data)
{
	static xypair_t xypair = {0, 0};
	if (TOUCH_GetPointer(&xypair) == 0) {
		data->state = LV_INDEV_STATE_PRESSED;
	} else {
		data->state = LV_INDEV_STATE_RELEASED;
	}
	data->point.x = xypair.x;
	data->point.y = xypair.y;
}
static void hw_touch_init(void)
{
	TOUCH_Init(i2c_addr_5D);
}
#endif

static void lv_disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
	CDCdrv->Control(CDC200_FRAMEBUF_UPDATE, (uint32_t) color_p);
	CDCdrv->Control(CDC200_SHADOW_RELOAD_REGISTER, ENABLE);

	lv_disp_flush_ready(disp_drv);
}

void lv_port_disp_init(void)
{
	/* display structures */
	static lv_disp_drv_t disp_drv;
	static lv_disp_draw_buf_t disp_buf;
	lv_init();

	lv_disp_drv_init(&disp_drv);
	lv_disp_draw_buf_init(&disp_buf, lcd_image, lcd_image2, DIMAGE_Y*DIMAGE_X);

	disp_drv.draw_buf = &disp_buf;
	disp_drv.hor_res = DIMAGE_X;
	disp_drv.ver_res = DIMAGE_Y;
	disp_drv.direct_mode = 1;
	disp_drv.full_refresh = 1;
	disp_drv.flush_cb = lv_disp_flush;
	lv_disp_drv_register(&disp_drv);

	hw_disp_init();

#ifndef DISABLE_I2C_TOUCH
	/* input device structure */
	static lv_indev_drv_t touch_drv;

	lv_indev_drv_init(&touch_drv);
	touch_drv.type = LV_INDEV_TYPE_POINTER;
	touch_drv.read_cb = lv_touch_get;
	lv_indev_drv_register(&touch_drv);

	hw_touch_init();
#endif
}

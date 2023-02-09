#include <string.h>
#include <stdbool.h>

#include "Driver_GPIO.h"
#include "Driver_I3C.h"
#include "Driver_PINMUX_AND_PINPAD.h"

#include "GT911_touch_driver.h"
#include "system_utils.h"

#define TOUCH_TX_COUNT     2
#define TOUCH_I2C_TIMEOUT  80

#if 1
#define DEBUG_PRINTF(...) (0)
#else
#include <stdio.h>
//#define DEBUG_PRINTF printf
#endif

//#define USE_I2C_GPIO_BITBANG

volatile uint8_t touch_event_gpio;
volatile uint8_t touch_event_i2c;

uint8_t i2c_addr = i2c_addr_5D;

#ifdef USE_I2C_GPIO_BITBANG
#include "drv_i2c_bitbang.h"
#else
extern volatile uint32_t ms_ticks;
extern ARM_DRIVER_I3C Driver_I3C0;
static ARM_DRIVER_I3C *I3Cdrv = &Driver_I3C0;
#endif

extern ARM_DRIVER_GPIO ARM_Driver_GPIO_(2);
extern ARM_DRIVER_GPIO ARM_Driver_GPIO_(4);
static ARM_DRIVER_GPIO *gpio2 = &ARM_Driver_GPIO_(2);
static ARM_DRIVER_GPIO *gpio4 = &ARM_Driver_GPIO_(4);

static int32_t TOUCH_Write(uint8_t dev_addr, uint16_t reg_addr, uint8_t *reg_data, uint8_t num_bytes)
{
	uint32_t ms_wait = 0;
	uint8_t  tx_data[12] = {0};
	uint8_t  i;
	int32_t  ret;

	tx_data[0] = (reg_addr >> 8) & 0xFF;
	tx_data[1] = (reg_addr     ) & 0xFF;

	if (num_bytes) {
		for(i = 0; i < num_bytes; i++) {
			tx_data[i + TOUCH_TX_COUNT] = reg_data[i];
		}
	}

#ifdef USE_I2C_GPIO_BITBANG
	ret = i2c_master_WriteData(dev_addr, tx_data, num_bytes + TOUCH_TX_COUNT);
	if (ret == 0) {
		/* TX Success: Got ACK from slave */
		DEBUG_PRINTF(">> I2C Master Transmit Success: Got ACK from slave addr: 0x%X\n", dev_addr);
		if (num_bytes) {
			DEBUG_PRINTF("\twrote %d bytes to reg addr: 0x%X\n", num_bytes, reg_addr);
			DEBUG_PRINTF("\tTransmited Data to slave: 0x");
			for (i = 0; i < (num_bytes + TOUCH_TX_COUNT); i++) {
				DEBUG_PRINTF("%02X", tx_data[i]);
			}
			DEBUG_PRINTF("\n");
		} else {
			DEBUG_PRINTF("\treading from reg addr: 0x%X\n", reg_addr);
		}
	} else {
		/* TX Error: Got NACK from slave */
		DEBUG_PRINTF(">> I2C Master Transmit Error: Got NACK from slave addr: 0x%X\n", dev_addr);
	}
#else
	touch_event_i2c = 0;
	do {
		ret = I3Cdrv->MasterTransmit(dev_addr, tx_data, num_bytes + TOUCH_TX_COUNT);
	} while(ret == ARM_DRIVER_ERROR_BUSY);

	if(ret != ARM_DRIVER_OK) {
		DEBUG_PRINTF("MasterTransmit: %d\r\n", ret);
		return ret;
	}

	ms_wait = ms_ticks + TOUCH_I2C_TIMEOUT;
	while (1) {
		if (touch_event_i2c) break;
		if (ms_wait == ms_ticks) {
			DEBUG_PRINTF("Error: I2C Master Transmit timeout after %dms\n", TOUCH_I2C_TIMEOUT);
			return -1;
		}
	}

	if (touch_event_i2c & ARM_I3C_EVENT_TRANSFER_DONE) {
		/* TX Success: Got ACK from slave */
		DEBUG_PRINTF(">> I2C Master Transmit Success: Got ACK from slave addr: 0x%X\n", dev_addr);
		if (num_bytes) {
			DEBUG_PRINTF("\twrote %d bytes to reg addr: 0x%X\n", num_bytes, reg_addr);
			DEBUG_PRINTF("\tTransmited Data to slave: 0x");
			for (i = 0; i < (num_bytes + TOUCH_TX_COUNT); i++) {
				DEBUG_PRINTF("%02X", tx_data[i]);
			}
			DEBUG_PRINTF("\n");
		} else {
			DEBUG_PRINTF("\treading from reg addr: 0x%X\n", reg_addr);
		}
	}

	if (touch_event_i2c & ARM_I3C_EVENT_TRANSFER_ERROR) {
		/* TX Error: Got NACK from slave */
		DEBUG_PRINTF(">> I2C Master Transmit Error: Got NACK from slave addr: 0x%X\n", dev_addr);
		return -1;
	}
#endif

	return 0;
}

static int32_t TOUCH_Read(uint8_t dev_addr, uint16_t reg_addr, uint8_t *reg_data, uint8_t num_bytes)
{
	uint32_t ms_wait;
	uint8_t  rx_data[12] = {0};
	uint8_t  i;
	int32_t  ret;

	if (TOUCH_Write(dev_addr, reg_addr, 0, 0)) return -1;

#ifdef USE_I2C_GPIO_BITBANG
	ret = i2c_master_ReadData(dev_addr, rx_data, num_bytes);
	if (ret == 0) {
		/* RX Success: Got ACK from slave */
		DEBUG_PRINTF(">> I2C Master Receive Success: Got ACK from slave addr: 0x%X\n", dev_addr);
		DEBUG_PRINTF("\tread %d bytes from reg addr: 0x%X\n", num_bytes, reg_addr);
		DEBUG_PRINTF("\tReceived Data from slave: 0x");
		for (i = 0; i < num_bytes; i++) {
			DEBUG_PRINTF("%02X", rx_data[i]);
		}
		DEBUG_PRINTF("\n");
	} else {
		/* RX Error: Got NACK from slave */
		DEBUG_PRINTF(">> I2C Master Receive Error: Got NACK from slave addr: 0x%X\n", dev_addr);
	}
#else
	touch_event_i2c = 0;
	ret = I3Cdrv->MasterReceive(dev_addr, rx_data, num_bytes);

	if(ret != ARM_DRIVER_OK) {
		DEBUG_PRINTF("MasterReceive: %d\r\n", ret);
		return -12;
	}

	ms_wait = ms_ticks + TOUCH_I2C_TIMEOUT;
	while (1) {
		if (touch_event_i2c) break;
		if (ms_wait == ms_ticks) {
			DEBUG_PRINTF("Error: I2C Master Receive timeout after %dms\n", TOUCH_I2C_TIMEOUT);
			return -1;
		}
	}

	if (touch_event_i2c & ARM_I3C_EVENT_TRANSFER_DONE) {
		/* RX Success: Got ACK from slave */
		DEBUG_PRINTF(">> I2C Master Receive Success: Got ACK from slave addr: 0x%X\n", dev_addr);
		DEBUG_PRINTF("\tread %d bytes from reg addr: 0x%X\n", num_bytes, reg_addr);
		DEBUG_PRINTF("\tReceived Data from slave: 0x");
		for (i = 0; i < num_bytes; i++) {
			DEBUG_PRINTF("%02X", rx_data[i]);
		}
		DEBUG_PRINTF("\n");
	}

	if (touch_event_i2c & ARM_I3C_EVENT_TRANSFER_ERROR) {
		DEBUG_PRINTF(">> I2C Master Receive Error: Got NACK from slave addr: 0x%X\n", dev_addr);
		return -1;
	}
#endif

	memcpy(reg_data, rx_data, num_bytes);

	return 0;
}

static void TOUCH_IntEnable(bool enable)
{
	uint32_t arg = ARM_GPIO_IRQ_POLARITY_LOW | ARM_GPIO_IRQ_EDGE_SENSITIVE_SINGLE | ARM_GPIO_IRQ_SENSITIVE_EDGE;

	enable ?
			gpio2->Control(PIN_NUMBER_20, ARM_GPIO_ENABLE_INTERRUPT, &arg):
			gpio2->Control(PIN_NUMBER_20, ARM_GPIO_DISABLE_INTERRUPT, NULL);
}

#ifndef USE_I2C_GPIO_BITBANG
static void TOUCH_I2C_CB(uint32_t event)
{
	touch_event_i2c |= event;
}
#endif

static void TOUCH_GPIO_CB(uint32_t event)
{
	TOUCH_IntEnable(false);
	touch_event_gpio = 1;
}

void TOUCH_Reset()
{
	/* TOUCH INT pin */
	gpio2->Control(PIN_NUMBER_20, ARM_GPIO_DISABLE_INTERRUPT, NULL);
	gpio2->SetValue(PIN_NUMBER_20, GPIO_PIN_OUTPUT_STATE_LOW);
	gpio2->SetDirection(PIN_NUMBER_20, GPIO_PIN_DIRECTION_OUTPUT);

	/* TOUCH RST pin */
	gpio4->Control(PIN_NUMBER_2, ARM_GPIO_DISABLE_INTERRUPT, NULL);
	gpio4->SetValue(PIN_NUMBER_2, GPIO_PIN_OUTPUT_STATE_LOW);
	gpio4->SetDirection(PIN_NUMBER_2, GPIO_PIN_DIRECTION_OUTPUT);

	PMU_delay_loop_us(1000);

	gpio4->SetValue(PIN_NUMBER_2, GPIO_PIN_OUTPUT_STATE_HIGH);

	PMU_delay_loop_us(10000);

	gpio2->SetValue(PIN_NUMBER_20, GPIO_PIN_OUTPUT_STATE_LOW);

	PMU_delay_loop_us(50000);

	gpio2->SetDirection(PIN_NUMBER_20, GPIO_PIN_DIRECTION_INPUT);
	gpio2->Initialize(PIN_NUMBER_20, &TOUCH_GPIO_CB);
}

int32_t TOUCH_Init(uint8_t dev_addr)
{

	uint32_t config_gpio =
			PAD_FUNCTION_READ_ENABLE |
			PAD_FUNCTION_DRIVER_DISABLE_STATE_WITH_PULL_UP |
			PAD_FUNCTION_SCHMITT_TRIGGER_ENABLE;

	/* gpio2 config */
	PINMUX_Config(PORT_NUMBER_2, PIN_NUMBER_20, PINMUX_ALTERNATE_FUNCTION_0);
	PINPAD_Config(PORT_NUMBER_2, PIN_NUMBER_20, config_gpio);
	//PINPAD_Config(PORT_NUMBER_4, PIN_NUMBER_2,  config_gpio);

#ifdef USE_I2C_GPIO_BITBANG
	i2c_init();
#else
	uint32_t config_i3c =
			PAD_FUNCTION_READ_ENABLE |
			PAD_FUNCTION_DRIVER_DISABLE_STATE_WITH_PULL_UP |
			PAD_FUNCTION_DRIVER_OPEN_DRAIN;

	/* i3c */
	PINMUX_Config(PORT_NUMBER_3, PIN_NUMBER_8, PINMUX_ALTERNATE_FUNCTION_3);	// P3_8: SDA (mux mode 3)
	PINPAD_Config(PORT_NUMBER_3, PIN_NUMBER_8, config_i3c);
	PINMUX_Config(PORT_NUMBER_3, PIN_NUMBER_9, PINMUX_ALTERNATE_FUNCTION_4);	// P3_9: SCL (mux mode 4)
	PINPAD_Config(PORT_NUMBER_3, PIN_NUMBER_9, config_i3c);

	I3Cdrv->Initialize(TOUCH_I2C_CB);
	I3Cdrv->PowerControl(ARM_POWER_FULL);
	I3Cdrv->Control(
			I3C_MASTER_SET_BUS_MODE,
			I3C_BUS_MODE_MIXED_FAST_I2C_FM_SPEED_400_KBPS);

	I3Cdrv->AttachI2Cdev(dev_addr);
#endif

	TOUCH_Reset();
	TOUCH_IntEnable(true);

	uint8_t data = 0;
	TOUCH_Write(dev_addr, GT911_COMMAND_REG, &data, 1);

	uint8_t product_id[12] = {0};
	TOUCH_Read(dev_addr, GT911_PRODUCT_ID, product_id, 11);

	if (product_id[0] != '9') // ascii character
	{
		DEBUG_PRINTF("[ERROR] - touch controller product id doesn't match\r\n");
		return -1;
	}

	return 0;
}

int32_t TOUCH_GetPointer(xypair_t * xypair)
{
	if (touch_event_gpio) {
		uint8_t data;
		touch_event_gpio = 0;

		TOUCH_Read(i2c_addr, GT911_TOUCH_STATUS, &data, 1);

		bool msb = data & 0x80;
		uint8_t numTouches = data & 0x07; // number of concurrent touch spots

		if (msb) {
			if (numTouches) {
				uint8_t rx_buf[4] = {0};
				TOUCH_Read(i2c_addr, GT911_FIRST_TOUCHPOINT_DATA, rx_buf, 4);
				xypair->x = (int16_t)(((rx_buf[1] << 8) | rx_buf[0]) + 0);
				xypair->y = (int16_t)(((rx_buf[3] << 8) | rx_buf[2]) + 0);
			}

			data = 0;
			TOUCH_Write(i2c_addr, GT911_TOUCH_STATUS, &data, 1);
		}

		TOUCH_IntEnable(true);

		return 0;
	}

	return -1;
}

int32_t TOUCH_Deinit(uint8_t dev_addr)
{
#ifndef USE_I2C_GPIO_BITBANG
	I3Cdrv->Detachdev(dev_addr);

	I3Cdrv->PowerControl(ARM_POWER_OFF);
	I3Cdrv->Uninitialize();
#endif
	return 0;
}

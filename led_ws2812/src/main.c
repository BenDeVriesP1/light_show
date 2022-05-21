/*
 * Copyright (c) 2017 Linaro Limited
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <string.h>

#define LOG_LEVEL 4
#include <logging/log.h>
LOG_MODULE_REGISTER(main);

#include <zephyr.h>
#include <drivers/led_strip.h>
#include <device.h>
#include <drivers/spi.h>
#include <sys/util.h>

#define STRIP_NODE		DT_NODELABEL(led_strip)
#define STRIP_NUM_PIXELS	DT_PROP(DT_NODELABEL(led_strip), chain_length)

//#define DELAY_TIME K_MSEC(50)

#define DELAY_TIME K_MSEC(10)

#define RGB(_r, _g, _b) { .r = (_r), .g = (_g), .b = (_b) }

// static const struct led_rgb colors[] = {
// 	RGB(0x00, 0x00, 0x7f), /* blue */
// 	RGB(0x7f, 0x00, 0x00), /* red */
// 	RGB(0x00, 0x7f, 0x00), /* green */
// };

static const struct led_rgb colors[] = {
	
	RGB(0xff, 0x00, 0x00), /* red */
	RGB(0x00, 0xff, 0x00), /* green */
	RGB(0x00, 0x00, 0xff), /* blue */
	RGB(0xff, 0xff, 0x00), /* red */
	RGB(0x00, 0xff, 0xff), /* green */
	RGB(0xff, 0x00, 0xff), /* blue */

};

#define LED_MAX_VALUE 0xff

void color_wheel_to_rgb(float theta,float r,float scale,struct led_rgb *out)
{
	float scales[3];
	r = (r>1) ? 1.0 : r;
	r = (r<0) ? 0 : r;
	scale = (scale>1) ? 1.0 : scale;
	scale = (scale<0) ? 0 : scale;
	for(uint8_t i=0;i<3;i++)
	{
		while(theta > 360)
		{
			theta -= 360;
		}
		if(theta < 120)
		{
			scales[i] = 1.0 * scale;
		}
		else if (theta < 180)
		{
			scales[i] = ((180 - theta)/(60.0)) * scale;
		}
		else if (theta < 300)
		{
			scales[i] = (1 - r) * scale;
		}
		else
		{
			scales[i] = (1 - ((360-theta)/(60.0))) * scale;
		}
		theta += 120;
	}
	out->r = (uint8_t)(LED_MAX_VALUE * scales[1]);
	out->g = (uint8_t)(LED_MAX_VALUE * scales[0]);
	out->b = (uint8_t)(LED_MAX_VALUE * scales[2]);
	//return (struct led_rgb RGB((uint8_t)(LED_MAX_VALUE * scales[1]),(uint8_t)(LED_MAX_VALUE * scales[0]),(uint8_t)(LED_MAX_VALUE * scales[2])));
}

// static const struct led_rgb colors[] = {
// 	RGB(0x00, 0x00, 0x0f), /* blue */
// 	RGB(0x0f, 0x00, 0x00), /* red */
// 	RGB(0x00, 0x0f, 0x00), /* green */
// };

static const struct led_rgb clear = RGB(0x00, 0x00, 0x00);
struct led_rgb pixels[STRIP_NUM_PIXELS];

static const struct device *strip = DEVICE_DT_GET(STRIP_NODE);


void main(void)
{
	size_t cursor = 0, color = 0;
	int rc;
	//strip = device_get_binding("bitbang");
	if (device_is_ready(strip)) {
		LOG_INF("Found LED strip device %s", strip->name);
	} else {
		LOG_ERR("LED strip device %s is not ready", strip->name);
		return;
	}
	float theta_base = 0;
	float theta_n = 0;

	uint16_t i = 0;
	LOG_INF("Displaying pattern on strip");
	while (1) {
		color = 0;
		cursor = 0;
		for(i = 0; i<(ARRAY_SIZE(colors) * STRIP_NUM_PIXELS * 3);i++)
			{
				theta_base += 3;
				if(theta_base >= 360)
				{
					theta_base = 0;
				}
				float theta_n = 0;

				for(uint16_t j = 0;j<STRIP_NUM_PIXELS;j++)
				{
					
					color_wheel_to_rgb(theta_base+theta_n,1,1,&pixels[j]);
					
					if(j>22)
					{
						theta_n -= 30;
					}
					else
					{
						theta_n += 30;
					}
				}
				rc = led_strip_update_rgb(strip, pixels, STRIP_NUM_PIXELS);

			if (rc) {
				LOG_ERR("couldn't update strip: %d", rc);
			}

			// cursor++;
			// if (cursor >= STRIP_NUM_PIXELS) {
			// 	cursor = 0;
			// 	color++;
			// 	if (color == ARRAY_SIZE(colors)) {
			// 		color = 0;
			// 	}
			// }
			k_sleep(DELAY_TIME);
		}
		// color = 0;
		// for(i = 0; i<(ARRAY_SIZE(colors) * 2);i++)
		// {
		// 	cursor = 0;
		// 	memset(&pixels, 0x00, sizeof(pixels));
		// 	while (cursor < STRIP_NUM_PIXELS)
		// 	{
		// 		memcpy(&pixels[cursor++], &colors[color], sizeof(struct led_rgb));
		// 		//cursor++;
		// 		rc = led_strip_update_rgb(strip, pixels, STRIP_NUM_PIXELS);
		// 		k_sleep(DELAY_TIME);
		// 	}
		// 	cursor = 0;
		// 	while (cursor < STRIP_NUM_PIXELS)
		// 	{
		// 		memcpy(&pixels[cursor++], &clear, sizeof(struct led_rgb));
		// 		//cursor++;
		// 		rc = led_strip_update_rgb(strip, pixels, STRIP_NUM_PIXELS);
		// 		k_sleep(DELAY_TIME);
		// 	}
		// 	color++;
		// 	if (color == ARRAY_SIZE(colors)) {
		// 		color = 0;
		// 	}
		// }
		// color = 0;
		// for(i = 0; i<(ARRAY_SIZE(colors));i++)
		// {
		// 	memset(&pixels, 0x00, sizeof(pixels));
		// 	for(uint16_t j = 0;j<STRIP_NUM_PIXELS;j++)
		// 	{
		// 		memcpy(&pixels[j], &colors[color], sizeof(struct led_rgb));
		// 	}
		// 	rc = led_strip_update_rgb(strip, pixels, STRIP_NUM_PIXELS);
		// 	color++;
		// 	k_sleep(K_MSEC(25*STRIP_NUM_PIXELS));
		// }
	}
}

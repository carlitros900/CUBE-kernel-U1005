/*
 * Copyright (C) 2015 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef BUILD_LK
#include <linux/string.h>
#include <linux/string.h>
#include <linux/regulator/consumer.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/syscore_ops.h>
#include <linux/timer.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/regulator/consumer.h>
#include <linux/gpio.h>
#include <linux/pinctrl/consumer.h>
#include <linux/i2c.h>
#include <mach/upmu_sw.h>
#else
#include <string.h>
#endif

#include "lcm_drv.h"
#include "amsa05bv_dsi_vdo_mt8173.h"
#include "ddp_hal.h"
#ifdef BUILD_LK
#include <platform/mt_gpio.h>
#include <platform/mt_pmic.h>
#elif defined(BUILD_UBOOT)
#else
#endif

/* --------------------------------------------------------------------------- */
/* Local Constants */
/* --------------------------------------------------------------------------- */

#define FRAME_WIDTH  (2560)
#define FRAME_HEIGHT (1600)

/* --------------------------------------------------------------------------- */
/* Local Variables */
/* --------------------------------------------------------------------------- */

static LCM_UTIL_FUNCS lcm_util = {
	.set_reset_pin = NULL,
	.udelay = NULL,
	.mdelay = NULL,
};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#if 0
#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))
#endif

//unsigned int GPIO_LCD_RST_EN_X115;
//unsigned int GPIO_AVDD_EN;

unsigned int GPIO_LCD_PWR_EN_X115;
unsigned int GPIO_LCD_PWR2_EN_X115;
unsigned int GPIO_LCD_RST_EN_X115;

#define REGFLAG_DELAY                                       0xFC
#define REGFLAG_END_OF_TABLE                                0xFD

//extern void DSI_clk_HS_mode(DISP_MODULE_ENUM module, void* cmdq, bool enter);
/* --------------------------------------------------------------------------- */
/* Local Functions */
/* --------------------------------------------------------------------------- */

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update) lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)    lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)                                   lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)               lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)                         lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size) lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

#define GPIO_OUT_ONE 1
#define GPIO_OUT_ZERO 0

#define   LCM_DSI_CMD_MODE                                 1

/* #define  PUSH_TABLET_USING */
/* #define REGFLAG_DELAY                                       0xFFFC */
/* #define REGFLAG_END_OF_TABLE                                0xFFFD */
extern void DSI_clk_HS_mode(DISP_MODULE_ENUM module, void *cmdq, bool enter);

struct LCM_setting_table {
	   unsigned char cmd;
	   unsigned char count;
	   unsigned char para_list[64];
};

struct LCM_setting_table lcm_initialization_setting_300nit[]= {
	//300nit
	{0x82, 1, {0x00}},
	{0x85, 2, {0x06,0x00}},
	
	{0xB0, 1, {0x34}},
	{0xBB, 1, {0x1b}},

	{0xB0, 1, {0x2E}},
	{0xBB, 1, {0x01}},
};

struct LCM_setting_table lcm_initialization_setting_282nit[]= {
	//282nit
	{0x82, 1, {0x01}},
	{0x85, 2, {0x06,0x00}},
	
	{0xB0, 1, {0x34}},
	{0xBB, 1, {0x1a}},

	{0xB0, 1, {0x2E}},
	{0xBB, 1, {0x01}},
};

struct LCM_setting_table lcm_initialization_setting_249nit[]= {
	//249nit
	{0x82, 1, {0x02}},
	{0x85, 2, {0x06,0x00}},

	{0xB0, 1, {0x34}},
	{0xBB, 1, {0x1c}},

	{0xB0, 1, {0x2E}},
	{0xBB, 1, {0x01}},
};

struct LCM_setting_table lcm_initialization_setting_220nit[]= {
	//220nit
	{0x82, 1, {0x03}},
	{0x85, 2, {0x23,0x00}},

	{0xB0, 1, {0x34}},
	{0xBB, 1, {0x1d}},

	{0xB0, 1, {0x2E}},
	{0xBB, 1, {0x01}},
};

struct LCM_setting_table lcm_initialization_setting_195nit[]= {
	//195nit
	{0x82, 1, {0x04}},
	{0x85, 2, {0x58,0x00}},

	{0xB0, 1, {0x34}},
	{0xBB, 1, {0x1d}},

	{0xB0, 1, {0x2E}},
	{0xBB, 1, {0x01}},
};

struct LCM_setting_table lcm_initialization_setting_172nit[]= {
	//172nit
	{0x82, 1, {0x05}},
	{0x85, 2, {0x87,0x00}},

	{0xB0, 1, {0x34}},
	{0xBB, 1, {0x1d}},

	{0xB0, 1, {0x2E}},
	{0xBB, 1, {0x01}},
};

struct LCM_setting_table lcm_initialization_setting_152nit[]= {
	//152nit
	{0x82, 1, {0x06}},
	{0x85, 2, {0xad,0x00}},

	{0xB0, 1, {0x34}},
	{0xBB, 1, {0x1d}},

	{0xB0, 1, {0x2E}},
	{0xBB, 1, {0x01}},
};

struct LCM_setting_table lcm_initialization_setting_134nit[]= {
	//134nit
	{0x82, 1, {0x07}},
	{0x85, 2, {0xad,0x00}},

	{0xB0, 1, {0x34}},
	{0xBB, 1, {0x1f}},

	{0xB0, 1, {0x2E}},
	{0xBB, 1, {0x01}},
};

struct LCM_setting_table lcm_initialization_setting_119nit[]= {
	//119nit
	{0x82, 1, {0x08}},
	{0x85, 2, {0xad,0x00}},

	{0xB0, 1, {0x34}},
	{0xBB, 1, {0x1f}},

	{0xB0, 1, {0x2E}},
	{0xBB, 1, {0x01}},
};

struct LCM_setting_table lcm_initialization_setting_105nit[]= {
	//105nit
	{0x82, 1, {0x09}},
	{0x85, 2, {0xad,0x00}},

	{0xB0, 1, {0x34}},
	{0xBB, 1, {0x1f}},

	{0xB0, 1, {0x2E}},
	{0xBB, 1, {0x01}},
};

struct LCM_setting_table lcm_initialization_setting_93nit[]= {
	//93nit
	{0x82, 1, {0x0a}},
	{0x85, 2, {0xad,0x00}},

	{0xB0, 1, {0x34}},
	{0xBB, 1, {0x1f}},

	{0xB0, 1, {0x2E}},
	{0xBB, 1, {0x01}},
};

struct LCM_setting_table lcm_initialization_setting_82nit[]= {
	//82nit
	{0x82, 1, {0x0b}},
	{0x85, 2, {0xad,0x00}},

	{0xB0, 1, {0x34}},
	{0xBB, 1, {0x1f}},

	{0xB0, 1, {0x2E}},
	{0xBB, 1, {0x01}},
};

struct LCM_setting_table lcm_initialization_setting_72nit[]= {
	//72nit
	{0x82, 1, {0x0c}},
	{0x85, 2, {0xad,0x00}},

	{0xB0, 1, {0x34}},
	{0xBB, 1, {0x1f}},

	{0xB0, 1, {0x2E}},
	{0xBB, 1, {0x01}},
};

struct LCM_setting_table lcm_initialization_setting_64nit[]= {
	//64nit
	{0x82, 1, {0x0d}},
	{0x85, 2, {0xb3,0x00}},

	{0xB0, 1, {0x34}},
	{0xBB, 1, {0x1f}},

	{0xB0, 1, {0x2E}},
	{0xBB, 1, {0x01}},
};

struct LCM_setting_table lcm_initialization_setting_56nit[]= {
	//56nit
	{0x82, 1, {0x0e}},
	{0x85, 2, {0xd2,0x00}},

	{0xB0, 1, {0x34}},
	{0xBB, 1, {0x1f}},

	{0xB0, 1, {0x2E}},
	{0xBB, 1, {0x01}},
};

struct LCM_setting_table lcm_initialization_setting_50nit[]= {
	//50nit
	{0x82, 1, {0x0f}},
	{0x85, 2, {0xe8,0x00}},

	{0xB0, 1, {0x34}},
	{0xBB, 1, {0x1f}},

	{0xB0, 1, {0x2E}},
	{0xBB, 1, {0x01}},
};

struct LCM_setting_table lcm_initialization_setting_44nit[]= {
	//44nit
	{0x82, 1, {0x10}},
	{0x85, 2, {0xfe,0x00}},

	{0xB0, 1, {0x34}},
	{0xBB, 1, {0x1f}},

	{0xB0, 1, {0x2E}},
	{0xBB, 1, {0x01}},
};

struct LCM_setting_table lcm_initialization_setting_39nit[]= {
	//39nit
	{0x82, 1, {0x11}},
	{0x85, 2, {0x11,0x01}},

	{0xB0, 1, {0x34}},
	{0xBB, 1, {0x1f}},

	{0xB0, 1, {0x2E}},
	{0xBB, 1, {0x01}},
};

struct LCM_setting_table lcm_initialization_setting_34nit[]= {
	//34nit
	{0x82, 1, {0x12}},
	{0x85, 2, {0x23,0x01}},

	{0xB0, 1, {0x34}},
	{0xBB, 1, {0x1f}},

	{0xB0, 1, {0x2E}},
	{0xBB, 1, {0x01}},
};

struct LCM_setting_table lcm_initialization_setting_30nit[]= {
	//30nit
	{0x82, 1, {0x13}},
	{0x85, 2, {0x32,0x01}},
	
	{0xB0, 1, {0x34}},
	{0xBB, 1, {0x1f}},

	{0xB0, 1, {0x2E}},
	{0xBB, 1, {0x01}},
};


static void push_table(struct LCM_setting_table *table, unsigned int count,
		       unsigned char force_update)
{
	unsigned int data_array[16], i;
	for (i = 0; i < count; i++) {
		unsigned cmd;
		cmd = table[i].cmd;
		if(0x85 == cmd) {
			data_array[0] = 0x0003390a;
			data_array[1] = 0x00000085 | 
				(table[i].para_list[0]<<8) | 
				(table[i].para_list[1]<<16);

			dsi_set_cmdq(data_array, 2, 1);
			pr_info("%s 0x%08x 0x%08x\n", __func__, data_array[0], data_array[1]);
		} else {
			data_array[0] = 0x00001508 |
				(table[i].cmd << 16) |
				(table[i].para_list[0]<<24);
			dsi_set_cmdq(data_array, 1, 1);
			pr_info("%s 0x%08x\n", __func__, data_array[0]);
		}
		//msleep(5);
	}

}

/* --------------------------------------------------------------------------- */
/* LCM Driver Implementations */
/* --------------------------------------------------------------------------- */
static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
	memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}

 void lcm_set_gpio_output(unsigned int GPIO, unsigned int output)
{
	if (GPIO == 0xFFFFFFFF) {
#ifdef BUILD_LK
		printf("[LK/LCM] GPIO_LCD_RST_EN_X115 =  0x%x\n", GPIO_LCD_RST_EN_X115);
		printf("[LK/LCM] GPIO_LCD_PWR_EN_X115 =  0x%x\n", GPIO_LCD_PWR_EN_X115);
#elif (defined BUILD_UBOOT)
#else
#endif
		return;
	}
#ifdef BUILD_LK
	   mt_set_gpio_mode(GPIO, GPIO_MODE_00);
	   mt_set_gpio_dir(GPIO, GPIO_DIR_OUT);
	   mt_set_gpio_out(GPIO, (output > 0) ? GPIO_OUT_ONE : GPIO_OUT_ZERO);
#else
	   gpio_direction_output(GPIO, output);
	   gpio_set_value(GPIO, (output > 0) ? GPIO_OUT_ONE : GPIO_OUT_ZERO);
#endif
}

static void lcm_get_params(LCM_PARAMS *params)
{
    memset(params, 0, sizeof(LCM_PARAMS));

    params->type = LCM_TYPE_DSI;

    params->width = FRAME_WIDTH;
    params->height = FRAME_HEIGHT;

	params->lcm_if = LCM_INTERFACE_DSI_DUAL;
	params->lcm_cmd_if = LCM_INTERFACE_DSI_DUAL;

#if (LCM_DSI_CMD_MODE)
	params->dsi.mode = CMD_MODE;
	params->dsi.switch_mode = SYNC_PULSE_VDO_MODE;
#else
    params->dsi.mode = BURST_VDO_MODE;
#endif
	params->dsi.switch_mode_enable = 0;
	// DSI
	/* Command mode setting */
	params->dsi.LANE_NUM = LCM_FOUR_LANE;
	//The following defined the fomat for data coming from LCD engine.
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format = LCM_DSI_FORMAT_RGB888;

	params->dsi.packet_size = 256;
	params->dsi.PS = LCM_PACKED_PS_24BIT_RGB888;

	params->dsi.vertical_sync_active = 1;
	params->dsi.vertical_backporch = 20;
	params->dsi.vertical_frontporch = 20;
	params->dsi.vertical_active_line = FRAME_HEIGHT;

	params->dsi.horizontal_sync_active = 1;
	params->dsi.horizontal_backporch = 64;
	params->dsi.horizontal_frontporch = 64;
	params->dsi.horizontal_active_pixel = FRAME_WIDTH;

	params->dsi.PLL_CLOCK = 450;  //480
	params->density=240;
	params->dsi.ssc_disable = 1;
	params->dsi.cont_clock = 1;

	params->dsi.ufoe_enable  = 1;
	params->dsi.ufoe_params.lr_mode_en = 1;
	//params->dsi.lcm_ext_te_enable = 1;

	params->dsi.clk_lp_per_line_enable = 0;

}

static void lcm_init_power(void)
{
	pr_warn("%s\n", __func__);
}

static void lcm_suspend_power(void)
{
	pr_info("%s-\n", __func__);

	msleep(50);
	lcm_set_gpio_output(GPIO_LCD_RST_EN_X115, GPIO_OUT_ZERO);
	msleep(10);

	lcm_set_gpio_output(GPIO_LCD_PWR_EN_X115, GPIO_OUT_ZERO);
	msleep(50);

	/* turn off 3.3V */
	lcm_set_gpio_output(GPIO_LCD_PWR2_EN_X115, GPIO_OUT_ZERO);
	msleep(10);

    //  	lcm_set_gpio_output(LCM_POWER_EN, GPIO_OUT_ZERO);
	//msleep(10);

}

static void lcm_suspend(void)
{
	unsigned int data_array[16];
	pr_info("%s+\n", __func__);
#if 0	
#error  "ddp_stop, mipi down~~"
	DSI_clk_HS_mode(DISP_MODULE_DSI0, NULL, 1);
	DSI_clk_HS_mode(DISP_MODULE_DSI1, NULL, 1);
	msleep(15);
	push_table(lcm_suspend_setting,
	       sizeof(lcm_suspend_setting) /
	       sizeof(struct LCM_setting_table), 1);
	msleep(120);
#endif
		/*wait TCON_RDY min 280mS(cmd_mode)*/
   // msleep(400);
	DSI_clk_HS_mode(DISP_MODULE_DSI0, NULL, 1);
	DSI_clk_HS_mode(DISP_MODULE_DSI1, NULL, 1);
        msleep(5);
	/*sleep out 0x11*/
	data_array[0] = 0x00280508;
	dsi_set_cmdq(data_array, 1, 1);
	//msleep(10);
	//data_array[0] = 0x00100508;
	//dsi_set_cmdq(data_array, 1, 1);
	//msleep(120);	
	msleep(180);
}

static void lcm_resume_power(void)
{
	pr_info("%s-\n", __func__);
	

	msleep(10);
	lcm_set_gpio_output(GPIO_LCD_PWR2_EN_X115, GPIO_OUT_ONE);

	msleep(30);

	lcm_set_gpio_output(GPIO_LCD_PWR_EN_X115, GPIO_OUT_ONE);
	msleep(30);

	lcm_set_gpio_output(GPIO_LCD_RST_EN_X115, GPIO_OUT_ONE);
	msleep(15);

}

static void lcm_resume(void)
{
	unsigned int data_array[16];
	pr_info("%s+\n", __func__);

	/*wait TCON_RDY min 280mS(cmd_mode)*/
        msleep(290);
	DSI_clk_HS_mode(DISP_MODULE_DSI0, NULL, 1);
	DSI_clk_HS_mode(DISP_MODULE_DSI1, NULL, 1);
        msleep(20);
	/*sleep out 0x11*/
	//data_array[0] = 0x00110508;
	//dsi_set_cmdq(data_array, 1, 1);
	//msleep(120);
/*
	//34nit
	82 12, 85 23 01, b0 34, bb 1f, b0 2e, bb 01
*/
	
	/*set brightness*/
	/*34nit*/	
	/*82. gamma*/
	data_array[0] = 0x12821508;
	dsi_set_cmdq(data_array, 1, 1);
	msleep(5);
	/*85. aor*/
	data_array[0] = 0x0003390a;
	data_array[1] = 0x00012385;
	dsi_set_cmdq(data_array, 2, 1);
	msleep(10);
	/*34. elvss*/
	data_array[0] = 0x34b01508;
	dsi_set_cmdq(data_array, 1, 1);
	data_array[0] = 0x1fbb1508;
	dsi_set_cmdq(data_array, 1, 1);
	msleep(5);
	/*update gamma*/
	data_array[0] = 0x2eb01508;
	dsi_set_cmdq(data_array, 1, 1);
	msleep(5);
	data_array[0] = 0x01bb1508;
	dsi_set_cmdq(data_array, 1, 1);
	msleep(5);
	
	data_array[0] = 0x00290508;
	dsi_set_cmdq(data_array, 1, 1);
	msleep(10);
}


static void lcm_init(void)
{
	pr_info("%s+\n", __func__);
}

void lcm_cmd_set_backlight(unsigned int level)
{
	pr_warn("%s: level=%d\n", __func__, level);
	switch(level) {
		case 0 ... 29:    push_table(lcm_initialization_setting_30nit,  sizeof(lcm_initialization_setting_30nit)  / sizeof(struct LCM_setting_table), 1); break;
		case 30 ... 34:   push_table(lcm_initialization_setting_34nit,  sizeof(lcm_initialization_setting_34nit)  / sizeof(struct LCM_setting_table), 1); break;
		case 35 ... 39:   push_table(lcm_initialization_setting_39nit,  sizeof(lcm_initialization_setting_39nit)  / sizeof(struct LCM_setting_table), 1); break;
		case 40 ... 44:	  push_table(lcm_initialization_setting_44nit,  sizeof(lcm_initialization_setting_44nit)  / sizeof(struct LCM_setting_table), 1); break;
		case 45 ... 49:	  push_table(lcm_initialization_setting_50nit,  sizeof(lcm_initialization_setting_50nit)  / sizeof(struct LCM_setting_table), 1); break;
		case 50 ... 54:   push_table(lcm_initialization_setting_56nit,  sizeof(lcm_initialization_setting_56nit)  / sizeof(struct LCM_setting_table), 1); break;
		case 55 ... 64:   push_table(lcm_initialization_setting_64nit,  sizeof(lcm_initialization_setting_64nit)  / sizeof(struct LCM_setting_table), 1); break;
		case 65 ... 74:   push_table(lcm_initialization_setting_72nit,  sizeof(lcm_initialization_setting_72nit)  / sizeof(struct LCM_setting_table), 1); break;
		case 75 ... 83:   push_table(lcm_initialization_setting_82nit,  sizeof(lcm_initialization_setting_82nit)  / sizeof(struct LCM_setting_table), 1); break;
		case 84 ... 93:   push_table(lcm_initialization_setting_93nit,  sizeof(lcm_initialization_setting_93nit)  / sizeof(struct LCM_setting_table), 1); break;
		case 94 ... 103:  push_table(lcm_initialization_setting_105nit, sizeof(lcm_initialization_setting_105nit) / sizeof(struct LCM_setting_table), 1); break;
		case 104 ... 113: push_table(lcm_initialization_setting_119nit, sizeof(lcm_initialization_setting_119nit) / sizeof(struct LCM_setting_table), 1); break;
		case 114 ... 122: push_table(lcm_initialization_setting_134nit, sizeof(lcm_initialization_setting_134nit) / sizeof(struct LCM_setting_table), 1); break;
		case 123 ... 132: push_table(lcm_initialization_setting_152nit, sizeof(lcm_initialization_setting_152nit) / sizeof(struct LCM_setting_table), 1); break;
		case 133 ... 142: push_table(lcm_initialization_setting_172nit, sizeof(lcm_initialization_setting_172nit) / sizeof(struct LCM_setting_table), 1); break;
		case 143 ... 169: push_table(lcm_initialization_setting_195nit, sizeof(lcm_initialization_setting_195nit) / sizeof(struct LCM_setting_table), 1); break;
		case 170 ... 205: push_table(lcm_initialization_setting_220nit, sizeof(lcm_initialization_setting_220nit) / sizeof(struct LCM_setting_table), 1); break;
		case 206 ... 230: push_table(lcm_initialization_setting_249nit, sizeof(lcm_initialization_setting_249nit) / sizeof(struct LCM_setting_table), 1); break;
		case 231 ... 250: push_table(lcm_initialization_setting_282nit, sizeof(lcm_initialization_setting_282nit) / sizeof(struct LCM_setting_table), 1); break;
		case 251 ... 255: push_table(lcm_initialization_setting_300nit, sizeof(lcm_initialization_setting_300nit) / sizeof(struct LCM_setting_table), 1); break;
	}
}

void lcm_set_backlight(unsigned int level)
{
	pr_warn("%s: level=%d\n", __func__, level);
}	

void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
#if 0
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0>>8)&0xFF);
	unsigned char x0_LSB = (x0&0xFF);
	unsigned char x1_MSB = ((x1>>8)&0xFF);
	unsigned char x1_LSB = (x1&0xFF);
	unsigned char y0_MSB = ((y0>>8)&0xFF);
	unsigned char y0_LSB = (y0&0xFF);
	unsigned char y1_MSB = ((y1>>8)&0xFF);
	unsigned char y1_LSB = (y1&0xFF);

	unsigned int data_array[16];

	data_array[0] = 0x00053902;
	data_array[1] = (x1_MSB << 24) | (x0_LSB << 16) | (x0_MSB << 8) | 0x2a;
	data_array[2] = (x1_LSB);
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0] = 0x00053902;
	data_array[1] = (y1_MSB << 24) | (y0_LSB << 16) | (y0_MSB << 8) | 0x2b;
	data_array[2] = (y1_LSB);
	dsi_set_cmdq(data_array, 3, 1);
	
	data_array[0] = 0x002c3909;
	dsi_set_cmdq(data_array, 1, 0);
#endif
}

LCM_DRIVER amsa05bv_dsi_vdo_mt8173_lcm_drv = {
	.name = "amsa05bv_dsi_vdo_mt8173",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params = lcm_get_params,
	.init = lcm_init,
	.suspend = lcm_suspend,
	.resume = lcm_resume,
	.set_backlight = lcm_set_backlight,
	/*.compare_id     = lcm_compare_id, */
	.init_power = lcm_init_power,
	.resume_power = lcm_resume_power,
	.suspend_power = lcm_suspend_power,
	.update         = lcm_update,
};

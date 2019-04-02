#ifndef BUILD_LK
#include <linux/string.h>
#include <linux/gpio.h>
#include <linux/pinctrl/consumer.h>
#endif

#ifdef BUILD_LK
#include <platform/gpio_const.h>
#include <platform/mt_gpio.h>
#include <platform/mt_pmic.h>
#include <string.h>
#include <platform/upmu_common.h>
#else
#include <mach/upmu_hw.h>
#include <asm-generic/gpio.h>
#include <mt-plat/mt_gpio.h>
#include <mt-plat/upmu_common.h>
#endif
#include "lcm_drv.h"
#include <mt-plat/mt_gpio.h>
extern unsigned int GPIO_LCM_PWR;
extern unsigned int GPIO_LCM_RST;
extern unsigned int GPIO_LCM_VDD;

#define GPIO_LCD_RST_EN        GPIO_LCM_RST
#define GPIO_LCM_PWREN         GPIO_LCM_PWR
#define GPIO_LCM_EN            GPIO_LCM_VDD

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------
#define FRAME_WIDTH (800)
#define FRAME_HEIGHT (1280)



// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define UDELAY(n) 											(lcm_util.udelay(n))
#define MDELAY(n) 											(lcm_util.mdelay(n))
#define REGFLAG_DELAY 0xFD
#define REGFLAG_END_OF_TABLE 0xFE
// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg											lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)    
struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};
static void lcm_set_gpio_output(unsigned int GPIO, unsigned int output)
{
	gpio_direction_output(GPIO, output);
    gpio_set_value(GPIO, output);
    /*mt_set_gpio_mode(GPIO, GPIO_MODE_00);
    mt_set_gpio_dir(GPIO, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO, (output>0)? GPIO_OUT_ONE: GPIO_OUT_ZERO);*/
}


static void lcd_power_en(unsigned char enabled)
{
    lcm_set_gpio_output(GPIO_LCM_EN, enabled);
}

static void lcd_vdd_enable(unsigned char enabled)
{
	lcm_set_gpio_output(GPIO_LCM_PWREN, enabled);
	
	if (enabled) {
	    pmic_config_interface(DIGLDO_CON30, 0x3, PMIC_RG_VGP3_VOSEL_MASK, PMIC_RG_VGP3_VOSEL_SHIFT);
		pmic_config_interface(DIGLDO_CON9, 0x1, PMIC_RG_VGP3_EN_MASK, PMIC_RG_VGP3_EN_SHIFT);
		
		lcm_set_gpio_output(GPIO_LCM_VDD, GPIO_OUT_ONE);
        MDELAY(20);

	} else {	
		pmic_config_interface(DIGLDO_CON30, 0x0, PMIC_RG_VGP3_VOSEL_MASK, PMIC_RG_VGP3_VOSEL_SHIFT);
		pmic_config_interface(DIGLDO_CON9, 0x0, PMIC_RG_VGP3_EN_MASK, PMIC_RG_VGP3_EN_SHIFT);
		
		lcm_set_gpio_output(GPIO_LCM_VDD, 0);
        MDELAY(20);      
	}

}

static void lcd_reset(unsigned char enabled)
{
	lcm_set_gpio_output(GPIO_LCD_RST_EN, enabled);
}

// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
	memset(params, 0, sizeof(LCM_PARAMS));

	params->type   = LCM_TYPE_DSI;

	params->width  = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;

	params->dsi.mode   = BURST_VDO_MODE; //SYNC_PULSE_VDO_MODE;//BURST_VDO_MODE; 

	params->dsi.LANE_NUM				= LCM_FOUR_LANE; //LCM_FOUR_LANE;

	params->dsi.data_format.format		= LCM_DSI_FORMAT_RGB888;

	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

	params->dsi.vertical_sync_active = 4; 
    params->dsi.vertical_backporch					=12;
    params->dsi.vertical_frontporch 				=20;
    params->dsi.vertical_active_line				= FRAME_HEIGHT; 

    params->dsi.horizontal_sync_active				= 20; //20;
    params->dsi.horizontal_backporch				= 20; //20;
    params->dsi.horizontal_frontporch				= 30;
	params->dsi.horizontal_active_pixel = FRAME_WIDTH;

	params->dsi.PLL_CLOCK = 210;//256;	// 256  251 


}
static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;
	for(i = 0; i < count; i++) {
		unsigned cmd;
		cmd = table[i].cmd;
		switch (cmd) {
		case REGFLAG_DELAY :
			MDELAY(table[i].count);
			break;
		case REGFLAG_END_OF_TABLE :
			break;
		default:
			dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
		}
	}
}
//data1 = 0x67011500;

static struct LCM_setting_table lcm_initialization_setting[] = {	
		{0xE0,1,{0x00}},
        {0xE1,1,{0x93}},
        {0xE2,1,{0x65}},
		{0xE3,1,{0xF8}},
		{0x80,1,{0x03}},
		{0x70,1,{0x10}},
		{0x71,1,{0x13}},
		{0x72,1,{0x06}},
		{0x75,1,{0x03}},
		{0xE0,1,{0x04}},
		{0x2D,1,{0x03}},
		{0xE0,1,{0x01}},
		{0x00,1,{0x00}},
		{0x01,1,{0x5A}},
		{0x03,1,{0x00}},
		{0x04,1,{0x58}},
		{0x17,1,{0x00}},
		{0x18,1,{0xEF}},
		{0x19,1,{0x01}},
		{0x1A,1,{0x00}},
		{0x1B,1,{0xEF}},
		{0x1C,1,{0x01}},
		{0x1F,1,{0x7A}},
		{0x20,1,{0x24}},
		{0x21,1,{0x24}},
		{0x22,1,{0x4E}},
		{0x37,1,{0x59}},
		{0x35,1,{0x2C}},
		{0x38,1,{0x05}},
		{0x39,1,{0x08}},
		{0x3A,1,{0x10}},
		{0x3C,1,{0x88}},
		{0x3D,1,{0xFF}},
		{0x3E,1,{0xFF}},
		{0x3F,1,{0x7F}},
		{0x40,1,{0x06}},
		{0x41,1,{0xA0}},
		{0x43,1,{0x14}},
		{0x44,1,{0x0F}},
		{0x45,1,{0x24}},
		{0x55,1,{0x01}},
		{0x56,1,{0x01}},
		{0x57,1,{0x89}},
		{0x58,1,{0x0A}},
		{0x59,1,{0x0A}},
		{0x5A,1,{0x39}},
		{0x5B,1,{0x10}},
		{0x5C,1,{0x16}},
		{0x5D,1,{0x7A}},
		{0x5E,1,{0x65}},
		{0x5F,1,{0x56}},
		{0x60,1,{0x49}},
		{0x61,1,{0x43}},
		{0x62,1,{0x33}},
		{0x63,1,{0x35}},
		{0x64,1,{0x1C}},
		{0x65,1,{0x33}},
		{0x66,1,{0x30}},
		{0x67,1,{0x2E}},
		{0x68,1,{0x4A}},
		{0x69,1,{0x36}},
		{0x6A,1,{0x3D}},
		{0x6B,1,{0x2F}},
		{0x6C,1,{0x2D}},
		{0x6D,1,{0x23}},
		{0x6E,1,{0x15}},
		{0x6F,1,{0x04}},
		{0x70,1,{0x7A}},
		{0x71,1,{0x65}},
		{0x72,1,{0x56}},
		{0x73,1,{0x49}},
		{0x74,1,{0x43}},
		{0x75,1,{0x33}},
		{0x76,1,{0x35}},
		{0x77,1,{0x1C}},
		{0x78,1,{0x33}},
		{0x79,1,{0x30}},
		{0x7A,1,{0x2E}},
		{0x7B,1,{0x4A}},
		{0x7C,1,{0x36}},
		{0x7D,1,{0x3D}},
		{0x7E,1,{0x2F}},
		{0x7F,1,{0x2D}},
		{0x80,1,{0x23}},
		{0x81,1,{0x15}},
		{0x82,1,{0x04}},
		{0xE0,1,{0x02}},
		{0x00,1,{0x1E}},
		{0x01,1,{0x1F}},
		{0x02,1,{0x57}},
		{0x03,1,{0x58}},
		{0x04,1,{0x48}},
		{0x05,1,{0x4A}},
		{0x06,1,{0x44}},
		{0x07,1,{0x46}},
		{0x08,1,{0x40}},
		{0x09,1,{0x1F}},
		{0x0A,1,{0x1F}},
		{0x0B,1,{0x1F}},
		{0x0C,1,{0x1F}},
		{0x0D,1,{0x1F}},
		{0x0E,1,{0x1F}},
		{0x0F,1,{0x42}},
		{0x10,1,{0x1F}},
		{0x11,1,{0x1F}},
		{0x12,1,{0x1F}},
		{0x13,1,{0x1F}},
		{0x14,1,{0x1F}},
		{0x15,1,{0x1F}},
		{0x16,1,{0x1E}},
		{0x17,1,{0x1F}},
		{0x18,1,{0x57}},
		{0x19,1,{0x58}},
		{0x1A,1,{0x49}},
		{0x1B,1,{0x4B}},
		{0x1C,1,{0x45}},
		{0x1D,1,{0x47}},
		{0x1E,1,{0x41}},
		{0x1F,1,{0x1F}},
		{0x20,1,{0x1F}},
		{0x21,1,{0x1F}},
		{0x22,1,{0x1F}},
		{0x23,1,{0x1F}},
		{0x24,1,{0x1F}},
		{0x25,1,{0x43}},
		{0x26,1,{0x1F}},
		{0x27,1,{0x1F}},
		{0x28,1,{0x1F}},
		{0x29,1,{0x1F}},
		{0x2A,1,{0x1F}},
		{0x2B,1,{0x1F}},
		{0x2C,1,{0x1F}},
		{0x2D,1,{0x1E}},
		{0x2E,1,{0x17}},
		{0x2F,1,{0x18}},
		{0x30,1,{0x07}},
		{0x31,1,{0x05}},
		{0x32,1,{0x0B}},
		{0x33,1,{0x09}},
		{0x34,1,{0x03}},
		{0x35,1,{0x1F}},
		{0x36,1,{0x1F}},
		{0x37,1,{0x1F}},
		{0x38,1,{0x1F}},
		{0x39,1,{0x1F}},
		{0x3A,1,{0x1F}},
		{0x3B,1,{0x01}},
		{0x3C,1,{0x1F}},
		{0x3D,1,{0x1F}},
		{0x3E,1,{0x1F}},
		{0x3F,1,{0x1F}},
		{0x40,1,{0x1F}},
		{0x41,1,{0x1F}},
		{0x42,1,{0x1F}},
		{0x43,1,{0x1E}},
		{0x44,1,{0x17}},
		{0x45,1,{0x18}},
		{0x46,1,{0x06}},
		{0x47,1,{0x04}},
		{0x48,1,{0x0A}},
		{0x49,1,{0x08}},
		{0x4A,1,{0x02}},
		{0x4B,1,{0x1F}},
		{0x4C,1,{0x1F}},
		{0x4D,1,{0x1F}},
		{0x4E,1,{0x1F}},
		{0x4F,1,{0x1F}},
		{0x50,1,{0x1F}},
		{0x51,1,{0x00}},
		{0x52,1,{0x1F}},
		{0x53,1,{0x1F}},
		{0x54,1,{0x1F}},
		{0x55,1,{0x1F}},
		{0x56,1,{0x1F}},
		{0x57,1,{0x1F}},
		{0x58,1,{0x40}},
		{0x59,1,{0x00}},
		{0x5A,1,{0x00}},
		{0x5B,1,{0x30}},
		{0x5C,1,{0x05}},
		{0x5D,1,{0x30}},
		{0x5E,1,{0x01}},
		{0x5F,1,{0x02}},
		{0x60,1,{0x30}},
		{0x61,1,{0x03}},
		{0x62,1,{0x04}},
		{0x63,1,{0x6A}},
		{0x64,1,{0x6A}},
		{0x65,1,{0x75}},
		{0x66,1,{0x0D}},
		{0x67,1,{0x73}},
		{0x68,1,{0x09}},
		{0x69,1,{0x6A}},
		{0x6A,1,{0x6A}},
		{0x6B,1,{0x08}},
		{0x6C,1,{0x00}},
		{0x6D,1,{0x04}},
		{0x6E,1,{0x00}},
		{0x6F,1,{0x88}},
		{0x70,1,{0x00}},
		{0x71,1,{0x00}},
		{0x72,1,{0x06}},
		{0x73,1,{0x7B}},
		{0x74,1,{0x00}},
		{0x75,1,{0xBC}},
		{0x76,1,{0x00}},
		{0x77,1,{0x0D}},
		{0x78,1,{0x23}},
		{0x79,1,{0x00}},
		{0x7A,1,{0x00}},
		{0x7B,1,{0x00}},
		{0x7C,1,{0x00}},
		{0x7D,1,{0x03}},
		{0x7E,1,{0x7B}},
		{0xE0,1,{0x04}},
		{0x2B,1,{0x2B}},
		{0x2E,1,{0x44}},
		{0xE0,1,{0x00}},
		{0xE6,1,{0x02}},
		{0xE7,1,{0x02}},
		{0x11,1,{0x00}},
		{REGFLAG_DELAY, 120, {}},
		{0x29,1,{0x00}},
		{REGFLAG_DELAY, 20, {}},		
};


static void lcm_init(void)
{
	lcd_power_en(1);

	MDELAY(50);
	lcd_vdd_enable(1);
	MDELAY(30);
	
	lcd_reset(1);
	MDELAY(50);
	lcd_reset(0);
	MDELAY(50);
	lcd_reset(1);
	
	MDELAY(80);//Must > 5ms
	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
	MDELAY(80);//Must > 5ms
}


static void lcm_suspend(void)
{
    unsigned int data_array[16];

	data_array[0]=0x00280500; // Display Off
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(5); 

	data_array[0] = 0x00100500; // Sleep In
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(5); 

	lcd_reset(0);	
	MDELAY(10);	

	lcd_power_en(0);
	MDELAY(10);
		
	lcd_vdd_enable(0);
	MDELAY(80);

}


static void lcm_resume(void)
{
    lcm_init();
}

LCM_DRIVER c716_pb_jd9365_hsd_wxga_ips_101_lcm_drv = 
{
	.name = "c716_pb_jd9365_hsd_wxga_ips_101",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params = lcm_get_params,
	.init = lcm_init,
	.suspend = lcm_suspend,
	.resume = lcm_resume,
};

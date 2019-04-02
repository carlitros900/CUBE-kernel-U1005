#ifdef BUILD_LK
#include <platform/mt_gpio.h>
#include <platform/mt_i2c.h>
#include <platform/mt_pmic.h>
#include <string.h>
#else
#include <linux/string.h>
#include <linux/wait.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/pinctrl/consumer.h>
#include <linux/of_gpio.h>
#include <asm-generic/gpio.h>

#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>

#ifdef CONFIG_OF
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/regulator/consumer.h>
#include <linux/clk.h>
#endif
#endif
#include "lcm_drv.h"
#include <mt-plat/mt_gpio.h>
extern unsigned int GPIO_LCM_PWR;
extern unsigned int GPIO_LCM_VDD;
extern unsigned int GPIO_LCM_RST;

//---------------------------------------------------------------------------
//LocalConstants
//---------------------------------------------------------------------------

#define FRAME_WIDTH (800)
#define FRAME_HEIGHT (1280)
//---------------------------------------------------------------------------
//LocalVariables
//---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))

//---------------------------------------------------------------------------
//LocalFunctions
//---------------------------------------------------------------------------
#define dsi_set_cmdq_V3(para_tbl,size,force_update) lcm_util.dsi_set_cmdq_V3(para_tbl,size,force_update)
#define dsi_set_cmdq_V2(cmd,count,ppara,force_update) lcm_util.dsi_set_cmdq_V2(cmd,count,ppara,force_update)
#define dsi_set_cmdq(pdata,queue_size,force_update) lcm_util.dsi_set_cmdq(pdata,queue_size,force_update)
#define wrtie_cmd(cmd) lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr,pdata,byte_nums) lcm_util.dsi_write_regs(addr,pdata,byte_nums)
#define read_reg lcm_util.dsi_read_reg()
#define read_reg_v2(cmd,buffer,buffer_size) lcm_util.dsi_dcs_read_lcm_reg_v2(cmd,buffer,buffer_size)

static void lcm_set_gpio_output(unsigned int GPIO, unsigned int output)
{
        gpio_direction_output(GPIO, output);
        gpio_set_value(GPIO, output);
}


// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------


static void lcd_power_en(unsigned char enabled)
{

	if (enabled)
	{
	    lcm_set_gpio_output(GPIO_LCM_VDD, GPIO_OUT_ONE);
        MDELAY(20);

	}
	else
	{	
		lcm_set_gpio_output(GPIO_LCM_VDD, 0);
        MDELAY(20);      
	}

	
}

static void lcd_vdd(unsigned char enabled)
{
    if (enabled)
    {
        lcm_set_gpio_output(GPIO_LCM_PWR, GPIO_OUT_ONE);

    }
    else
    {   
        lcm_set_gpio_output(GPIO_LCM_PWR, 0);      
    }
    
}

static void lcd_reset(unsigned char enabled)
{

    if (enabled)
    {
        lcm_set_gpio_output(GPIO_LCM_RST, GPIO_OUT_ONE);
        MDELAY(20);

    }
    else
    {   
        lcm_set_gpio_output(GPIO_LCM_RST, 0);
        MDELAY(20);      
    }

    
}

static void init_lcm_registers(void)
{
	unsigned int data_array[16];
data_array[0] = 0x00043902;
data_array[1] = 0x9483FFB9;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00113902;
data_array[1] = 0x008213BA;
data_array[2] = 0x1000C516;
data_array[3] = 0x03240FFF;
data_array[4] = 0x20252421;
data_array[5] = 0x00000008;
dsi_set_cmdq(data_array, 6, 1);
MDELAY(1);

data_array[0] = 0x00123902;
data_array[1] = 0x040001B1;
data_array[2] = 0xF11203C4;
data_array[3] = 0x3F3F2C24;
data_array[4] = 0xE6000247;
data_array[5] = 0x0000A6E2;
dsi_set_cmdq(data_array, 6, 1);
MDELAY(1);

data_array[0] = 0x00073902;
data_array[1] = 0x0EC800B2;
data_array[2] = 0x00810030;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);

data_array[0] = 0x00203902;
data_array[1] = 0x320480B4;
data_array[2] = 0x15540810;
data_array[3] = 0x0810220F;
data_array[4] = 0x0A444347;
data_array[5] = 0x0244434B;
data_array[6] = 0x06025555;
data_array[7] = 0x0A5F0644;
data_array[8] = 0x0805706B;
dsi_set_cmdq(data_array, 9, 1);
MDELAY(1);

data_array[0] = 0x3fB61500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00373902;
data_array[1] = 0x000000D5;
data_array[2] = 0x01000A00;
data_array[3] = 0x66220022;
data_array[4] = 0x23010111;
data_array[5] = 0xBC9A6745;
data_array[6] = 0x8845BBAA;
data_array[7] = 0x88888888;
data_array[8] = 0x88678888;
data_array[9] = 0x29810888;
data_array[10] = 0x48088883;
data_array[11] = 0x68288581;
data_array[12] = 0x48888783;
data_array[13] = 0x00000085;
data_array[14] = 0x00013C00;
dsi_set_cmdq(data_array, 15, 1);
MDELAY(1);

data_array[0] = 0x09CC1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00053902;
data_array[1] = 0x100206BF;
data_array[2] = 0x00000004;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);

data_array[0] = 0x00053902;
data_array[1] = 0x001000C7;
data_array[2] = 0x00000010;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);

data_array[0] = 0x002b3902;
data_array[1] = 0x0e0800E0;
data_array[2] = 0x1c3F3430;
data_array[3] = 0x0E0c0639;
data_array[4] = 0x14121411;
data_array[5] = 0x18001811;
data_array[6] = 0x3F34300e;
data_array[7] = 0x0C06391c;
data_array[8] = 0x1214110E;
data_array[9] = 0x0B181114;
data_array[10] = 0x0B120717;
data_array[11] = 0x00120717;
dsi_set_cmdq(data_array, 12, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x00170CC0;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x000808C6;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x32D41500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00110500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(120);

data_array[0] = 0x00290500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(20);
}


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
	params->dsi.mode    = SYNC_PULSE_VDO_MODE;
	//params->dsi.mode	 = SYNC_EVENT_VDO_MODE; 

	// DSI
	/* Command mode setting */
	params->dsi.LANE_NUM				= LCM_FOUR_LANE;//LCM_THREE_LANE;//LCM_FOUR_LANE;

	params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;



	params->dsi.vertical_sync_active				= 4;
	params->dsi.vertical_backporch					= 12; //4//4
	params->dsi.vertical_frontporch					= 15;  //8
	params->dsi.vertical_active_line					= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active				= 36;//16
	params->dsi.horizontal_backporch				= 118;//48
	params->dsi.horizontal_frontporch				= 118;//16
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
	
	params->dsi.PLL_CLOCK = 200; 

}


static void lcm_init(void)
{
   

    lcd_reset(0);
    lcd_power_en(0);
	lcd_vdd(1);
    lcd_power_en(1);
    MDELAY(20);//Must > 5ms
    lcd_reset(1);
    MDELAY(10);//Must > 5ms
    lcd_reset(0);
    MDELAY(10);//Must > 5ms
    lcd_reset(1);
    MDELAY(10);
	init_lcm_registers();
	MDELAY(80);
	
}

static void lcm_suspend_jd9364(void)
{
    lcd_reset(0);
    MDELAY(10);
	lcd_vdd(0);
    lcd_power_en(0);
    MDELAY(20);
}

static void lcm_resume_jd9364(void)
{

	lcm_init();
}


       
LCM_DRIVER c805_zs_hx8394a01_inx_wxga_ips_8_lcm_drv= 
{
	.name			= "c805_zs_hx8394a01_inx_wxga_ips_8",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend			=lcm_suspend_jd9364,
	.resume				=lcm_resume_jd9364,	
};

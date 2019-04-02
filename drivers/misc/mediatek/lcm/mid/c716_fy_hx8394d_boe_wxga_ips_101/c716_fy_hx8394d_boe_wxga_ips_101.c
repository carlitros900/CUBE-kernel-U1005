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

// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg											lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)    

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

	params->dsi.LANE_NUM				= LCM_THREE_LANE; //LCM_FOUR_LANE;

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

	params->dsi.PLL_CLOCK = 285;//256;	// 256  251 


}

//data1 = 0x67011500;
static void init_lcm_registers(void)
{
	unsigned int data_array[16];
	data_array[0] = 0x00043902;
	data_array[1] = 0x9483FFB9;
	dsi_set_cmdq(data_array, 2, 1);
	MDELAY(1);
	
	data_array[0] = 0x72BA1500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);
	
	data_array[0] = 0x00830500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);
	
	data_array[0] = 0x00103902;
	data_array[1] = 0x0F0F6CB1;
	data_array[2] = 0xF1110425;
	data_array[3] = 0x239A6881;
	data_array[4] = 0x58D2C080;
	dsi_set_cmdq(data_array, 5, 1);
	MDELAY(1);
	
	data_array[0] = 0x000c3902;
	data_array[1] = 0x0F6400B2;
	data_array[2] = 0x081C2009;
	data_array[3] = 0x004D1C08;
	dsi_set_cmdq(data_array, 4, 1);
	MDELAY(1);
	
	data_array[0] = 0x000d3902;
	data_array[1] = 0x07FF00B4;
	data_array[2] = 0x07580758;
	data_array[3] = 0x01700158;
	data_array[4] = 0x00000070;
	dsi_set_cmdq(data_array, 5, 1);
	MDELAY(1);
	
	data_array[0] = 0x00D21500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);
	
	data_array[0] = 0x00213902;
	data_array[1] = 0x000000D3;
	data_array[2] = 0x08080701;
	data_array[3] = 0x00011032;
	data_array[4] = 0x0A103201;
	data_array[5] = 0x32000A00;
	data_array[6] = 0x33000810;
	data_array[7] = 0x37030333;
	data_array[8] = 0x08370D0D;
	data_array[9] = 0x00000008;
	dsi_set_cmdq(data_array, 10, 1);
	MDELAY(1);
	
	data_array[0] = 0x002d3902;
	data_array[1] = 0x191919D5;
	data_array[2] = 0x21202119;
	data_array[3] = 0x23222320;
	data_array[4] = 0x18181822;
	data_array[5] = 0x18181818;
	data_array[6] = 0x19191918;
	data_array[7] = 0x03181819;
	data_array[8] = 0x01020302;
	data_array[9] = 0x07000100;
	data_array[10] = 0x05060706;
	data_array[11] = 0x18040504;
	data_array[12] = 0x00000018;
	dsi_set_cmdq(data_array, 13, 1);
	MDELAY(1);
	
	data_array[0] = 0x002b3902;
	data_array[1] = 0x100900E0;
	data_array[2] = 0x1F3D332D;
	data_array[3] = 0x0D0B073D;
	data_array[4] = 0x16131018;
	data_array[5] = 0x12071514;
	data_array[6] = 0x0A001814;
	data_array[7] = 0x3D332D10;
	data_array[8] = 0x0A073D20;
	data_array[9] = 0x130E170D;
	data_array[10] = 0x07141315;
	data_array[11] = 0x00171311;
	dsi_set_cmdq(data_array, 12, 1);
	MDELAY(1);
	
	data_array[0] = 0x002b3902;
	data_array[1] = 0x130C00E0;
	data_array[2] = 0x243D3834;
	data_array[3] = 0x0D0B0843;
	data_array[4] = 0x14120F17;
	data_array[5] = 0x12071312;
	data_array[6] = 0x0D001814;
	data_array[7] = 0x3D383413;
	data_array[8] = 0x0B074324;
	data_array[9] = 0x120E180D;
	data_array[10] = 0x07131214;
	data_array[11] = 0x00171412;
	dsi_set_cmdq(data_array, 12, 1);
	MDELAY(1);
	
	data_array[0] = 0x00033902;
	data_array[1] = 0x004646B6;
	dsi_set_cmdq(data_array, 2, 1);
	MDELAY(1);
	
	data_array[0] = 0x09CC1500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);
	
	data_array[0] = 0xBDC61500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);
	
	data_array[0] = 0x00053902;
	data_array[1] = 0x40C000C7;
	data_array[2] = 0x000000C0;
	dsi_set_cmdq(data_array, 3, 1);
	MDELAY(1);
	
	data_array[0] = 0x00033902;
	data_array[1] = 0x001430C0;
	dsi_set_cmdq(data_array, 2, 1);
	MDELAY(1);
	
	data_array[0] = 0x02BD1500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);
	
	data_array[0] = 0x000d3902;
	data_array[1] = 0xEFFFFFD8;
	data_array[2] = 0xFFF0FFBF;
	data_array[3] = 0xFFBFEFFF;
	data_array[4] = 0x000000F0;
	dsi_set_cmdq(data_array, 5, 1);
	MDELAY(1);
	
	data_array[0] = 0x00110500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(120);
	
	data_array[0] = 0x00290500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(20);
}


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
	
	init_lcm_registers();

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

LCM_DRIVER c716_fy_hx8394d_boe_wxga_ips_101_lcm_drv = 
{
	.name = "c716_fy_hx8394d_boe_wxga_ips_101",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params = lcm_get_params,
	.init = lcm_init,
	.suspend = lcm_suspend,
	.resume = lcm_resume,
};

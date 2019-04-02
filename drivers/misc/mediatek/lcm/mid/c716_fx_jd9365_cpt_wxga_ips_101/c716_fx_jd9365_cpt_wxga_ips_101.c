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
    params->dsi.vertical_backporch					=4;
    params->dsi.vertical_frontporch 				=16;
    params->dsi.vertical_active_line				= FRAME_HEIGHT; 

    params->dsi.horizontal_sync_active				= 18; //20;
    params->dsi.horizontal_backporch				= 18; //20;
    params->dsi.horizontal_frontporch				= 18;
	params->dsi.horizontal_active_pixel = FRAME_WIDTH;

	params->dsi.PLL_CLOCK = 285;//256;	// 256  251 


}

//data1 = 0x67011500;
static void init_lcm_registers(void)
{		
	unsigned int data_array[16];

data_array[0] = 0x00E01500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x93E11500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x65E21500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0xF8E31500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x02801500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x04E01500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x032D1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x00E01500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x10701500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x13711500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x06721500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x01E01500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x00001500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x7B011500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x00171500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0xC2181500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x01191500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x001A1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0xC21B1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x011C1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x3F1F1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x24201500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x24211500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x0E221500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x09371500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x04381500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x00391500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x013A1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x783C1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0xFF3D1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0xFF3E1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0xFF3F1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x06401500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0xA0411500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x08431500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x07441500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x24451500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x01551500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x01561500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x69571500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x0A581500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x2A591500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x285A1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x0F5B1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x7C5D1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x675E1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x555F1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x48601500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x3E611500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x2D621500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x30631500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x16641500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x2C651500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x28661500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x26671500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x42681500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x30691500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x386A1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x296B1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x266C1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x1C6D1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x0D6E1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x006F1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x7C701500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x69711500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x55721500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x48731500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x3E741500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x2D751500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x30761500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x16771500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x2C781500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x28791500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x267A1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x427B1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x307C1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x387D1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x297E1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x267F1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x1C801500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x0D811500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x00821500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x02E01500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x09001500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x05011500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x08021500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x04031500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x06041500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x0A051500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x07061500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x0B071500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x00081500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x1F091500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x1F0A1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x1F0B1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x1F0C1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x1F0D1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x1F0E1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x170F1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x37101500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x1F111500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x1F121500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x1E131500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x10141500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x1F151500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x09161500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x05171500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x08181500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x04191500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x061A1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x0A1B1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x071C1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x0B1D1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x001E1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x1F1F1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x1F201500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x1F211500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x1F221500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x1F231500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x1F241500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x17251500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x37261500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x1F271500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x1F281500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x1E291500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x102A1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x1F2B1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x01581500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x00591500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x005A1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x005B1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x015C1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x705D1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x005E1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x005F1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x40601500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x00611500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x00621500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x65631500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x65641500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x45651500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x09661500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x73671500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x05681500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x00691500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x646A1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x006B1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x006C1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x006D1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x006E1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x886F1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x00701500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x00711500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x06721500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x7B731500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x00741500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x80751500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x00761500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x05771500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x18781500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x00791500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x007A1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x007B1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x007C1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x037D1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x7B7E1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x04E01500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x2B2B1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x442E1500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x00E01500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x02E61500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x02E71500;
dsi_set_cmdq(data_array, 1, 1);
data_array[0] = 0x00110500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(120);
data_array[0] = 0x00290500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(5);
data_array[0] = 0x00351500;
dsi_set_cmdq(data_array, 1, 1);
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

LCM_DRIVER c716_fx_jd9365_cpt_wxga_ips_101_lcm_drv = 
{
	.name = "c716_fx_jd9365_cpt_wxga_ips_101",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params = lcm_get_params,
	.init = lcm_init,
	.suspend = lcm_suspend,
	.resume = lcm_resume,
};

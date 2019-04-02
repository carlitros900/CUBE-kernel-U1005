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
#include <mt-plat/upmu_common.h>

extern unsigned int GPIO_LCM_PWR;
extern struct regulator *lcm_vgp;





//---------------------------------------------------------------------------
//LocalConstants
//---------------------------------------------------------------------------

#define FRAME_WIDTH (800)
#define FRAME_HEIGHT (1280)
//---------------------------------------------------------------------------
//LocalVariables
//---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v) (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))
#define REGFLAG_DELAY 0xFD
#define REGFLAG_END_OF_TABLE 0xFE
#define GPIO_LCD_RST_EN 0xFFFFFFFF

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

// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------
static void lcm_set_gpio_output(unsigned int GPIO, unsigned int output)
{
        gpio_direction_output(GPIO, output);
        gpio_set_value(GPIO, output);
}

static void lcd_power_en(unsigned char enabled)
{
	if (enabled)
	{
	
	lcm_set_gpio_output(GPIO_LCM_PWR, GPIO_OUT_ONE);
		/* VGP2_PMU 3V */
		pmic_config_interface(DIGLDO_CON29, 0x6, PMIC_RG_VGP2_VOSEL_MASK, PMIC_RG_VGP2_VOSEL_SHIFT);
		pmic_config_interface(DIGLDO_CON8, 0x1, PMIC_RG_VGP2_EN_MASK, PMIC_RG_VGP2_EN_SHIFT);

	}
	else
	{	
	
		lcm_set_gpio_output(GPIO_LCM_PWR, 0);
		/* VGP2_PMU 3V */
		pmic_config_interface(DIGLDO_CON8, 0x0, PMIC_RG_VGP2_EN_MASK, PMIC_RG_VGP2_EN_SHIFT);
		pmic_config_interface(DIGLDO_CON29, 0x0, PMIC_RG_VGP2_VOSEL_MASK, PMIC_RG_VGP2_VOSEL_SHIFT); 
	}

	
}


static void lcd_reset(unsigned char enabled)
{
	if (enabled)
	{
		lcm_set_gpio_output(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
	}
	else
	{	
		lcm_set_gpio_output(GPIO_LCD_RST_EN, GPIO_OUT_ZERO);
	}
}

//---------------------------------------------------------------------------
//LCMDriverImplementations
//---------------------------------------------------------------------------
static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util,util,sizeof(LCM_UTIL_FUNCS));
}

static void lcm_get_params(LCM_PARAMS *params)
{
    memset(params,0,sizeof(LCM_PARAMS));

    params->type=LCM_TYPE_DSI;
    params->width=FRAME_WIDTH;
    params->height=FRAME_HEIGHT;
	
    params->dsi.mode=BURST_VDO_MODE;    //SYNC_PULSE_VDO_MODE = 1,SYNC_EVENT_VDO_MODE = 2,BURST_VDO_MODE = 3

    //DSI
    /*Commandmodesetting*/
    params->dsi.LANE_NUM=LCM_FOUR_LANE;
    //ThefollowingdefinedthefomatfordatacomingfromLCDengine.
    params->dsi.data_format.format=LCM_DSI_FORMAT_RGB888;

    params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

    params->dsi.vertical_sync_active=4;//4;//4;//1;//4       
    params->dsi.vertical_backporch=18;//110;//3;//16;//4// 20 16
    params->dsi.vertical_frontporch=18;//50;//5;//15;//4// 20 15
    params->dsi.vertical_active_line=FRAME_HEIGHT;

    params->dsi.horizontal_sync_active=4;///4;//20 
    params->dsi.horizontal_backporch=20;//70;//48;//40;//50 20 80
    params->dsi.horizontal_frontporch=20;//40;//16;//40;//60 20 80
    params->dsi.horizontal_active_pixel=FRAME_WIDTH;




	//Bitratecalculation
	params->dsi.PLL_CLOCK=230;//230;

}

static void init_lcm_registers(void)
{
	unsigned int data_array[16];
data_array[0] = 0x00E01500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x93E11500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x65E21500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0xF8E31500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x03801500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x04E01500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x032D1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x01E01500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x61011500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00031500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x6C041500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00171500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0xD7181500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x01191500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x001A1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0xD71B1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x011C1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x6B1F1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x29201500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x29211500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x7E221500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x59371500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x28351500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x05381500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x08391500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x123A1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x7E3C1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0xFF3D1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0xFF3E1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x7F3F1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x06401500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0xA0411500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1E431500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x0B441500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x24451500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x0F551500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x01561500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x68571500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x0A581500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x0A591500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x295A1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x155B1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x165C1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x645D1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x4D5E1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x3E5F1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x33601500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x31611500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x23621500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x2A631500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x16641500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x30651500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x31661500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x31671500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x50681500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x3D691500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x446A1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x366B1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x306C1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x236D1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x116E1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x126F1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x64701500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x4D711500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x3E721500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x33731500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x31741500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x23751500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x2A761500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x16771500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x30781500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x31791500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x317A1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x507B1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x3D7C1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x447D1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x367E1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x307F1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x23801500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x11811500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x12821500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x02E01500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x53001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x5F011500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x5F021500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x5F031500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x5F041500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x51051500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x77061500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x57071500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x4F081500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x5F091500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x4D0A1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x4B0B1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x5F0C1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x490D1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x470E1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x450F1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x41101500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x5F111500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x5F121500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x5F131500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x5F141500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x5F151500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x52161500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x5F171500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x5F181500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x5F191500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x5F1A1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x501B1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x771C1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x571D1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x4E1E1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x5F1F1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x4C201500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x4A211500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x5F221500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x48231500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x46241500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x44251500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x40261500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x5F271500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x5F281500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x5F291500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x5F2A1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x5F2B1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x122C1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F2D1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F2E1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F2F1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F301500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00311500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x17321500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x17331500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x04341500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F351500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x06361500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x08371500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F381500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x0A391500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x0C3A1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x0E3B1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x103C1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F3D1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F3E1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F3F1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F401500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F411500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x13421500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F431500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F441500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F451500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F461500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x01471500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x17481500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x17491500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x054A1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F4B1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x074C1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x094D1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F4E1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x0B4F1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x0D501500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x0F511500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x11521500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F531500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F541500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F551500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F561500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F571500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00581500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00591500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x005A1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x105B1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x045C1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x405D1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x005E1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x005F1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x40601500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x03611500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x04621500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x70631500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x70641500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x75651500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x0B661500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0xB4671500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x06681500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x70691500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x706A1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x046B1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x006C1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x046D1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x006E1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x886F1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00701500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00711500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x06721500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x7B731500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00741500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0xBC751500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00761500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x05771500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x2A781500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00791500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x007A1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x007B1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x007C1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x037D1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x7B7E1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x04E01500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x480E1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x10091500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x2B2B1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x442E1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00E01500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x02E61500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x06E71500;
dsi_set_cmdq(data_array, 1, 1);
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
    MDELAY(80);
		
    lcd_reset(1);
    MDELAY(10);
    lcd_reset(0);
    MDELAY(10);
    lcd_reset(1);
    MDELAY(130); 
	
	init_lcm_registers();
}

static void lcm_suspend(void)
{
    unsigned int data_array[16];

    data_array[0]=0x00280500;//displayoff
    dsi_set_cmdq(data_array,1,1);
    MDELAY(15);
	
	lcd_reset(0);
    MDELAY(10);
	lcd_power_en(0);
	
}

static void lcm_resume(void)
{
    lcm_init();
}


LCM_DRIVER c102_qc_jd9364_inx_wxga_ips_101_lcm_drv={
	.name				="c102_qc_jd9364_inx_wxga_ips_101",
	.set_util_funcs	=lcm_set_util_funcs,
	.get_params			=lcm_get_params,
	.init				=lcm_init,
	.suspend			=lcm_suspend,
	.resume				=lcm_resume,
};

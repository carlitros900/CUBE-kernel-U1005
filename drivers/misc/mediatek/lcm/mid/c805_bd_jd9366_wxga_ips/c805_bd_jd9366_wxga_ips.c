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
		
#ifdef BUILD_LK 
	pmic_config_interface(DIGLDO_CON29, 0x6, PMIC_RG_VGP2_VOSEL_MASK, PMIC_RG_VGP2_VOSEL_SHIFT);
	pmic_config_interface(DIGLDO_CON8, 0x1, PMIC_RG_VGP2_EN_MASK, PMIC_RG_VGP2_EN_SHIFT);		
#else
	upmu_set_rg_vgp2_vosel(0x6);
	upmu_set_rg_vgp2_en(0x1);	
#endif

    }
    else
    {   
        lcm_set_gpio_output(GPIO_LCM_RST, 0);
#ifdef BUILD_LK 
	pmic_config_interface(DIGLDO_CON29, 0x0, PMIC_RG_VGP2_VOSEL_MASK, PMIC_RG_VGP2_VOSEL_SHIFT);
	pmic_config_interface(DIGLDO_CON8, 0x0, PMIC_RG_VGP2_EN_MASK, PMIC_RG_VGP2_EN_SHIFT);		
#else
	upmu_set_rg_vgp2_vosel(0x0);
	upmu_set_rg_vgp2_en(0x0);	
#endif
        MDELAY(20);      
    }

    
}

static void init_lcm_registers(void)
{		

		unsigned int data_array[16];
		MDELAY(100);

data_array[0] = 0x00E01500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x93E11500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x65E21500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0xF8E31500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x03801500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x04E01500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x2B2B1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x032D1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x442E1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x01E01500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x00001500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x6A011500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x00171500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0xC0181500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x01191500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x001A1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0xC01B1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x011C1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x6A1F1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x23201500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x23211500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x0E221500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x28351500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x59371500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x05381500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x04391500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x083A1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x083B1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x7C3C1500;
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

data_array[0] = 0x0B441500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x88451500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x0F551500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x01561500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0xAD571500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x0A581500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x0A591500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x285A1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x1E5B1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x5C5D1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x405E1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x305F1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x24601500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x21611500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x13621500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x1A631500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x07641500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x23651500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x25661500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x27671500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x47681500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x37691500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x3F6A1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x326B1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x2E6C1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x226D1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x116E1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x006F1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x5C701500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x40711500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x30721500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x24731500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x21741500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x13751500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x1A761500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x07771500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x23781500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x25791500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x277A1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x477B1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x377C1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x3F7D1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x327E1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x2E7F1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x22801500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x11811500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x00821500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x02E01500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x44001500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x44011500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x45021500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x45031500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x46041500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x46051500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x47061500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x47071500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x1D081500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x1D091500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x1D0A1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x1D0B1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x1D0C1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x1D0D1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x1D0E1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x570F1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x57101500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x58111500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x58121500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x40131500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x55141500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x55151500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x44161500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x44171500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x45181500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x45191500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x461A1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x461B1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x471C1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x471D1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x1D1E1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x1D1F1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x1D201500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x1D211500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x1D221500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x1D231500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x1D241500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x57251500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x57261500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x58271500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x58281500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x40291500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x552A1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x552B1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x40581500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x00591500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x005A1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x005B1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x0A5C1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x105D1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x015E1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x025F1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x10601500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x01611500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x02621500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x0B631500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x54641500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x45651500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x07661500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x31671500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x0B681500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x1E691500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x546A1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x046B1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x006C1500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x046D1500;
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

data_array[0] = 0xF8751500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x00761500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x0D771500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x14781500;
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

data_array[0] = 0x00E01500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x10091500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x02E61500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x06E71500;
dsi_set_cmdq(data_array, 1, 1);

data_array[0] = 0x00110500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(120);

data_array[0] = 0x00290500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(20);

data_array[0] = 0x00351500;
dsi_set_cmdq(data_array, 1, 1);

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
	params->dsi.mode    = BURST_VDO_MODE;

	// DSI
	/* Command mode setting */
	params->dsi.LANE_NUM				= LCM_FOUR_LANE;
	params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;
	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

	params->dsi.vertical_sync_active = 4; 
	params->dsi.vertical_backporch =  4; 
	params->dsi.vertical_frontporch = 8; 
	params->dsi.vertical_active_line = FRAME_HEIGHT;

	params->dsi.horizontal_sync_active = 4; 
	params->dsi.horizontal_backporch =	 32; 
	params->dsi.horizontal_frontporch =  24; 
	params->dsi.horizontal_active_pixel = FRAME_WIDTH;

	params->dsi.PLL_CLOCK=210; //
	
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
      
LCM_DRIVER c805_bd_jd9366_wxga_ips_lcm_drv= 
{
	.name			= "c805_bd_jd9366_wxga_ips",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend			=lcm_suspend_jd9364,
	.resume				=lcm_resume_jd9364,	
};

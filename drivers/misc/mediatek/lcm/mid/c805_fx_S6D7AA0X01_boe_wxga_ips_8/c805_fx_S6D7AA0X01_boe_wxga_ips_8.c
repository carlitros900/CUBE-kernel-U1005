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
	    pmic_config_interface(DIGLDO_CON29, 0x3, PMIC_RG_VGP2_VOSEL_MASK, PMIC_RG_VGP2_VOSEL_SHIFT);
		pmic_config_interface(DIGLDO_CON8, 0x1, PMIC_RG_VGP2_EN_MASK, PMIC_RG_VGP2_EN_SHIFT);
		
		lcm_set_gpio_output(GPIO_LCM_VDD, GPIO_OUT_ONE);
        MDELAY(20);

	}
	else
	{	
		pmic_config_interface(DIGLDO_CON29, 0x0, PMIC_RG_VGP2_VOSEL_MASK, PMIC_RG_VGP2_VOSEL_SHIFT);
		pmic_config_interface(DIGLDO_CON8, 0x0, PMIC_RG_VGP2_EN_MASK, PMIC_RG_VGP2_EN_SHIFT);
		
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

data_array[0] = 0x00033902;
data_array[1] = 0x005A5AF0;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);


data_array[0] = 0x00033902;
data_array[1] = 0x005A5AF1;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);


data_array[0] = 0x00033902;
data_array[1] = 0x00A5A5FC;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);


data_array[0] = 0x00033902;
data_array[1] = 0x001000D0;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);


data_array[0] = 0x10B11500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);


data_array[0] = 0x00053902;
data_array[1] = 0x2F2214B2;
data_array[2] = 0x00000004;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);


data_array[0] = 0x00063902;
data_array[1] = 0x080802F2;
data_array[2] = 0x00001040;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);


data_array[0] = 0x04B01500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);


data_array[0] = 0x09FD1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);


data_array[0] = 0x000b3902;
data_array[1] = 0xE2D701F3;
data_array[2] = 0x77F7F462;
data_array[3] = 0x0000263C;
dsi_set_cmdq(data_array, 4, 1);
MDELAY(1);


data_array[0] = 0x002e3902;
data_array[1] = 0x030200F4;
data_array[2] = 0x09020326;
data_array[3] = 0x16160700;
data_array[4] = 0x08080003;
data_array[5] = 0x120F0E03;
data_array[6] = 0x0C1E1D1C;
data_array[7] = 0x02040109;
data_array[8] = 0x72757461;
data_array[9] = 0xB0808083;
data_array[10] = 0x28010100;
data_array[11] = 0x01280304;
data_array[12] = 0x000032D1;
dsi_set_cmdq(data_array, 13, 1);
MDELAY(1);


data_array[0] = 0x001b3902;
data_array[1] = 0x28286CF5;
data_array[2] = 0x5298AB5F;
data_array[3] = 0x0443330F;
data_array[4] = 0x05525459;
data_array[5] = 0x604E6040;
data_array[6] = 0x52262740;
data_array[7] = 0x00186D25;
dsi_set_cmdq(data_array, 8, 1);
MDELAY(1);


data_array[0] = 0x00093902;
data_array[1] = 0x250025EE;
data_array[2] = 0x25002500;
data_array[3] = 0x00000000;
dsi_set_cmdq(data_array, 4, 1);
MDELAY(1);


data_array[0] = 0x00093902;
data_array[1] = 0x981234EF;
data_array[2] = 0x240020BA;
data_array[3] = 0x00000080;
dsi_set_cmdq(data_array, 4, 1);
MDELAY(1);


data_array[0] = 0x00213902;
data_array[1] = 0x0A0E0EF7;
data_array[2] = 0x0B0F0F0A;
data_array[3] = 0x0107050B;
data_array[4] = 0x01010101;
data_array[5] = 0x080C0C01;
data_array[6] = 0x090D0D08;
data_array[7] = 0x01060409;
data_array[8] = 0x01010101;
data_array[9] = 0x00000001;
dsi_set_cmdq(data_array, 10, 1);
MDELAY(1);


data_array[0] = 0x00043902;
data_array[1] = 0x084E01BC;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);


data_array[0] = 0x00063902;
data_array[1] = 0x1C1003E1;
data_array[2] = 0x000010A0;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);


data_array[0] = 0x00073902;
data_array[1] = 0xA62560F6;
data_array[2] = 0x00000000;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);


data_array[0] = 0x00073902;
data_array[1] = 0x030D00FE;
data_array[2] = 0x00080021;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);


data_array[0] = 0x00123902;
data_array[1] = 0x053500FA;
data_array[2] = 0x0B06010B;
data_array[3] = 0x19160D0A;
data_array[4] = 0x19191A19;
data_array[5] = 0x0000221A;
dsi_set_cmdq(data_array, 6, 1);
MDELAY(1);


data_array[0] = 0x00123902;
data_array[1] = 0x053500FB;
data_array[2] = 0x0B06010B;
data_array[3] = 0x19160D0A;
data_array[4] = 0x19191A19;
data_array[5] = 0x0000221A;
dsi_set_cmdq(data_array, 6, 1);
MDELAY(1);


data_array[0] = 0x00043902;
data_array[1] = 0x280040C3;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);


data_array[0] = 0x00350500;
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
	params->dsi.mode    = BURST_VDO_MODE;

	params->dsi.LANE_NUM				= LCM_FOUR_LANE;//LCM_THREE_LANE;//LCM_FOUR_LANE;

	params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

	params->dsi.vertical_sync_active 				= 4; 
    params->dsi.vertical_backporch					= 4;
    params->dsi.vertical_frontporch 				= 8;
    params->dsi.vertical_active_line				= FRAME_HEIGHT; 

    params->dsi.horizontal_sync_active				= 16;
    params->dsi.horizontal_backporch				= 48;
    params->dsi.horizontal_frontporch				= 16;
	params->dsi.horizontal_active_pixel = FRAME_WIDTH;

	params->dsi.PLL_CLOCK = 200;//256;	// 256  251

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


       
LCM_DRIVER c805_fx_S6D7AA0X01_boe_wxga_ips_8_lcm_drv= 
{
	.name			= "c805_fx_S6D7AA0X01_boe_wxga_ips_8",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend			=lcm_suspend_jd9364,
	.resume				=lcm_resume_jd9364,
};

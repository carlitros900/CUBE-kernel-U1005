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

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))

#define REGFLAG_DELAY             							0XFE
#define REGFLAG_END_OF_TABLE      							0xFF   // END OF REGISTERS MARKER
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
data_array[1] = 0x005a5af0;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x005a5af1;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x00a5a5fc;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x001000d0;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x10b11500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00053902;
data_array[1] = 0x2f2214b2;
data_array[2] = 0x00000004;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);

data_array[0] = 0x01b51500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00093902;
data_array[1] = 0x1f0000ee;
data_array[2] = 0x1f000000;
data_array[3] = 0x00000000;
dsi_set_cmdq(data_array, 4, 1);
MDELAY(1);

data_array[0] = 0x000e3902;
data_array[1] = 0x433456ef;
data_array[2] = 0x24809065;
data_array[3] = 0x11910080;
data_array[4] = 0x00001111;
dsi_set_cmdq(data_array, 5, 1);
MDELAY(1);

data_array[0] = 0x00213902;
data_array[1] = 0x090804f7;
data_array[2] = 0x0D0C0B0A;
data_array[3] = 0x17160F0E;
data_array[4] = 0x01010110;
data_array[5] = 0x09080401;
data_array[6] = 0x0D0C0B0A;
data_array[7] = 0x17160F0E;
data_array[8] = 0x01010110;
data_array[9] = 0x00000001;
dsi_set_cmdq(data_array, 10, 1);
MDELAY(1);

data_array[0] = 0x00063902;
data_array[1] = 0x080802f2;
data_array[2] = 0x00001040;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);

data_array[0] = 0x00073902;
data_array[1] = 0x262560f6;
data_array[2] = 0x00000000;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);

data_array[0] = 0x00123902;
data_array[1] = 0x073504fa;
data_array[2] = 0x100B120B;
data_array[3] = 0x2C241A16;
data_array[4] = 0x333B3B33;
data_array[5] = 0x00003334;
dsi_set_cmdq(data_array, 6, 1);
MDELAY(1);

data_array[0] = 0x00123902;
data_array[1] = 0x073504fb;
data_array[2] = 0x100B120B;
data_array[3] = 0x2C241A16;
data_array[4] = 0x333B3B33;
data_array[5] = 0x00003334;
dsi_set_cmdq(data_array, 6, 1);
MDELAY(1);

data_array[0] = 0x000b3902;
data_array[1] = 0xE0C401f3;
data_array[2] = 0x3783D462;
data_array[3] = 0x0000243C;
dsi_set_cmdq(data_array, 4, 1);
MDELAY(1);

data_array[0] = 0x00243902;
data_array[1] = 0x030200f4;
data_array[2] = 0x09020326;
data_array[3] = 0x16160700;
data_array[4] = 0x08080003;
data_array[5] = 0x121C1903;
data_array[6] = 0x1A1E1D1C;
data_array[7] = 0x02040109;
data_array[8] = 0x72757461;
data_array[9] = 0xF0808083;
dsi_set_cmdq(data_array, 10, 1);
MDELAY(1);

data_array[0] = 0x01b01500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00143902;
data_array[1] = 0x5F2F2Ff5;
data_array[2] = 0x0F5298AB;
data_array[3] = 0x59044333;
data_array[4] = 0x40055254;
data_array[5] = 0x40595D40;
dsi_set_cmdq(data_array, 6, 1);
MDELAY(1);

data_array[0] = 0x00043902;
data_array[1] = 0x084E01bc;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00063902;
data_array[1] = 0x1C1003e1;
data_array[2] = 0x000010A0;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);

data_array[0] = 0x00063902;
data_array[1] = 0x111016fd;
data_array[2] = 0x00000920;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);

data_array[0] = 0x00351500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00043902;
data_array[1] = 0x280040c3;
dsi_set_cmdq(data_array, 2, 1);
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
	//params->dsi.mode	 = SYNC_EVENT_VDO_MODE; 

	// DSI
	/* Command mode setting */
	params->dsi.LANE_NUM				= LCM_FOUR_LANE;//LCM_THREE_LANE;//LCM_FOUR_LANE;

	params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;



	params->dsi.vertical_sync_active 				= 5; 
    params->dsi.vertical_backporch					= 3;
    params->dsi.vertical_frontporch 				= 8;
    params->dsi.vertical_active_line				= FRAME_HEIGHT; 

    params->dsi.horizontal_sync_active				= 5;
    params->dsi.horizontal_backporch				= 59;
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

static void lcm_suspend_power(void)
{
    MDELAY(10);
    lcd_reset(0);
    MDELAY(10);
	lcd_vdd(0);
    lcd_power_en(0);
    MDELAY(10);
}

static void lcm_resume_power(void)
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
	MDELAY(20);
}

static void lcm_init_power(void){
  printk("=====lxl(lcm_init_power)=====\n");
}
       
LCM_DRIVER c805_hx_s6d7aa0x04_cpt_wxga_ips_8_lcm_drv= 
{
	.name			= "c805_hx_s6d7aa0x04_cpt_wxga_ips_8",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend			=lcm_suspend_jd9364,
	.resume				=lcm_resume_jd9364,
	.suspend_power 	= lcm_suspend_power,
	.resume_power 		= lcm_resume_power,
	.init_power 		= lcm_init_power,
	
};

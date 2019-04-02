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

struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};

static void lcm_initial_registers(void)
{
unsigned int data_array[16];

//===== JD9366 IVO10P1 JD5002 initial setting ======//                                                

//----------- JD EVB RGB setting -----------------//                                                

MDELAY(1);
//-------------------------------------------------------------//                                   

//JD9366 initial code                                                                                                                                                                   

//Page0                                                                                             

	data_array[0]=0x00043902;
	data_array[1]=0x9483FFB9; 
	dsi_set_cmdq(data_array, 2, 1); 
	MDELAY(1);

	data_array[0]=0x00063902;
	data_array[1]=0x1E2E1FC9; 
	data_array[2]=0x0000101E;
	dsi_set_cmdq(data_array, 3, 1); 
	MDELAY(1);

	data_array[0]=0x00103902;
	data_array[1]=0x301064b1; 
	data_array[2]=0xF1113443; 
	data_array[3]=0x34D97081; 
	data_array[4]=0x41D2C080; 
	dsi_set_cmdq(data_array, 5, 1); 
	MDELAY(1); 

	data_array[0]=0x000D3902;
	data_array[1]=0x0F6445b2; 
	data_array[2]=0x081C4009; 
	data_array[3]=0x004D1C08; 
	data_array[4]=0x00000000; 
	dsi_set_cmdq(data_array, 5, 1); 
	MDELAY(1); 

	data_array[0]=0x000D3902;
	data_array[1]=0x6fff00b4; 
	data_array[2]=0x6f606f60; 
	data_array[3]=0x0f6e0160; 
	data_array[4]=0x0000006e; 
	dsi_set_cmdq(data_array, 5, 1); 
	MDELAY(1); 

	data_array[0]=0x00033902;
	data_array[1]=0x006a6ab6;
	//data_array[1]=data_1;
	dsi_set_cmdq(data_array, 2, 1); 
	MDELAY(1);
	//data_1+=0x00020200;

	data_array[0]=0x00023902;
	data_array[1]=0x000009cc; 
	dsi_set_cmdq(data_array, 2, 1); 
	MDELAY(1);

	data_array[0]=0x00213902;
	data_array[1]=0x000800D3; 
	data_array[2]=0x08000701; 
	data_array[3]=0x000a1032; 
	data_array[4]=0x0a200005; 
	data_array[5]=0x32000905; 
	data_array[6]=0x35000810; 
	data_array[7]=0x47070d33;
	data_array[8]=0x0f47070d; 
	data_array[9]=0x00000008; 
	dsi_set_cmdq(data_array, 10, 1);
	MDELAY(1);

	data_array[0]=0x002D3902;
	data_array[1]=0x030203d5; 
	data_array[2]=0x01000102; 
	data_array[3]=0x07060700; 
	data_array[4]=0x05040506;
	data_array[5]=0x18202104;
	data_array[6]=0x18181818; 
	data_array[7]=0x18181818; 
	data_array[8]=0x23181818; 
	data_array[9]=0x18181822; 	
	data_array[10]=0x18181818; 
	data_array[11]=0x18181818; 
	data_array[12]=0x00000018; 
	dsi_set_cmdq(data_array, 13, 1);
	MDELAY(1);

	data_array[0]=0x002b3902;
	data_array[1]=0x1c1703e0; 
	data_array[2]=0x273b302d; 
	data_array[3]=0x0d0b0840; 
	data_array[4]=0x15120f18; 
	data_array[5]=0x12071413; 
	data_array[6]=0x17031714;
	data_array[7]=0x3b302d1c;	
	data_array[8]=0x0b084027; 
	data_array[9]=0x120f180d; 
	data_array[10]=0x07141315; 
	data_array[11]=0x00171412; 
	dsi_set_cmdq(data_array, 12, 1);
	MDELAY(1); 

	
	data_array[0] = 0x00110500; 
	dsi_set_cmdq(data_array, 1, 1); 
	MDELAY(200);

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

	// DSI
	/* Command mode setting */
	params->dsi.LANE_NUM				= LCM_FOUR_LANE;
	params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;
	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

	params->dsi.vertical_sync_active				= 4;
	params->dsi.vertical_backporch					= 11;
	params->dsi.vertical_frontporch					= 38;
	params->dsi.vertical_active_line				= FRAME_HEIGHT; 

	params->dsi.horizontal_sync_active				= 45;
	params->dsi.horizontal_backporch				= 45;
	params->dsi.horizontal_frontporch				= 72;
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

	params->dsi.PLL_CLOCK=200; 
	
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
	lcm_initial_registers();
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
  printk("=====wjh(lcm_init_power)=====\n");
}
       
LCM_DRIVER c805_xy_jd9366ab_boe_wxga_ips_8_lcm_drv= 
{
	.name			= "c805_xy_jd9366ab_boe_wxga_ips_8",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend			=lcm_suspend_jd9364,
	.resume				=lcm_resume_jd9364,
	.suspend_power 	= lcm_suspend_power,
	.resume_power 		= lcm_resume_power,
	.init_power 		= lcm_init_power,
	
};

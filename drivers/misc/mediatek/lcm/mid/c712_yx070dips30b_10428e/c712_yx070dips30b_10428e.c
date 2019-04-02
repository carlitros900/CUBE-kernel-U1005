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
extern unsigned int GPIO_LCM_RST;
extern unsigned int GPIO_LCM_VDD;
#define GPIO_LCM_EN GPIO_LCM_VDD




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

#ifdef BUILD_LK
		printf("%s(%d), LK \n", __func__, enabled?1:0);
#else
		printk("%s(%d), kernel", __func__, enabled?1:0);
#endif

	if (enabled)
	{

		lcm_set_gpio_output(GPIO_LCM_EN, GPIO_OUT_ONE);
		MDELAY(10);
		lcm_set_gpio_output(GPIO_LCM_PWR, GPIO_OUT_ONE);

/*
		lcm_set_gpio_output(GPIO_LCM_PWR, GPIO_OUT_ONE);
		MDELAY(10);
		lcm_set_gpio_output(GPIO_LCM_EN, GPIO_OUT_ONE);
*/
	}
	else
	{
		
		lcm_set_gpio_output(GPIO_LCM_EN, 0);
		MDELAY(10);
		lcm_set_gpio_output(GPIO_LCM_PWR, 0);

/*
		lcm_set_gpio_output(GPIO_LCM_PWR, 0);
		MDELAY(10);
		lcm_set_gpio_output(GPIO_LCM_EN, 0);
*/
	}

	MDELAY(50); 
}
/*
static void lcd_reset(unsigned char enabled)
{
#ifdef BUILD_LK
        printf("%s(%d), LK \n", __func__, enabled?1:0);
#else
        printk("%s(%d), kernel", __func__, enabled?1:0);
#endif

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
*/

static void init_lcm_registers(void)
{       

//    unsigned int data_array[16];
/*
data_array[0] = 0x00033902;
data_array[1] = 0x005A5AF0;
dsi_set_cmdq(data_array, 2, 1);
*/
/*
data_array[0] = 0x00110500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(120);

data_array[0] = 0x00290500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(20);
*/
/*
data_array[0] = 0x00043902;
data_array[1] = 0x280040C3;
dsi_set_cmdq(data_array, 2, 1);
*/
}

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}

static void lcm_get_params(LCM_PARAMS *params)
{
    printk("=====lxl(lcm_get_params)=====\n");
    memset(params, 0, sizeof(LCM_PARAMS));

    params->type   = LCM_TYPE_DSI;

    params->width  = FRAME_WIDTH;
    params->height = FRAME_HEIGHT;

    params->dsi.mode    = BURST_VDO_MODE;
    params->dsi.LANE_NUM                = LCM_FOUR_LANE;

    params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

    params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
    //params->dsi.word_count=800*3; 


    params->dsi.vertical_sync_active = 10;
    params->dsi.vertical_backporch =  3;
    params->dsi.vertical_frontporch = 5;
    params->dsi.vertical_active_line = FRAME_HEIGHT;

    params->dsi.horizontal_sync_active = 20;
    params->dsi.horizontal_backporch =  48; 
    params->dsi.horizontal_frontporch = 16;
    params->dsi.horizontal_active_pixel = FRAME_WIDTH;

    params->dsi.PLL_CLOCK=204; 

}


static void lcm_init(void)
{
   
 	//lcd_reset(0);
	lcd_power_en(0);
	lcd_power_en(1);
	MDELAY(20);
	//lcd_reset(1);
	//MDELAY(10);
	//lcd_reset(0);
	//MDELAY(15);
	//lcd_reset(1);
	init_lcm_registers();
	MDELAY(100);
}

static void lcm_suspend_jd9364(void)
{
    //lcd_reset(0);
    //MDELAY(10);
    lcd_power_en(0);
    MDELAY(20);
	printk("*****lxl(lcm_suspend)**********\n");
}

static void lcm_resume_jd9364(void)
{
	lcm_init();
}

static void lcm_suspend_power(void)
{
    //MDELAY(10);
	//lcd_reset(0);
    //MDELAY(10);
    lcd_power_en(0);
    MDELAY(20);
}

static void lcm_resume_power(void)
{
	  
    //lcd_reset(0);
    lcd_power_en(0);
    lcd_power_en(1);
    MDELAY(20);
    //lcd_reset(1);
	//MDELAY(10);
    //lcd_reset(0);
    //MDELAY(10);
    //lcd_reset(1);
	//MDELAY(20);
}

static void lcm_init_power(void){
  printk("=====lxl(lcm_init_power)=====\n");
}
       
LCM_DRIVER c712_yx070dips30b_10428e_lcm_drv= 
{
	.name			= "c712_yx070dips30b_10428e",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend			=lcm_suspend_jd9364,
	.resume				=lcm_resume_jd9364,
	.suspend_power 	= lcm_suspend_power,
	.resume_power 		= lcm_resume_power,
	.init_power 		= lcm_init_power,
	
};

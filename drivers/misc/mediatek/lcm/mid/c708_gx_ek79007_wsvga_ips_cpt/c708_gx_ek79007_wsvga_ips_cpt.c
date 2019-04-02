#ifdef BUILD_LK
	#include <platform/mt_gpio.h>
	#include <platform/mt_pmic.h>
#else
	#include <mt-plat/mt_gpio.h>
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

// ---------------------------------------------------------------------------
//  IOdefine
// ---------------------------------------------------------------------------
#ifdef BUILD_LK
#define GPIO_LCM_PWR_EN     GPIO83	//LCM内部升压控制
#define LCM_PWREN     		GPIO21	//LCM工作电压控制（C805公版默认使用VIO18_PMU）
#define LCM_RESET     		GPIO19	//LCM复位控制
#else
extern unsigned int 		GPIO_LCM_VDD; //GPIO83
extern unsigned int 		GPIO_LCM_PWR; //GPIO21 此三个外部声明都在lcm_common_get_power.c文件中定义
extern unsigned int			GPIO_LCM_RST; //GPIO19
#define GPIO_LCM_PWR_EN     GPIO_LCM_VDD	//LCM内部升压控制
#define LCM_PWREN     		GPIO_LCM_PWR	//LCM工作电压控制（C805公版默认使用VIO18_PMU）
#define LCM_RESET     		GPIO_LCM_RST	//LCM复位控制
#endif
// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------
#define FRAME_WIDTH  										(1024)
#define FRAME_HEIGHT 										(600)

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------
static LCM_UTIL_FUNCS lcm_util = {0};
#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))
#define UDELAY(n) 											(lcm_util.udelay(n))
#define MDELAY(n) 											(lcm_util.mdelay(n))

// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)										lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)   			lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)    

// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------
//static unsigned int is_lcm_inited=0;

static void init_lcm_registers(void)
{		

    unsigned int data_array[16];

	data_array[0] = 0x10b21500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);

	//=================control display effction======================
	data_array[0] = 0x58801500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);

	data_array[0] = 0x47811500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);

	data_array[0] = 0xD4821500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);

	data_array[0] = 0x88831500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);

	data_array[0] = 0xA9841500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);

	data_array[0] = 0xC3851500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);

	data_array[0] = 0x82861500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);
//====================================================
	data_array[0] = 0x00291500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(20);
	data_array[0] = 0x00111500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(50);


}

static void lcm_set_gpio_output(unsigned int GPIO, unsigned int output)
{
printk(" ##########  %s \r\n",__func__);
#ifdef BUILD_LK
   mt_set_gpio_mode(GPIO, GPIO_MODE_00);
   mt_set_gpio_dir(GPIO, GPIO_DIR_OUT);
   mt_set_gpio_out(GPIO, (output>0)? GPIO_OUT_ONE: GPIO_OUT_ZERO);
#else
	gpio_direction_output(GPIO, output);
	gpio_set_value(GPIO, output);
#endif
}
static void lcd_power_en(unsigned char enabled)
{
	if (enabled)
	{
		lcm_set_gpio_output(LCM_PWREN, 1);
		// lcm_set_gpio_output(GPIO_LCM_PWR_EN, 1);
	}
	else
	{
		lcm_set_gpio_output(LCM_PWREN, 0);
		// lcm_set_gpio_output(GPIO_LCM_PWR_EN, 0);
	}
}
static void lcd_bl_en(unsigned char enabled)
{
	if (enabled)
	{
		lcm_set_gpio_output(GPIO_LCM_PWR_EN, 1);
	}
	else
	{
		lcm_set_gpio_output(GPIO_LCM_PWR_EN, 0);
	}
}

static void lcd_reset(unsigned char enabled)
{
    if(enabled)
    {
		lcm_set_gpio_output(LCM_RESET, 1);
    }
    else
    {
		lcm_set_gpio_output(LCM_RESET, 0);
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
printk(" ##########  %s \r\n",__func__);
    memset(params, 0, sizeof(LCM_PARAMS));
    params->type   = LCM_TYPE_DSI;
    params->width  = FRAME_WIDTH;
    params->height = FRAME_HEIGHT;

    params->dbi.te_mode                         = LCM_DBI_TE_MODE_VSYNC_ONLY;
    params->dbi.te_edge_polarity                = LCM_POLARITY_RISING;


    params->dsi.mode   = SYNC_EVENT_VDO_MODE;	//BURST_VDO_MODE;
    params->dsi.LANE_NUM                                = LCM_TWO_LANE;
    params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB565;	//LCM_DSI_FORMAT_RGB888;
    params->dsi.PS=LCM_PACKED_PS_16BIT_RGB565;	//LCM_PACKED_PS_24BIT_RGB888;
    params->dsi.vertical_sync_active                            = 2; //4;
    params->dsi.vertical_backporch                              = 10; //16;
    params->dsi.vertical_frontporch                             = 5; 
    params->dsi.vertical_active_line                            = FRAME_HEIGHT; 

    params->dsi.horizontal_sync_active                          = 10; //5;//6;
    params->dsi.horizontal_backporch                            = 60; //60; //80;
    params->dsi.horizontal_frontporch                           = 60; 
    //params->dsi.horizontal_blanking_pixel                       = 60; 
    params->dsi.horizontal_active_pixel                         = FRAME_WIDTH;

	params->dsi.PLL_CLOCK=230;
}

static void lcm_init(void)
{
printk(" ##########  %s \r\n",__func__);
	lcd_power_en(1);
	lcd_bl_en(1);
	MDELAY(100);
	lcd_reset(1);
	MDELAY(10);
	lcd_reset(0);
	MDELAY(10);
	lcd_reset(1);
    MDELAY(150);

	init_lcm_registers();
}

static void lcm_suspend(void)
{    
printk(" ##########  %s \r\n",__func__);
	lcd_bl_en(0);
	MDELAY(10);
	lcd_reset(0);
	MDELAY(10);
	lcd_power_en(0);
	MDELAY(10);
}
static void lcm_resume(void)
{
printk(" ##########  %s \r\n",__func__);
    lcm_init();
}

LCM_DRIVER c708_gx_ek79007_wsvga_ips_cpt_lcm_drv={
	.name				="c708_gx_ek79007_wsvga_ips_cpt",
	.set_util_funcs	=lcm_set_util_funcs,
	.get_params			=lcm_get_params,
	.init				=lcm_init,
	.suspend			=lcm_suspend,
	.resume				=lcm_resume,
};

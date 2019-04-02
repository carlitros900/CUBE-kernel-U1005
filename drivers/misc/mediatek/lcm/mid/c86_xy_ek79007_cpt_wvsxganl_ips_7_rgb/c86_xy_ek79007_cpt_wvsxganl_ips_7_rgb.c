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
extern unsigned int GPIO_LCM_33V;


// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  (1024)	//(800)
#define FRAME_HEIGHT (600)	//(1280)


#define HSYNC_PULSE_WIDTH 10 
#define HSYNC_BACK_PORCH  60
#define HSYNC_FRONT_PORCH 60
#define VSYNC_PULSE_WIDTH 2
#define VSYNC_BACK_PORCH  5
#define VSYNC_FRONT_PORCH 10

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))



// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

static void lcm_set_gpio_output(unsigned int GPIO, unsigned int output)
{
        gpio_direction_output(GPIO, output);
        gpio_set_value(GPIO, output);
}
static void lcm_init_power(void)
{

	lcm_set_gpio_output(GPIO_LCM_33V, GPIO_OUT_ONE);
	MDELAY(20);
	lcm_set_gpio_output(GPIO_LCM_PWR, GPIO_OUT_ONE);
	MDELAY(20);

}

static void lcm_suspend_power(void)
{
	lcm_set_gpio_output(GPIO_LCM_33V, GPIO_OUT_ZERO);
	MDELAY(20);
	lcm_set_gpio_output(GPIO_LCM_PWR, GPIO_OUT_ZERO);
	MDELAY(20);

}

static void lcm_resume_power(void)
{
	lcm_init_power();
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

    params->type   = LCM_TYPE_DPI;
    params->width  = FRAME_WIDTH;
    params->height = FRAME_HEIGHT;
	params->ctrl   = LCM_CTRL_NONE;
    params->io_select_mode = 0;
    params->dpi.PLL_CLOCK = 52;  //67MHz


    /* RGB interface configurations */
    params->dpi.mipi_pll_clk_ref  = 0;
    params->dpi.mipi_pll_clk_div1 = 0x80000101;  //lvds pll 65M
    params->dpi.mipi_pll_clk_div2 = 0x800a0000;
    //params->dpi.dpi_clk_div       = 2;          
    //params->dpi.dpi_clk_duty      = 1;
	params->dpi.width = FRAME_WIDTH;
	params->dpi.height = FRAME_HEIGHT;

    params->dpi.clk_pol           = LCM_POLARITY_RISING;
    params->dpi.de_pol            = LCM_POLARITY_RISING;
    params->dpi.vsync_pol         = LCM_POLARITY_RISING;
    params->dpi.hsync_pol         = LCM_POLARITY_RISING;

    params->dpi.hsync_pulse_width = HSYNC_PULSE_WIDTH;
    params->dpi.hsync_back_porch  = HSYNC_BACK_PORCH;
    params->dpi.hsync_front_porch = HSYNC_FRONT_PORCH;
    params->dpi.vsync_pulse_width = VSYNC_PULSE_WIDTH;
    params->dpi.vsync_back_porch  = VSYNC_BACK_PORCH;
    params->dpi.vsync_front_porch = VSYNC_FRONT_PORCH;



    params->dpi.lvds_tx_en = 0;
    params->dpi.ssc_disable = 1;
    params->dpi.format            = LCM_DPI_FORMAT_RGB888;   // format is 24 bit
    params->dpi.rgb_order         = LCM_COLOR_ORDER_RGB;
    params->dpi.is_serial_output  = 0;

    params->dpi.intermediat_buffer_num = 0;

    params->dpi.io_driving_current = LCM_DRIVING_CURRENT_2MA;


}

static void lcm_init(void)
{	
	SET_RESET_PIN(1);
	MDELAY(20);

	SET_RESET_PIN(0);
	MDELAY(20);

	SET_RESET_PIN(1);
	MDELAY(20);
}

static void lcm_suspend(void)
{
	SET_RESET_PIN(1);
	MDELAY(10);
	
	SET_RESET_PIN(0);
	MDELAY(10);

}

static void lcm_resume(void)
{
	lcm_init();
}

LCM_DRIVER c86_xy_ek79007_cpt_wvsxganl_ips_7_rgb_lcm_drv = 
{
    .name		= "c86_xy_ek79007_cpt_wvsxganl_ips_7_rgb",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.init_power		= lcm_init_power,
	.resume_power 	= lcm_resume_power,
	.suspend_power 	= lcm_suspend_power,
};


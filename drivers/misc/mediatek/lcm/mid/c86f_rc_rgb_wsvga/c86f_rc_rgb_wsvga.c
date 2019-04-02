/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
#ifndef BUILD_LK
#include <linux/string.h>
#else
#include <string.h>
#endif 
	
#ifdef BUILD_LK
#include <platform/mt_gpio.h>
#include <platform/mt_i2c.h>
#include <platform/mt_pmic.h>
#elif (defined BUILD_UBOOT)
#include <asm/arch/mt6577_gpio.h>
#else
//#include <mach/mt_gpio.h>
//#include <mach/mt_pm_ldo.h>
#endif

	
#include <linux/string.h>
#include <linux/wait.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/pinctrl/consumer.h>
#include <linux/of_gpio.h>
#include <asm-generic/gpio.h>
	
#ifdef CONFIG_OF
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/regulator/consumer.h>
#include <linux/clk.h>
#endif
#include "lcm_drv.h"

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  (1024)	//(800)
#define FRAME_HEIGHT (600)	//(1280)

#if 0
#define HSYNC_PULSE_WIDTH 16 
#define HSYNC_BACK_PORCH  16
#define HSYNC_FRONT_PORCH 32
#define VSYNC_PULSE_WIDTH 2
#define VSYNC_BACK_PORCH  2
#define VSYNC_FRONT_PORCH 4
#else
#define HSYNC_PULSE_WIDTH 40 
#define HSYNC_BACK_PORCH  122
#define HSYNC_FRONT_PORCH 85
#define VSYNC_PULSE_WIDTH 5
#define VSYNC_BACK_PORCH  18
#define VSYNC_FRONT_PORCH 12
#endif

extern unsigned int GPIO_LCM_33V;
extern unsigned int GPIO_LCM_PWR;
extern struct regulator *lcm_vgp;

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))



static void lcm_set_gpio_output(unsigned int GPIO, unsigned int output)
{
        gpio_direction_output(GPIO, output);
        gpio_set_value(GPIO, output);
}

static void lcm_init_power(void)
{
#ifdef BUILD_LK 
	//printf("[LK/LCM] lcm_init_power() enter\n");
	//lcm_set_gpio_output(GPIO_LCM_PWR, 1);
	//MDELAY(20);
    //lcm_vgp_supply_enable()
		
#else
	//printk("[Kernel/LCM] lcm_init_power() enter\n");
	//lcm_set_gpio_output(GPIO_LCM_PWR, 1);
	//MDELAY(20);
	
#endif

}

static void lcm_suspend_power(void)
{
#ifdef BUILD_LK 
		
#else
	printk("[Kernel/LCM] lcm_suspend_power() enter\n");
	lcm_set_gpio_output(GPIO_LCM_33V, 0);
	MDELAY(200);
	lcm_set_gpio_output(GPIO_LCM_PWR, 0);	
//	lcm_vgp_supply_disable_r();
	MDELAY(200);

#endif
}

static void lcm_resume_power(void)
{
#ifdef BUILD_LK 
			
#else
	printk("[Kernel/LCM] lcm_resume_power() enter\n");
	lcm_set_gpio_output(GPIO_LCM_33V, 1);
	MDELAY(200);
	lcm_set_gpio_output(GPIO_LCM_PWR, 1);
		
//	lcm_vgp_supply_enable_r();
	MDELAY(200);
			
#endif
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
	params->ctrl   = LCM_CTRL_SERIAL_DBI;
	params->width  = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;
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
    params->dpi.format            = LCM_DPI_FORMAT_RGB666;   // format is 24 bit
    params->dpi.rgb_order         = LCM_COLOR_ORDER_RGB;
    params->dpi.is_serial_output  = 0;

    params->dpi.intermediat_buffer_num = 0;

    params->dpi.io_driving_current = LCM_DRIVING_CURRENT_2MA;


}


static void lcm_init(void)
{
	
#ifdef BUILD_LK 
	printf("[LK/LCM] lcm_init() enter\n");
	
	SET_RESET_PIN(1);
	MDELAY(20);

	SET_RESET_PIN(0);
	MDELAY(20);

	SET_RESET_PIN(1);
	MDELAY(20);
#else
	printk("[Kernel/LCM] lcm_init() enter\n");
#endif
    
}

static void lcm_suspend(void)
{

#ifdef BUILD_LK
	printf("[LK/LCM] lcm_suspend() enter\n");

	SET_RESET_PIN(1);
	MDELAY(10);
	
	SET_RESET_PIN(0);
	MDELAY(10);
#else
	printk("[Kernel/LCM] lcm_suspend() enter\n");
	SET_RESET_PIN(1);
	MDELAY(10);

	SET_RESET_PIN(0);
	MDELAY(10);
#endif
}

static void lcm_resume(void)
{
#ifdef BUILD_LK 
	printf("[LK/LCM] lcm_resume() enter\n");

	SET_RESET_PIN(1);
	MDELAY(20);

	SET_RESET_PIN(0);
	MDELAY(20);

	SET_RESET_PIN(1);
	MDELAY(20);
	
#else
	printk("[Kernel/LCM] lcm_resume() enter\n");
	SET_RESET_PIN(1);
	MDELAY(20);

	SET_RESET_PIN(0);
	MDELAY(20);

	SET_RESET_PIN(1);
	MDELAY(20);
#endif
}



LCM_DRIVER c86f_rc_rgb_wsvga_lcm_drv = 
{
    .name		= "c86f_rc_rgb_wsvga",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.init_power		= lcm_init_power,
	.resume_power 	= lcm_resume_power,
	.suspend_power 	= lcm_suspend_power,
};


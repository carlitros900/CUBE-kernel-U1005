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

static struct LCM_setting_table lcm_initialization_setting[] = {
 //hx080wq01  driver ic9366
    {0xe0,  1,  {0x00}},  //page0
    {0xe1,  1,  {0x93}},
    {0xe2,  1,  {0x65}},
    {0xe3,  1,  {0xf8}},
    {0xe6,  1,  {0x02}},
    {0xe7,  1,  {0x02}},
    {0xe0,  1,  {0x01}},//page1
    {0x00,  1,  {0x00}},
    {0x01,  1,  {0xB7}},
    {0x17,  1,  {0x00}},
    {0x18,  1,  {0xB2}},
    {0x19,  1,  {0x01}},
    {0x1A,  1,  {0x00}},
    {0x1B,  1,  {0xB2}},
    {0x1C,  1,  {0x01}},
    {0x1F,  1,  {0x7E}},
    {0x20,  1,  {0x28}},
    {0x21,  1,  {0x28}},
    {0x22,  1,  {0x0E}},
    {0x24,  1,  {0xC8}},    
    {0x37,  1,  {0x29}},
    {0x38,  1,  {0x05}},
    {0x39,  1,  {0x08}},
    {0x3a,  1,  {0x12}},
    {0x3b,  1,  {0x78}},
    {0x3c,  1,  {0xFF}},
    {0x3e,  1,  {0xFF}},
    {0x3f,  1,  {0xFF}},
    {0x40,  1,  {0x06}},
    {0x41,  1,  {0xA0}},
    {0x43,  1,  {0x08}},    
    {0x44,  1,  {0x07}},
    {0x45,  1,  {0x24}},
    {0x55,  1,  {0x01}},
    {0x56,  1,  {0x01}},
    {0x57,  1,  {0x89}},
    {0x58,  1,  {0x0A}},
    {0x59,  1,  {0x2A}},
    {0x5A,  1,  {0x31}},
    {0x5B,  1,  {0x15}},
    //gammar
    {0x5D,  1,  {0x7C}},
    {0x5E,  1,  {0x64}},
    {0x5F,  1,  {0x54}},
    {0x60,  1,  {0x47}},
    {0x61,  1,  {0x42}},
    {0x62,  1,  {0x32}},
    {0x63,  1,  {0x34}},
    {0x64,  1,  {0x1C}},
    {0x65,  1,  {0x32}},
    {0x66,  1,  {0x2E}},
    {0x67,  1,  {0x2B}},
    {0x68,  1,  {0x46}},
    {0x69,  1,  {0x32}},
    {0x6A,  1,  {0x38}},
    {0x6B,  1,  {0x2A}},
    {0x6C,  1,  {0x28}},
    {0x6D,  1,  {0x1B}},
    {0x6E,  1,  {0x0D}},
    {0x6F,  1,  {0x00}},
    {0x70,  1,  {0x7C}},
    {0x71,  1,  {0x64}},
    {0x72,  1,  {0x54}},
    {0x73,  1,  {0x47}},
    {0x74,  1,  {0x42}},
    {0x75,  1,  {0x32}},
    {0x76,  1,  {0x34}},
    {0x77,  1,  {0x1C}},
    {0x78,  1,  {0x32}},
    {0x79,  1,  {0x2E}},
    {0x7A,  1,  {0x2B}},
    {0x7B,  1,  {0x46}},
    {0x7C,  1,  {0x32}},
    {0x7D,  1,  {0x38}},
    {0x7E,  1,  {0x2A}},
    {0x7F,  1,  {0x28}},
    {0x80,  1,  {0x1B}},
    {0x81,  1,  {0x0D}},
    {0x82,  1,  {0x00}},

    {0xE0,  1,  {0x02}},
    {0x00,  1,  {0x00}},
    {0x01,  1,  {0x04}},   
    {0x02,  1,  {0x08}}, 
    {0x03,  1,  {0x05}}, 
    {0x04,  1,  {0x09}}, 
    {0x05,  1,  {0x06}}, 
    {0x06,  1,  {0x0A}}, 
    {0x07,  1,  {0x07}}, 
    {0x08,  1,  {0x0B}}, 
    {0x09,  1,  {0x1f}}, 
    {0x0A,  1,  {0x1f}}, 
    {0x0B,  1,  {0x1f}}, 
    {0x0C,  1,  {0x1f}}, 
    {0x0D,  1,  {0x1f}}, 
    {0x0E,  1,  {0x1f}}, 
    {0x0F,  1,  {0x17}}, 
    {0x10,  1,  {0x37}}, 
    {0x11,  1,  {0x10}}, 
    {0x12,  1,  {0x1F}}, 
    {0x13,  1,  {0x1F}}, 
    {0x14,  1,  {0x1F}}, 
    {0x15,  1,  {0x1F}}, 
    {0x16,  1,  {0x00}},
    {0x17,  1,  {0x04}},   
    {0x18,  1,  {0x08}}, 
    {0x19,  1,  {0x05}}, 
    {0x1A,  1,  {0x09}}, 
    {0x1B,  1,  {0x06}}, 
    {0x1C,  1,  {0x0A}}, 
    {0x1D,  1,  {0x07}}, 
    {0x1E,  1,  {0x0B}}, 
    {0x1F,  1,  {0x1f}}, 
    {0x20,  1,  {0x1f}}, 
    {0x21,  1,  {0x1f}}, 
    {0x22,  1,  {0x1f}}, 
    {0x23,  1,  {0x1f}}, 
    {0x24,  1,  {0x1f}}, 
    {0x25,  1,  {0x17}}, 
    {0x26,  1,  {0x37}}, 
    {0x27,  1,  {0x10}}, 
    {0x28,  1,  {0x1F}}, 
    {0x29,  1,  {0x1F}}, 
    {0x2A,  1,  {0x1F}}, 
    {0x2B,  1,  {0x1F}}, 
    {0x58,  1,  {0x01}}, 
    {0x59,  1,  {0x00}}, 
    {0x5A,  1,  {0x00}}, 
    {0x5B,  1,  {0x00}}, 
    {0x5C,  1,  {0x01}}, 
    {0x5D,  1,  {0x30}}, 
    {0x5E,  1,  {0x00}}, 
    {0x5F,  1,  {0x00}}, 
    {0x60,  1,  {0x30}}, 
    {0x61,  1,  {0x00}}, 
    {0x62,  1,  {0x00}}, 
    {0x63,  1,  {0x03}}, 
    {0x64,  1,  {0x6A}}, 
    {0x65,  1,  {0x45}}, 
    {0x66,  1,  {0x08}}, 
    {0x67,  1,  {0x73}}, 
    {0x68,  1,  {0x05}}, 
    {0x69,  1,  {0x06}}, 
    {0x6A,  1,  {0x6A}},
    {0x6B,  1,  {0x08}}, 
    {0x6C,  1,  {0x00}}, 
    {0x6D,  1,  {0x00}}, 
    {0x6E,  1,  {0x00}}, 
    {0x6F,  1,  {0x88}}, 
    {0x70,  1,  {0x00}}, 
    {0x71,  1,  {0x00}}, 
    {0x72,  1,  {0x06}}, 
    {0x73,  1,  {0x7B}}, 
    {0x74,  1,  {0x00}}, 
    {0x75,  1,  {0x80}}, 
    {0x76,  1,  {0x00}}, 
    {0x77,  1,  {0x05}}, 
    {0x78,  1,  {0x10}}, 
    {0x79,  1,  {0x00}}, 
    {0x7A,  1,  {0x00}}, 
    {0x7B,  1,  {0x00}}, 
    {0x7C,  1,  {0x00}}, 
    {0x7D,  1,  {0x03}}, 
    {0x7E,  1,  {0x7B}},
    
    {0xE0,  1,  {0x01}},
    {0x0E,  1,  {0x01}},

    {0xE0,  1,  {0x03}},//page3
    {0x98,  1,  {0x2F}},
    {0xE0,  1,  {0x04}},//page4
    {0x2B,  1,  {0x2B}},
    {0x2d,  1,  {0x03}},
    {0x2E,  1,  {0x44}},

    {0xE0,  1,  {0x00}},//page0
    {0xE1,  1,  {0x93}},
    {0xE2,  1,  {0x65}},
    {0xE3,  1,  {0xf8}},
    {0x80,  1,  {0x02}}, //3 lane
    //{0x4a,    1,  {0x35}},    
    {0x11,  0,  {0x00}},
    {REGFLAG_DELAY,100,{}},
    {0x29,  0,  {0x00}},
    {REGFLAG_DELAY,200,{}},
    {0x35,  0,  {0x00}},
//3 lane
/*{0xFF,4,{0xAA,0x55,0x25,0x01}},
{0x6F,1,{0x16,}},
{0xF7,1,{0x10,}},
{0xFF,4,{0xAA,0x55,0x25,0x00}},*/

//bist
/*{0xF0,5,{0x55,0xAA,0x52,0x08,0x00}},
{0xEF,2,{0x00,0x01}},
{0xEE,4,{0x87,0x78,0x02,0x40}},*/

//{0x35,0,{0x00}},          //TE on

//{0xF0,5,{0x55,0xAA,0x52,0x08,0x01}},

{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
        unsigned int i;

    for(i = 0; i < count; i++)
    {
        unsigned cmd;
        cmd = table[i].cmd;

        switch (cmd) {
            case REGFLAG_DELAY :
                if(table[i].count <= 10)
                    MDELAY(table[i].count);
                else
                    MDELAY(table[i].count);
                break;

            case REGFLAG_END_OF_TABLE :
                break;

            default:
                                dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
        }
    }

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
	params->dsi.LANE_NUM				= LCM_THREE_LANE;
	params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;
	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

	params->dsi.vertical_sync_active = 4;
	params->dsi.vertical_backporch =  4;
	params->dsi.vertical_frontporch = 8;
	params->dsi.vertical_active_line = FRAME_HEIGHT;

	params->dsi.horizontal_sync_active = 20;
	params->dsi.horizontal_backporch =	20; 
	params->dsi.horizontal_frontporch = 20;
	params->dsi.horizontal_active_pixel = FRAME_WIDTH;

	params->dsi.PLL_CLOCK=260; 
	
    params->dsi.noncont_clock = 1;
    params->dsi.noncont_clock_period= 2;

}


static void lcm_init(void)
{
  printk("=====lxl(lcm_init)=====\n");
   /*
    lcd_reset(0);
    lcd_power_en(0);
    lcd_power_en(1);
    MDELAY(20);//Must > 5ms
    lcd_reset(1);
    MDELAY(10);//Must > 5ms
    lcd_reset(0);
    MDELAY(10);//Must > 5ms
    lcd_reset(1);
    MDELAY(10);*/
	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
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
       
LCM_DRIVER c805_jd9366ab_hx00821_lcm_drv= 
{
	.name			= "c805_jd9366ab_hx00821",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend			=lcm_suspend_jd9364,
	.resume				=lcm_resume_jd9364,
	.suspend_power 	= lcm_suspend_power,
	.resume_power 		= lcm_resume_power,
	.init_power 		= lcm_init_power,
	
};

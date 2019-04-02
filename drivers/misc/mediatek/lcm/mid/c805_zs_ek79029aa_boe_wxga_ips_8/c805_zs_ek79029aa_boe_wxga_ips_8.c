#ifdef BUILD_LK
	#include <platform/mt_gpio.h>
	#include <string.h>
	#include <platform/mt_pmic.h>
	#include <platform/mt_typedefs.h>
#elif defined(BUILD_UBOOT)
	#include <asm/arch/mt_gpio.h>
#else
#include <linux/string.h>
#include <linux/wait.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/pinctrl/consumer.h>
#include <linux/of_gpio.h>
#include <asm-generic/gpio.h>
#include <mt-plat/mt_gpio.h>
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


//---------------------------------------------------------------------------
//LocalConstants
//---------------------------------------------------------------------------
#define FRAME_WIDTH (800)
#define FRAME_HEIGHT (1280)
  
#ifndef TRUE
    #define TRUE 1
#endif

#ifndef FALSE
    #define FALSE 0
#endif

#ifdef BUILD_LK
#define GPIO_LCD_POWER_EN     (GPIO83  | 0x80000000) 
#define GPIO_LCM_RST     GPIO19
#define GPIO_LCD_3v3     (GPIO21  | 0x80000000) 
static unsigned int lcm_esd_test = FALSE;      ///only for ESD test
#define REGFLAG_DELAY             							0XFE
#define REGFLAG_END_OF_TABLE      							0xFF   // END OF REGISTERS MARKER
#else
extern unsigned int GPIO_LCM_PWR;
extern unsigned int GPIO_LCM_VDD;
extern unsigned int GPIO_LCM_RST;
#endif

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------
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

//static struct LCM_setting_table lcm_initialization_setting[] = {
static void init_lcm_registers(void)
{
	unsigned int data_array[16];
	data_array[0] = 0xAACD1500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);

	data_array[0] = 0x00301500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);

	data_array[0] = 0x11331500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);

	data_array[0] = 0x00321500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);

	data_array[0] = 0x41361500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);

	data_array[0] = 0x023A1500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);

	data_array[0] = 0x82671500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);

	data_array[0] = 0x27691500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);

	data_array[0] = 0x016D1500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);

	data_array[0] = 0x16681500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);

	data_array[0] = 0x00093902;
	data_array[1] = 0x000F0055;
	data_array[2] = 0x000F000F;
	data_array[3] = 0x0000000F;
	dsi_set_cmdq(data_array, 4, 1);
	MDELAY(1);

	data_array[0] = 0x00113902;
	data_array[1] = 0x000F0056;
	data_array[2] = 0x000F000F;
	data_array[3] = 0x000F000F;
	data_array[4] = 0x000F000F;
	data_array[5] = 0x0000000F;
	dsi_set_cmdq(data_array, 6, 1);
	MDELAY(1);

	data_array[0] = 0x30731500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);

	data_array[0] = 0x035E1500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);

	data_array[0] = 0x5E411500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);

	data_array[0] = 0x84611500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);

	data_array[0] = 0x10741500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);

	data_array[0] = 0x203F1500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);

	data_array[0] = 0x14471500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);

	data_array[0] = 0x66481500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);

	data_array[0] = 0x504F1500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);

	data_array[0] = 0x4F4E1500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);

	data_array[0] = 0x11391500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);

	data_array[0] = 0x10601500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);

	data_array[0] = 0xD0501500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);

	data_array[0] = 0x34761500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);

	data_array[0] = 0x0E2E1500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);

	data_array[0] = 0x00143902;
	data_array[1] = 0x181A1F53;
	data_array[2] = 0x16141414;
	data_array[3] = 0x10131718;
	data_array[4] = 0x0B0E0E0F;
	data_array[5] = 0x01030609;
	dsi_set_cmdq(data_array, 6, 1);
	MDELAY(1);

	data_array[0] = 0x00143902;
	data_array[1] = 0x181A1F54;
	data_array[2] = 0x16141414;
	data_array[3] = 0x12151918;
	data_array[4] = 0x0B0E0E0F;
	data_array[5] = 0x01030609;
	dsi_set_cmdq(data_array, 6, 1);
	MDELAY(1);

	data_array[0] = 0x285F1500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);

	data_array[0] = 0x24631500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);

	data_array[0] = 0x31281500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);

	data_array[0] = 0x10291500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);

	data_array[0] = 0xFC341500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);

	data_array[0] = 0x312D1500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);

	data_array[0] = 0x67781500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);

	data_array[0] = 0x004D1500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);


	data_array[0] = 0x00110500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(120);

};
/*
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
*/
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

//#if (LCM_DSI_CMD_MODE)
//	params->dsi.mode   = CMD_MODE;
//#else
    params->dsi.mode   = BURST_VDO_MODE;
//#endif

	// DSI
	/* Command mode setting */
	params->dsi.LANE_NUM					= LCM_THREE_LANE;	//LCM_TWO_LANE;	//LCM_FOUR_LANE;
	//The following defined the fomat for data coming from LCD engine.
	params->dsi.data_format.color_order 	= LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq   	= LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding     	= LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format      	= LCM_DSI_FORMAT_RGB888;

	// Highly depends on LCD driver capability.
	params->dsi.packet_size=256;
	//video mode timing
	params->dsi.intermediat_buffer_num = 2;
	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

    params->dsi.word_count=FRAME_WIDTH*3;

	
	params->dsi.vertical_sync_active                            = 6; //2; //4;
    params->dsi.vertical_backporch                              = 8; //10; //16;
    params->dsi.vertical_frontporch                             = 16;//5; 
    params->dsi.vertical_active_line                            = FRAME_HEIGHT; 

    params->dsi.horizontal_sync_active                          = 60; // 10; //5;//6;
    params->dsi.horizontal_backporch                            = 60; //60; //60; //80;
    params->dsi.horizontal_frontporch                           = 50; //60; 

	params->dsi.PLL_CLOCK = 310;//256;	// 256  251 //trz 300
	params->dsi.ssc_disable = 1;

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
	//push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
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
       
LCM_DRIVER c805_zs_ek79029aa_boe_wxga_ips_8_lcm_drv= 
{
	.name			= "c805_zs_ek79029aa_boe_wxga_ips_8",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend			=lcm_suspend_jd9364,
	.resume				=lcm_resume_jd9364,
	.suspend_power 	= lcm_suspend_power,
	.resume_power 		= lcm_resume_power,
	.init_power 		= lcm_init_power,
	
};

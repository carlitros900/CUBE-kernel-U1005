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
extern unsigned int GPIO_LCM_PWR; //1.8v
extern unsigned int GPIO_LCM_RST;
extern unsigned int GPIO_LCM_VDD; //3.7v
#define GPIO_LCM_EN GPIO_LCM_VDD

extern struct regulator *lcm_vgp;

bool lcm_is_c710_lt070me05000_wxga = false;
bool lcm_c710_lt070me05000_bl_en = false;

#include <mach/upmu_sw.h>

//---------------------------------------------------------------------------
//LocalConstants
//---------------------------------------------------------------------------

#define FRAME_WIDTH (1200)
#define FRAME_HEIGHT (1920)
//---------------------------------------------------------------------------
//LocalVariables
//---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v) (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))
#define REGFLAG_DELAY 0xFD
#define REGFLAG_END_OF_TABLE 0xFE
#define GPIO_LCD_RST_EN 0xFFFFFFFF

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
static unsigned int is_lcm_inited=0;

struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};

static unsigned bl_cmd = 0x51;
static unsigned int bl_level = 0x66;
int c710_lt070me05000_wxga_set_bl(unsigned int level)
{
	bl_level = level;
	dsi_set_cmdq_V2(bl_cmd, 1, (unsigned char *)&level, 1);
	//printk(" ########### c710_lt070me05000_wxga_set_bl  bl_level = 0x%x \r\n",level);
	return 0;
}


static struct LCM_setting_table lcm_initialization_setting[] = {

        {0x01,0,{0x00}},
		{REGFLAG_DELAY, 50, {}}, 
        {0xb0,1,{0x04}},
        {0xb3,5,{0x04,0x08,0x00,0x22,0x00}},
        {0xb4,1,{0x0c}},
        {0xb6,2,{0x3a,0xd3}},
        {0x51,1,{0xe6}}, //backlight 0x00~0xff
        {0x53,1,{0x2c}},
        {0x3a,1,{0x77}},
 //       {0x2a,4,{0x00,0x00,0x04,0xaf}},
 //       {0x2b,4,{0x00,0x00,0x07,0x7f}},
 //       {0x2c,0,{0x00}},
        {0x11,0,{0x00}},
        {REGFLAG_DELAY, 150, {}},  
         {0x29,0,{0x00}},
        {REGFLAG_DELAY, 50, {}},
        {0xd6,1,{0x01}},   
        {0xb3,5,{0x14,0x08,0x00,0x22,0x00}},  
        {REGFLAG_END_OF_TABLE,0x00,{}}
 
};

static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;
	//printk(" ##########  %s \r\n",__func__);

	for(i = 0; i < count; i++) {
		unsigned cmd;
		cmd = table[i].cmd;
		switch (cmd) {
		case REGFLAG_DELAY :
			MDELAY(table[i].count);
			break;
		case REGFLAG_END_OF_TABLE :
			break;
		default:
			dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
		}
	}
}

static void lcd_power_en(unsigned char enabled)
{

#ifdef BUILD_LK
		printf("%s(%d), LK \n", __func__, enabled?1:0);
#else
		printk("%s(%d), kernel", __func__, enabled?1:0);
#endif

	if (enabled)
	{
		lcm_set_gpio_output(GPIO_LCM_PWR, GPIO_OUT_ONE);
		MDELAY(20);
		lcm_set_gpio_output(GPIO_LCM_EN, GPIO_OUT_ONE);
		/*MDELAY(20);
		lcm_set_gpio_output(GPIO_LCM_PWR, GPIO_OUT_ONE);
*/
	}
	else
	{
		lcm_set_gpio_output(GPIO_LCM_EN, 0);
        MDELAY(20);
		lcm_set_gpio_output(GPIO_LCM_PWR, 0);     
/*		MDELAY(20);
		lcm_set_gpio_output(GPIO_LCM_EN, 0);
*/
	}
	MDELAY(50);
}

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
    }
    else
    {   
        lcm_set_gpio_output(GPIO_LCM_RST, 0);
    }
    MDELAY(50);

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
    memset(params,0,sizeof(LCM_PARAMS));

    params->type=LCM_TYPE_DSI;
    params->width=FRAME_WIDTH;
    params->height=FRAME_HEIGHT;
    params->dsi.mode=BURST_VDO_MODE;    //SYNC_PULSE_VDO_MODE = 1,SYNC_EVENT_VDO_MODE = 2,BURST_VDO_MODE = 3

    //DSI
    /*Commandmodesetting*/
    params->dsi.LANE_NUM=LCM_FOUR_LANE;
    //ThefollowingdefinedthefomatfordatacomingfromLCDengine.
    params->dsi.data_format.format=LCM_DSI_FORMAT_RGB888;

    params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
	//params->dsi.LC_HS_TX_EN=1;
    params->dsi.vertical_sync_active=2;
    params->dsi.vertical_backporch=4;
    params->dsi.vertical_frontporch=4;
    params->dsi.vertical_active_line=FRAME_HEIGHT;

    params->dsi.horizontal_sync_active=12;
    params->dsi.horizontal_backporch=48;
    params->dsi.horizontal_frontporch=136;
    params->dsi.horizontal_active_pixel=FRAME_WIDTH;

	//Bitratecalculation
	params->dsi.PLL_CLOCK=485;
	
	lcm_is_c710_lt070me05000_wxga = true;
}

static void lcm_init(void)
{
	//printk(" ##########  %s \r\n",__func__);
	
	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
	dsi_set_cmdq_V2(bl_cmd, 1, (unsigned char *)&bl_level, 1); //backlight

	MDELAY(100);

    if(0==is_lcm_inited){
		MDELAY(100);
		is_lcm_inited=1;
    }

}

static void lcm_suspend(void)
{
    unsigned int data_array[16];
	//printk(" ##########  %s \r\n",__func__);

    data_array[0]=0x00281500;
    dsi_set_cmdq(data_array,1,1);
    MDELAY(20);
    data_array[0]=0x00101500;
    dsi_set_cmdq(data_array,1,1);
    MDELAY(80);

}

static void lcm_suspend_power(void)
{
	//printk(" ##########  %s \r\n",__func__);

	lcm_c710_lt070me05000_bl_en = false;

    lcd_reset(0);
    MDELAY(20);	
    lcd_power_en(0);
    MDELAY(20);

}

static void lcm_resume(void)
{
	//printk(" ##########  %s \r\n",__func__);

    lcm_init();
    //MDELAY(50);
}

static void lcm_resume_power(void)
{
	//printk(" ##########  %s \r\n",__func__);

	lcm_c710_lt070me05000_bl_en = true;

   	lcd_reset(0);
    lcd_power_en(0);
	MDELAY(20);
    lcd_power_en(1);
    MDELAY(50);

    lcd_reset(1);
    MDELAY(50); 

}

static void lcm_init_power(void)
{
	//printk(" ##########  %s \r\n",__func__);

   	lcd_reset(0);
    lcd_power_en(0);
	MDELAY(20);
    lcd_power_en(1);
    MDELAY(50);

    lcd_reset(1);
	MDELAY(50);  
	
}

LCM_DRIVER c710_lt070me05000_wxga_lcm_drv={
	.name				="c710_lt070me05000_wxga",
	.set_util_funcs	=lcm_set_util_funcs,
	.get_params			=lcm_get_params,
	.init				=lcm_init,
	.suspend			=lcm_suspend,
	.resume				=lcm_resume,
	.suspend_power 	= lcm_suspend_power,
	.resume_power 		= lcm_resume_power,
	.init_power 		= lcm_init_power,
	
};

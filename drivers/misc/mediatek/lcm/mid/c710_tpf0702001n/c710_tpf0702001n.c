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

extern struct regulator *lcm_vgp;

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
//static unsigned int is_lcm_inited=0;

struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};

//#define LCM_BIST
#ifdef LCM_BIST
static struct LCM_setting_table lcm_bist_initialization_setting[] = {
 /*
        {0x01,0,{0x00}},        //data type 05
        {0xb0,1,{0x00}},       //data type 23
        {0xb3,5,{0x14,0x08,0x00,0x22,0x00}},  //data type 29
        {0xb4,1,{0x0c}},                 //data type  29
        {0xb6,2,{0x3a,0xd3}},            //data type  29
        {0x51,1,{0xe6}},                //data type 15
        {0x53,1,{0x2c}},                //data type 15
        {0x29,1,{0x00}},              //data type 05
        {REGFLAG_DELAY, 120, {}},  
        {0x11,1,{0x00}},             //data type  05
        {REGFLAG_DELAY, 20, {}},      
        {REGFLAG_END_OF_TABLE,0x00,{}}
*/
		{0x01,0,{0x00}},
		{REGFLAG_DELAY, 50, {}}, 
		{0xb0,1,{0x00}},
		{0xb4,1,{0x0c}},  
		{0xb6,2,{0x3a,0xd3}}, 
		{0xde,5,{0x01,0x00,0x3f,0xff,0x10}}, 
		{REGFLAG_END_OF_TABLE,0x00,{}}	
 
};

static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;
	printk(" ##########  %s \r\n",__func__);

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
#else
static void init_lcm_registers(void)
{		

    unsigned int data_array[16];

	data_array[0] = 0x00011500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(50);
	data_array[0] = 0x00b01500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00063902;
	data_array[1] = 0x000814b3;
	data_array[2] = 0x00000022;
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0] = 0x0cb41500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00033902;
	data_array[1] = 0x00d33ab6;
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0] = 0xe6511500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x2c531500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00291500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(50);
	data_array[0] = 0x00111500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(50);

}
#endif

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
		MDELAY(50);
		lcm_set_gpio_output(GPIO_LCM_EN, GPIO_OUT_ONE);
	}
	else
	{	
		lcm_set_gpio_output(GPIO_LCM_EN, 0);
        MDELAY(50);
		lcm_set_gpio_output(GPIO_LCM_PWR, 0);      
	}
	//MDELAY(30);
	
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
    //MDELAY(30);

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
// 1 16 15 4 40 40 fps 60
    params->dsi.vertical_sync_active=2;//1;//4;//1;//4       
    params->dsi.vertical_backporch=4;//180;//3;//16;//4// 20 16
    params->dsi.vertical_frontporch=4;//10;//5;//15;//4// 20 15
    params->dsi.vertical_active_line=FRAME_HEIGHT;

    params->dsi.horizontal_sync_active=12;///4;//20 
    params->dsi.horizontal_backporch=48;//70;//48;//40;//50 20 80
    params->dsi.horizontal_frontporch=136;//40;//16;//40;//60 20 80
    params->dsi.horizontal_active_pixel=FRAME_WIDTH;

	//Bitratecalculation
	params->dsi.PLL_CLOCK=485;//230;

}

static void lcm_init(void)
{
	printk(" ##########  %s \r\n",__func__);
	
	#ifdef LCM_BIST
		push_table(lcm_bist_initialization_setting, sizeof(lcm_bist_initialization_setting) / sizeof(struct LCM_setting_table), 1);

	#else
		init_lcm_registers();
	#endif

	MDELAY(200);

    /*if(0==is_lcm_inited){
		MDELAY(100);
		is_lcm_inited=1;
    }*/

}

static void lcm_suspend(void)
{/*
    unsigned int data_array[16];
	printk(" ##########  %s \r\n",__func__);

    data_array[0]=0x00281500;
    dsi_set_cmdq(data_array,1,1);
    MDELAY(10);
    data_array[0]=0x00101500;
    dsi_set_cmdq(data_array,1,1);
    MDELAY(10);
*/
}

static void lcm_suspend_power(void)
{
	printk(" ##########  %s \r\n",__func__);

    lcd_power_en(0);
    MDELAY(50);
    lcd_reset(0);
    MDELAY(50);
}

static void lcm_resume(void)
{
	printk(" ##########  %s \r\n",__func__);

    lcm_init();
    //MDELAY(50);
}

static void lcm_resume_power(void)
{
	printk(" ##########  %s \r\n",__func__);

   	lcd_reset(0);
	MDELAY(50);
    lcd_power_en(0);
	MDELAY(50);
    lcd_power_en(1);
    MDELAY(50);

    lcd_reset(1);
    MDELAY(50); 

}

static void lcm_init_power(void)
{
	printk(" ##########  %s \r\n",__func__);

   	lcd_reset(0);
	MDELAY(50);
    lcd_power_en(0);
	MDELAY(50);
    lcd_power_en(1);
    MDELAY(50);

    lcd_reset(1);
	MDELAY(50); 
	
}

LCM_DRIVER c710_tpf0702001n_lcm_drv={
	.name				="c710_tpf0702001n",
	.set_util_funcs	=lcm_set_util_funcs,
	.get_params			=lcm_get_params,
	.init				=lcm_init,
	.suspend			=lcm_suspend,
	.resume				=lcm_resume,
	.suspend_power 	= lcm_suspend_power,
	.resume_power 		= lcm_resume_power,
	.init_power 		= lcm_init_power,
	
};

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
extern struct regulator *lcm_vgp;





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




int lcm_vgp_supply_enable_c102_wuxga(void)
{
        int ret;
       // unsigned int volt;

        printk("LCM: lcm_vgp_supply_enable\n");
        if (NULL == lcm_vgp){
			printk("++++++++++lxl++++++++++++++++++\n");
            return 0;
        }
       // printk("LCM: set regulator voltage lcm_vgp voltage to 1.8V\n");
       // /* set voltage to 1.8V */
       // ret = regulator_set_voltage(lcm_vgp, 3300000, 3300000);
		//MDELAY(20);
       // if (ret != 0) {
       //         printk("LCM: lcm failed to set lcm_vgp voltage: %d\n", ret);
        //       return ret;
       //}

        /* get voltage settings again */
		//printk("==========lxl================\n");
       // volt = regulator_get_voltage(lcm_vgp);
		//MDELAY(20);
        //if (volt == 3300000)
        //        printk("LCM: check regulator voltage=1800000 pass!\n");
       // else
       //        printk("LCM: check regulator voltage=1800000 fail! (voltage: %d)\n", volt);

        ret = regulator_enable(lcm_vgp);
		MDELAY(15);
		printk("&&&&&&&&&lxl&&&&&&&&&&&&&&\n");
        if (ret != 0) {
                printk("LCM: Failed to enable lcm_vgp: %d\n", ret);
                return ret;
        }

        return ret;
}

int lcm_vgp_supply_disable_c102_wuxga(void)
{
        int ret = 0;
        unsigned int isenable;

        if (NULL == lcm_vgp)
                return 0;

        /* disable regulator */
        isenable = regulator_is_enabled(lcm_vgp);

        printk("LCM: lcm query regulator enable status[0x%d]\n", isenable);

        if (isenable) {
                ret = regulator_disable(lcm_vgp);
                if (ret != 0) {
                        printk("LCM: lcm failed to disable lcm_vgp: %d\n", ret);
                        return ret;
                }
                /* verify */
                isenable = regulator_is_enabled(lcm_vgp);
                if (!isenable)
                        printk("LCM: lcm regulator disable pass\n");
        }

        return ret;
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


static struct LCM_setting_table lcm_initialization_setting[] = {		
		{0x11,1,{0x00}},
		{REGFLAG_DELAY, 120, {}},
		{0x29,1,{0x00}},
		{REGFLAG_DELAY, 20, {}},
		
	
	    //{0xF0,5,{0x55,0xAA,0x52,0x08,0x00}},//========== Internal setting ==========
		//{0xEF,2,{0x07,0xFF}},//========== Internal setting ==========
		//{0xEE,4,{0X87,0X78,0X02,0X40}},//========== Internal setting ==========
		//{REGFLAG_END_OF_TABLE, 0x00, {}}			
};

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
		lcm_vgp_supply_enable_c102_wuxga();

	}
	else
	{	
		lcm_set_gpio_output(GPIO_LCM_PWR, 0);
        MDELAY(20);
		lcm_vgp_supply_disable_c102_wuxga();
        
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
    params->dsi.vertical_sync_active=1;//1;//4;//1;//4       
    params->dsi.vertical_backporch=25;//180;//3;//16;//4// 20 16
    params->dsi.vertical_frontporch=35;//10;//5;//15;//4// 20 15
    params->dsi.vertical_active_line=FRAME_HEIGHT;

    params->dsi.horizontal_sync_active=1;///4;//20 
    params->dsi.horizontal_backporch=60;//70;//48;//40;//50 20 80
    params->dsi.horizontal_frontporch=80;//40;//16;//40;//60 20 80
    params->dsi.horizontal_active_pixel=FRAME_WIDTH;




	//Bitratecalculation
	params->dsi.PLL_CLOCK=500;//230;
	params->dsi.cont_clock = 1;

}
static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;
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

extern void DSI_clk_HS_mode(unsigned char enter);
//extern void DSI_Continuous_HS(void);


static void lcm_init(void)
{
   

	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
	
	//DSI_clk_HS_mode(1);
	MDELAY(80);



    if(0==is_lcm_inited){
		//DSI_Continuous_HS();
		//DSI_clk_HS_mode(1);
		MDELAY(100);
		
		is_lcm_inited=1;
    }
	
	
	
}

static void lcm_suspend(void)
{
    unsigned int data_array[16];
    #ifdef BUILD_LK
    printf("%s,LK\n",__func__);
    #else
    printk("%s,kernel",__func__);
    #endif
    data_array[0]=0x00280500;//displayoff
    dsi_set_cmdq(data_array,1,1);
    MDELAY(15);
	//DSI_clk_HS_mode(0);
	is_lcm_inited=0;
	printk("*****lxl(lcm_suspend)**********\n");
}

static void lcm_resume(void)
{

	lcm_init();
    MDELAY(10);
}

static void lcm_suspend_power(void)
{
    MDELAY(10);
    lcd_power_en(0);
    MDELAY(10);
}

static void lcm_resume_power(void)
{
	  
        lcm_set_gpio_output(GPIO_LCM_PWR, 1);
		MDELAY(10);
        lcm_vgp_supply_enable_c102_wuxga();
		MDELAY(20);
}

static void lcm_init_power(void){
	
  printk("=====lxl(lcm_init_power)=====\n");
}

LCM_DRIVER c102_nt51021_wuxga_lcm_drv={
	.name				="c102_nt51021_wuxga",
	.set_util_funcs	=lcm_set_util_funcs,
	.get_params			=lcm_get_params,
	.init				=lcm_init,
	.suspend			=lcm_suspend,
	.resume				=lcm_resume,
	.suspend_power 	= lcm_suspend_power,
	.resume_power 		= lcm_resume_power,
	.init_power 		= lcm_init_power,
};

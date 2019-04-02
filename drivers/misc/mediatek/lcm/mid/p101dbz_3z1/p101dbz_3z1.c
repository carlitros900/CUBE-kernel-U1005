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

#define FRAME_WIDTH (800)
#define FRAME_HEIGHT (1280)
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




int lcm_vgp_supply_enable(void)
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

int lcm_vgp_supply_disable(void)
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


		{0xFF,4,{0xAA,0x55,0x25,0x01}},//========== Internal setting ==========
		{0xFC,1,{0x08}},
		{REGFLAG_DELAY, 1, {}},
		{0xFC,1,{0x00}},
		{REGFLAG_DELAY, 1, {}},
		{0x6F,3,{0x21,0xF7,0x01}},
		{REGFLAG_DELAY, 1, {}},
		{0x6F,3,{0x21,0xF7,0x00}},
		{REGFLAG_DELAY, 1, {}},
		{0x6F,3,{0x1A,0xF7,0x05}},
		{REGFLAG_DELAY, 1, {}},
		{0xFF,4,{0xAA,0x55,0x25,0x00}},
		{0xFF,4,{0xAA,0x55,0xA5,0x80}},

		{0x6F,2,{0x11,0x00}},// MIPI related Timing Setting
		{0xF7,2,{0x20,0x00}},

		{0x6F,1,{0x06}},//  Improve ESD option
		{0xF7,1,{0xA0}},
		{0x6F,1,{0x19}},
		{0xF7,1,{0x12}},
		{0xF4,1,{0x03}},

		{0x6F,1,{0x08}},// Vcom floating
		{0xFA,1,{0x40}},
		{0x6F,1,{0x11}},
		{0xF3,1,{0x01}},

		{0x6f,1,{0x06}},
		{0xfc,1,{0x03}},
		
		{0xF0,5,{0x55,0xAA,0x52,0x08,0x00}},//========== page0 relative ==========
		{0xB1,2,{0x68,0x01}},
		{0xB6,1,{0x08}},// Set source output hold time

		{0x6F,1,{0x02}},//EQ control function
		{0xB8,1,{0x08}},

		{0xBB,2,{0x54,0x44}},// Set bias current for GOP and SOP

		{0xBC,2,{0x05,0x05}},// Inversion setting 
		{0xC7,1,{0x01}},

		{0xBD,5,{0x02,0xB0,0x1E,0x1E,0x00}},// DSP Timing Settings update for BIST
		{0xC5,2,{0x01,0x07}},
		{0xC8,1,{0x80}},

		{0xF0,5,{0x55,0xAA,0x52,0x08,0x01}},//========== page1 relative ==========

		{0xB0,2,{0x05,0x05}},// Setting AVDD, AVEE clamp
		{0xB1,2,{0x05,0x05}},
		{0xB2,2,{0x00,0x00}},

		{0xBC,2,{0x90,0x01}},// VGMP, VGMN, VGSP, VGSN setting
		{0xBD,2,{0x90,0x01}},

		{0xCA,1,{0x00}},// gate signal control

		{0xC0,1,{0x04}},// power IC control
		//{0xB2,2,{0x00,0x00}},// VCL SET -2.5V

		{0xBE,1,{0x29}},// VCOM = -1.888V

		{0xB3,2,{0x28,0x28}},// Setting VGH=15V, VGL=-11V
		{0xB4,2,{0x12,0x12}},

		{0xB9,2,{0x35,0x35}},// power control for VGH, VGL
		{0xBA,2,{0x25,0x25}},

		{0xF0,5,{0x55,0xAA,0x52,0x08,0x02}},//========== page2 relative ==========

		{0xEE,1,{0x01}},//gamma setting
		{0xEF,4,{0x09,0x06,0x15,0x18}},//Gradient Control for Gamma Voltage

		{0xB0,6,{0x00,0x00,0x00,0x24,0x00,0x55}},
		{0x6F,1,{0x06}},
		{0xB0,6,{0x00,0x77,0x00,0x94,0x00,0xc0}},
		{0x6F,1,{0x0C}},
		{0xB0,4,{0x00,0xE3,0x01,0x1A}},                                                                                                                                            
		{0xB1,6,{0x01,0x46,0x01,0x88,0x01,0xbc}},
		{0x6F,1,{0x06}},
		{0xB1,6,{0x02,0x0B,0x02,0x4B,0x02,0x4D}},
		{0x6F,1,{0x0C}},
		{0xB1,4,{0x02,0x88,0x02,0xC9}},
		{0xB2,6,{0x02,0xF3,0x03,0x29,0x03,0x4E}},
		{0x6F,1,{0x06}},
		{0xB2,6,{0x03,0x7D,0x03,0x9B,0x03,0xBE}},
		{0x6F,1,{0x0C}},
		{0xB2,4,{0x03,0xD3,0x03,0xE9}},
		{0xB3,4,{0x03,0xfB,0x03,0xFF}},

		//========== GOA relative ==========
		{0xF0,5,{0x55,0xAA,0x52,0x08,0x06}},// PAGE6 : GOUT Mapping, VGLO select
		{0xB0,2,{0x0B,0x2E}},
		{0xB1,2,{0x2E,0x2E}},
		{0xB2,2,{0x2E,0x09}},
		{0xB3,2,{0x2a,0x29}},
		{0xB4,2,{0x1B,0x19}},
		{0xB5,2,{0x17,0x15}},
		{0xB6,2,{0x13,0x11}},
		{0xB7,2,{0x01,0x2E}},
		{0xB8,2,{0x2E,0x2E}},
		{0xB9,2,{0x2E,0x2E}},
		{0xBA,2,{0x2E,0x2E}},
		{0xBB,2,{0x2E,0x2E}},
		{0xBC,2,{0x2E,0x00}},
		{0xBD,2,{0x10,0x12}},
		{0xBE,2,{0x14,0x16}},
		{0xBF,2,{0x18,0x1a}},
		{0xC0,2,{0x29,0x2A}},
		{0xC1,2,{0x08,0x2E}},
		{0xC2,2,{0x2E,0x2E}},
		{0xC3,2,{0x2E,0x0A}},
		{0xE5,2,{0x2E,0x2E}},
		{0xC4,2,{0x0A,0x2E}},
		{0xC5,2,{0x2E,0x2E}},
		{0xC6,2,{0x2E,0x00}},
		{0xC7,2,{0x2A,0x29}},
		{0xC8,2,{0x10,0x12}},
		{0xC9,2,{0x14,0x16}},
		{0xCA,2,{0x18,0x1A}},
		{0xCB,2,{0x08,0x2E}},
		{0xCC,2,{0x2E,0x2E}},
		{0xCD,2,{0x2E,0x2E}},
		{0xCE,2,{0x2E,0x2E}},
		{0xCF,2,{0x2E,0x2E}},
		{0xD0,2,{0x2E,0x09}},
		{0xD1,2,{0x1B,0x19}},
		{0xD2,2,{0x17,0x15}},
		{0xD3,2,{0x13,0x11}},
		{0xD4,2,{0x29,0x2A}},
		{0xD5,2,{0x01,0x2E}},
		{0xD6,2,{0x2E,0x2E}},
		{0xD7,2,{0x2E,0x0B}},
		{0xE6,2,{0x2E,0x2E}},
		{0xD8,5,{0x00,0x00,0x00,0x00,0x00}},//VGL level select
		{0xD9,5,{0x00,0x00,0x00,0x00,0x00}},
		{0xE7,1,{0x00}},

		// PAGE3 :
		{0xF0,5,{0x55,0xAA,0x52,0x08,0x03}},//gate timing control
		{0xB0,2,{0x20,0x00}},
		{0xB1,2,{0x20,0x00}},
		{0xB2,5,{0x05,0x00,0x00,0x00,0x00}},		
		{0xB6,5,{0x05,0x00,0x00,0x00,0x00}},
		{0xB7,5,{0x05,0x00,0x00,0x00,0x00}},
		{0xBA,5,{0x57,0x00,0x00,0x00,0x00}},
		{0xBB,5,{0x57,0x00,0x00,0x00,0x00}},
		{0xc0,4,{0x00,0x00,0x00,0x00}},
		{0xc1,4,{0x00,0x00,0x00,0x00}},
		{0xC4,1,{0x60}},
		{0xC5,1,{0x40}},

		// PAGE5 :
		{0xF0,5,{0x55,0xAA,0x52,0x08,0x05}},
		{0xBD,5,{0x03,0x01,0x03,0x03,0x03}},
		{0xB0,2,{0x17,0x06}},	
		{0xB1,2,{0x17,0x06}},
		{0xB2,2,{0x17,0x06}},
		{0xB3,2,{0x17,0x06}},			
		{0xB4,2,{0x17,0x06}},
		{0xB5,2,{0x17,0x06}},
		{0xB8,1,{0x00}},	
		{0xB9,1,{0x00}},
		{0xBA,1,{0x00}},
		{0xBB,1,{0x02}},
		{0xBC,1,{0x00}},
		{0xC0,1,{0x07}},			
		{0xC4,1,{0x80}},
		{0xC5,1,{0xa4}},		
		{0xC8,2,{0x05,0x30}},
		{0xC9,2,{0x01,0x31}},	
		{0xcc,3,{0x00,0x00,0x3c}},
		{0xcd,3,{0x00,0x00,0x3c}},
		{0xd1,5,{0x00,0x05,0x09,0x07,0x10}},
		{0xd2,5,{0x00,0x05,0x0E,0x07,0x10}},
		
		{0xE5,1,{0x06}},
		{0xE6,1,{0x06}},
		{0xE7,1,{0x06}},
		{0xE8,1,{0x06}},
		{0xE9,1,{0x06}},
		{0xEa,1,{0x06}},
		
		{0xed,1,{0x30}},
		{0x6f,1,{0x11}},
		{0xf3,1,{0x01}},
		
		{0x35,1,{0x00}},
		
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
		lcm_vgp_supply_enable();

	}
	else
	{	
		lcm_set_gpio_output(GPIO_LCM_PWR, 0);
        MDELAY(20);
		lcm_vgp_supply_disable();
        
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
    params->dsi.mode=SYNC_PULSE_VDO_MODE;    //SYNC_PULSE_VDO_MODE = 1,SYNC_EVENT_VDO_MODE = 2,BURST_VDO_MODE = 3

    //DSI
    /*Commandmodesetting*/
    params->dsi.LANE_NUM=LCM_FOUR_LANE;
    //ThefollowingdefinedthefomatfordatacomingfromLCDengine.
    params->dsi.data_format.format=LCM_DSI_FORMAT_RGB888;

    params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
	//params->dsi.LC_HS_TX_EN=1;
// 1 16 15 4 40 40 fps 60
  params->dsi.vertical_sync_active=4;//4;//4;//1;//4       
    params->dsi.vertical_backporch=18;//110;//3;//16;//4// 20 16
    params->dsi.vertical_frontporch=18;//50;//5;//15;//4// 20 15
    params->dsi.vertical_active_line=FRAME_HEIGHT;

    params->dsi.horizontal_sync_active=4;///4;//20 
    params->dsi.horizontal_backporch=20;//70;//48;//40;//50 20 80
    params->dsi.horizontal_frontporch=20;//40;//16;//40;//60 20 80
    params->dsi.horizontal_active_pixel=FRAME_WIDTH;




	//Bitratecalculation
	params->dsi.PLL_CLOCK=230;//230;

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
   
	MDELAY(50);
	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
	
	//DSI_clk_HS_mode(1);
	MDELAY(10);


/*
    if(0==is_lcm_inited){
		//DSI_Continuous_HS();
		//DSI_clk_HS_mode(1);
		MDELAY(100);
		
		is_lcm_inited=1;
    }
*/	
	
	
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
	MDELAY(50);
	data_array[0]=0x00100500;//displayoff
    dsi_set_cmdq(data_array,1,1);
	
    MDELAY(120);
	DSI_clk_HS_mode(0);
	is_lcm_inited=0;
	printk("*****lxl(lcm_suspend)**********\n");
}

static void lcm_resume(void)
{

	lcm_init();
//    MDELAY(10);
}

static void lcm_suspend_power(void)
{
    MDELAY(10);
    lcd_power_en(0);
    MDELAY(10);
	lcm_vgp_supply_disable();
}

static void lcm_resume_power(void)
{
	  
        lcm_set_gpio_output(GPIO_LCM_PWR, 1);
		MDELAY(10);
        lcm_vgp_supply_enable();
		MDELAY(120);
}

static void lcm_init_power(void){
	lcm_set_gpio_output(GPIO_LCM_PWR, 1);
	MDELAY(10);
	lcm_vgp_supply_enable();
	MDELAY(120);
	printk("=====lxl(lcm_init_power)=====\n");
}

LCM_DRIVER p101dbz_3z1_lcm_drv={
	.name				="p101dbz_3z1",
	.set_util_funcs	=lcm_set_util_funcs,
	.get_params			=lcm_get_params,
	.init				=lcm_init,
	.suspend			=lcm_suspend,
	.resume				=lcm_resume,
	.suspend_power 	= lcm_suspend_power,
	.resume_power 		= lcm_resume_power,
	.init_power 		= lcm_init_power,
	
};

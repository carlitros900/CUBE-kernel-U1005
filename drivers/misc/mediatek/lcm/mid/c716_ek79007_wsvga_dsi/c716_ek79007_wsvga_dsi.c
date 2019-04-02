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

#define FRAME_WIDTH (1024)
#define FRAME_HEIGHT (600)
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
/*
static int lcm_vgp_supply_enable_ek79007(void)
{
        int ret;
       // unsigned int volt;
        printk("LCM: lcm_vgp_supply_enable\n");
        if (NULL == lcm_vgp){
			printk("++++++++++lxl++++++++++++++++++\n");
            return 0;
        }
        ret = regulator_enable(lcm_vgp);
		MDELAY(15);
		printk("&&&&&&&&&lxl&&&&&&&&&&&&&&\n");
        if (ret != 0) {
                printk("LCM: Failed to enable lcm_vgp: %d\n", ret);
                return ret;
        }
        return ret;
}

static int lcm_vgp_supply_disable_ek79007(void)
{
        int ret = 0;
        unsigned int isenable;

        if (NULL == lcm_vgp)
                return 0;
        isenable = regulator_is_enabled(lcm_vgp);
        printk("LCM: lcm query regulator enable status[0x%d]\n", isenable);

        if (isenable) {
                ret = regulator_disable(lcm_vgp);
                if (ret != 0) {
                        printk("LCM: lcm failed to disable lcm_vgp: %d\n", ret);
                        return ret;
                }
                isenable = regulator_is_enabled(lcm_vgp);
                if (!isenable)
                        printk("LCM: lcm regulator disable pass\n");
        }
        return ret;
}
*/

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

	data_array[0] = 0x10b21500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);

	//=================control display effction======================
	data_array[0] = 0xac801500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);

	data_array[0] = 0xb8811500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);

	data_array[0] = 0x09821500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);

	data_array[0] = 0x78831500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);

	data_array[0] = 0x7f841500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);

	data_array[0] = 0xbb851500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(1);

	data_array[0] = 0x70861500;
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
		MDELAY(10);
		lcm_set_gpio_output(GPIO_LCM_EN, GPIO_OUT_ONE);

		//lcm_vgp_supply_enable_ek79007();
	}
	else
	{	
		lcm_set_gpio_output(GPIO_LCM_EN, 0);
        MDELAY(10);
		lcm_set_gpio_output(GPIO_LCM_PWR, 0);     
		
		//lcm_vgp_supply_disable_ek79007();
	}
	MDELAY(30);
	
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
    MDELAY(30);

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
    memset(params, 0, sizeof(LCM_PARAMS));

    params->type   = LCM_TYPE_DSI;

    params->width  = FRAME_WIDTH;
    params->height = FRAME_HEIGHT;

    // enable tearing-free
    params->dbi.te_mode                                 = LCM_DBI_TE_MODE_VSYNC_ONLY;
    params->dbi.te_edge_polarity                = LCM_POLARITY_RISING;


    params->dsi.mode   = SYNC_EVENT_VDO_MODE;	//BURST_VDO_MODE;
    // DSI
    /* Command mode setting */
    params->dsi.LANE_NUM                                = LCM_TWO_LANE;
    //The following defined the fomat for data coming from LCD engine.
    params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
    params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
    params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
    params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB565;	//LCM_DSI_FORMAT_RGB888;

    // Highly depends on LCD driver capability.
    // Not support in MT6573
    //params->dsi.packet_size=256;

    // Video mode setting               
    //params->dsi.intermediat_buffer_num = 2;

    params->dsi.PS=LCM_PACKED_PS_16BIT_RGB565;	//LCM_PACKED_PS_24BIT_RGB888;

//    params->dsi.word_count=480*3;       //DSI CMD mode need set these two bellow params, different to 6577
 //   params->dsi.vertical_active_line=854;

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
#if 0
#if 0
    // Bit rate calculation
    params->dsi.pll_div1=29;            // fref=26MHz, fvco=fref*(div1+1)       (div1=0~63, fvco=500MHZ~1GHz)
    params->dsi.pll_div2=1;             // div2=0~15: fout=fvo/(2*div2)
#else
    // Bit rate calculation
    //1 Every lane speed
    //params->dsi.pll_select=1;
    //params->dsi.PLL_CLOCK  = LCM_DSI_6589_PLL_CLOCK_377;
    params->dsi.PLL_CLOCK=230;
    params->dsi.pll_div1=1;             // div1=0,1,2,3;div1_real=1,2,4,4 ----0: 546Mbps  1:273Mbps
    params->dsi.pll_div2=0;             // div2=0,1,2,3;div1_real=1,2,4,4       
#if (LCM_DSI_CMD_MODE)
    params->dsi.fbk_div =9; 
#else
    params->dsi.fbk_div =9;    // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)     
#endif
#endif
    /* ESD or noise interference recovery For video mode LCM only. */ // Send TE packet to LCM in a period of n frames and check the response. 
    params->dsi.lcm_int_te_monitor = FALSE; 
    params->dsi.lcm_int_te_period = 1; // Unit : frames 

    // Need longer FP for more opportunity to do int. TE monitor applicably. 
    if(params->dsi.lcm_int_te_monitor) 
        params->dsi.vertical_frontporch *= 2;  

    // Monitor external TE (or named VSYNC) from LCM once per 2 sec. (LCM VSYNC must be wired to baseband TE pin.) 
    params->dsi.lcm_ext_te_monitor = FALSE; 
    // Non-continuous clock 
    params->dsi.noncont_clock = TRUE; 
    params->dsi.noncont_clock_period = 2; // Unit : frames
#endif
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
printk(" ##########  %s \r\n",__func__);
}

static void lcm_suspend_power(void)
{
	printk(" ##########  %s \r\n",__func__);

    lcd_reset(0);
    MDELAY(10);
    lcd_power_en(0);
    MDELAY(10);
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
    lcd_power_en(0);
	MDELAY(20);
    lcd_power_en(1);
    MDELAY(100);

    lcd_reset(1);
    MDELAY(50);
	lcd_reset(0);
	MDELAY(50);
	lcd_reset(1);
    MDELAY(200); 
}

static void lcm_init_power(void)
{
	printk(" ##########  %s \r\n",__func__);

   	lcd_reset(0);
    lcd_power_en(0);
	MDELAY(20);
    lcd_power_en(1);
    MDELAY(100);

    lcd_reset(1);
    MDELAY(50);
	lcd_reset(0);
	MDELAY(50);
	lcd_reset(1);
    MDELAY(200); 
	
}

LCM_DRIVER c716_ek79007_wsvga_dsi_lcm_drv={
	.name				="c716_ek79007_wsvga_dsi",
	.set_util_funcs	=lcm_set_util_funcs,
	.get_params			=lcm_get_params,
	.init				=lcm_init,
	.suspend			=lcm_suspend,
	.resume				=lcm_resume,
	.suspend_power 	= lcm_suspend_power,
	.resume_power 		= lcm_resume_power,
	.init_power 		= lcm_init_power,
	
};

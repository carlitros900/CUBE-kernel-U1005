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
data_array[0] = 0x00053902;
data_array[1] = 0x2555AAFF;
data_array[2] = 0x00000001;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);

data_array[0] = 0x08FC1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00FC1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x216F1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x01F71500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x216F1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00F71500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1A6F1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x05F71500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00053902;
data_array[1] = 0x2555AAFF;
data_array[2] = 0x00000000;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);

data_array[0] = 0x00063902;
data_array[1] = 0x52AA55F0;
data_array[2] = 0x00000008;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x000168B1;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x08B61500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x026F1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x08B81500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x004454BB;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x000505BC;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x01C71500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00063902;
data_array[1] = 0x1EB002BD;
data_array[2] = 0x0000001E;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x000701C5;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x83C81500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00063902;
data_array[1] = 0x52AA55F0;
data_array[2] = 0x00000108;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x000505B0;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x000505B1;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x000190BC;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x000190BD;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00CA1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x04C01500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x29BE1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x002828B3;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x001212B4;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x003434B9;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x002424BA;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00063902;
data_array[1] = 0x52AA55F0;
data_array[2] = 0x00000208;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);

data_array[0] = 0x01EE1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00053902;
data_array[1] = 0x150609EF;
data_array[2] = 0x00000018;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);

data_array[0] = 0x00073902;
data_array[1] = 0x000000B0;
data_array[2] = 0x00350019;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);

data_array[0] = 0x066F1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00073902;
data_array[1] = 0x004800B0;
data_array[2] = 0x00770059;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);

data_array[0] = 0x0C6F1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00053902;
data_array[1] = 0x009A00B0;
data_array[2] = 0x000000CF;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);

data_array[0] = 0x00073902;
data_array[1] = 0x01FC00B1;
data_array[2] = 0x007C0143;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);

data_array[0] = 0x066F1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00073902;
data_array[1] = 0x02D201B1;
data_array[2] = 0x001C021A;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);

data_array[0] = 0x0C6F1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00053902;
data_array[1] = 0x025F02B1;
data_array[2] = 0x000000A9;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);

data_array[0] = 0x00073902;
data_array[1] = 0x03D902B2;
data_array[2] = 0x004B031C;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);

data_array[0] = 0x066F1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00073902;
data_array[1] = 0x038603B2;
data_array[2] = 0x00BF039E;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);

data_array[0] = 0x0C6F1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00053902;
data_array[1] = 0x03D303B2;
data_array[2] = 0x000000EA;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);

data_array[0] = 0x00053902;
data_array[1] = 0x03FD03B3;
data_array[2] = 0x000000F3;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);

data_array[0] = 0x00063902;
data_array[1] = 0x52AA55F0;
data_array[2] = 0x00000608;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x002E0BB0;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x002E2EB1;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x00092EB2;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x00292AB3;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x00191BB4;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x001517B5;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x001113B6;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x002E01B7;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x002E2EB8;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x002E2EB9;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x002E2EBA;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x002E2EBB;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x00002EBC;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x001210BD;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x001614BE;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x001A18BF;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x002A29C0;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x002E08C1;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x002E2EC2;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x000A2EC3;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x002E2EE5;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x002E0AC4;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x002E2EC5;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x00002EC6;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x00292AC7;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x001210C8;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x001614C9;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x001A18CA;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x002E08CB;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x002E2ECC;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x002E2ECD;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x002E2ECE;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x002E2ECF;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x00092ED0;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x00191BD1;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x001517D2;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x001113D3;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x002A29D4;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x002E01D5;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x002E2ED6;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x000B2ED7;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x002E2EE6;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00063902;
data_array[1] = 0x000000D8;
data_array[2] = 0x00000000;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);

data_array[0] = 0x00063902;
data_array[1] = 0x000000D9;
data_array[2] = 0x00000000;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);

data_array[0] = 0x00E71500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00063902;
data_array[1] = 0x52AA55F0;
data_array[2] = 0x00000308;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x000020B0;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x000020B1;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00063902;
data_array[1] = 0x000005B2;
data_array[2] = 0x00000000;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);

data_array[0] = 0x00063902;
data_array[1] = 0x000005B6;
data_array[2] = 0x00000000;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);

data_array[0] = 0x00063902;
data_array[1] = 0x000005B7;
data_array[2] = 0x00000000;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);

data_array[0] = 0x00063902;
data_array[1] = 0x000057BA;
data_array[2] = 0x00000000;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);

data_array[0] = 0x00063902;
data_array[1] = 0x000057BB;
data_array[2] = 0x00000000;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);

data_array[0] = 0x00053902;
data_array[1] = 0x000000C0;
data_array[2] = 0x00000000;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);

data_array[0] = 0x00053902;
data_array[1] = 0x000000C1;
data_array[2] = 0x00000000;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);

data_array[0] = 0x60C41500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x40C51500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00063902;
data_array[1] = 0x52AA55F0;
data_array[2] = 0x00000508;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);

data_array[0] = 0x00063902;
data_array[1] = 0x030103BD;
data_array[2] = 0x00000303;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x000617B0;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x000617B1;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x000617B2;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x000617B3;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x000617B4;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x000617B5;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00B81500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00B91500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00BA1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x02BB1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00BC1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x07C01500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x80C41500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0xA4C51500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x003005C8;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x003101C9;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00043902;
data_array[1] = 0x3C0000CC;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00043902;
data_array[1] = 0x3C0000CD;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00063902;
data_array[1] = 0x090500D1;
data_array[2] = 0x00001007;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);

data_array[0] = 0x00063902;
data_array[1] = 0x0E0500D2;
data_array[2] = 0x00001007;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);

data_array[0] = 0x06E51500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x06E61500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x06E71500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x06E81500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x06E91500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x06EA1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x30ED1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00351500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00110500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(120);

data_array[0] = 0x00290500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(20);
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
		
    //#if (LCM_DSI_CMD_MODE)
	//	params->dsi.mode   = CMD_MODE;
    //#else
		params->dsi.mode   =BURST_VDO_MODE;// BURST_VDO_MODE;	//SYNC_EVENT_VDO_MODE;		//SYNC_PULSE_VDO_MODE;
    //#endif
	
		// DSI
		/* Command mode setting */
		// Three lane or Four lane
		params->dsi.LANE_NUM			= LCM_FOUR_LANE;
		
		//The following defined the fomat for data coming from LCD engine.
//		params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
//		params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
//		params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

		// Highly depends on LCD driver capability.
		// Not support in MT6573
//		params->dsi.packet_size=256;

		// Video mode setting		
//		params->dsi.intermediat_buffer_num = 0;

		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
//		params->dsi.word_count=FRAME_WIDTH*3;
		
		
		params->dsi.vertical_sync_active				= 6;
		params->dsi.vertical_backporch					= 20;
		params->dsi.vertical_frontporch 				= 20;
		params->dsi.vertical_active_line				= FRAME_HEIGHT;

		params->dsi.horizontal_sync_active				= 20;
		params->dsi.horizontal_backporch				= 80;
		params->dsi.horizontal_frontporch				= 80;

		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
		
 
		
		params->dsi.PLL_CLOCK = 300;//200
	//	params->dsi.cont_clock=0;
	//	params->dsi.noncont_clock = 1;						//fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)
	//    params->dsi.noncont_clock_period = 2; // Unit : frames


		
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

LCM_DRIVER c716_zs18032901_dsi_wxga_lcm_drv={
	.name				="c716_zs18032901_dsi_wxga",
	.set_util_funcs	=lcm_set_util_funcs,
	.get_params			=lcm_get_params,
	.init				=lcm_init,
	.suspend			=lcm_suspend,
	.resume				=lcm_resume,
	.suspend_power 	= lcm_suspend_power,
	.resume_power 		= lcm_resume_power,
	.init_power 		= lcm_init_power,
	
};

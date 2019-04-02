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

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))

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

static int lcm_vgp_supply_enable_r(void)
{
        int ret;
        unsigned int volt;

        printk("LCM: lcm_vgp_supply_enable\n");
        if (NULL == lcm_vgp){
			printk("++++++++++lxl++++++++++++++++++\n");
            return 0;
        }
       // printk("LCM: set regulator voltage lcm_vgp voltage to 1.8V\n");
       // /* set voltage to 1.8V */
        ret = regulator_set_voltage(lcm_vgp, 3000000, 3000000);
		//MDELAY(20);
        if (ret != 0) {
                printk("LCM: lcm failed to set lcm_vgp voltage: %d\n", ret);
              return ret;
       }

        /* get voltage settings again */
		//printk("==========lxl================\n");
        volt = regulator_get_voltage(lcm_vgp);
		MDELAY(20);
        if (volt == 3000000)
               printk("LCM: check regulator voltage=3000000 pass!\n");
        else
               printk("LCM: check regulator voltage=3000000 fail! (voltage: %d)\n", volt);

        ret = regulator_enable(lcm_vgp);
		MDELAY(15);
		printk("&&&&&&&&&lxl&&&&&&&&&&&&&&\n");
        if (ret != 0) {
                printk("LCM: Failed to enable lcm_vgp: %d\n", ret);
                return ret;
        }

        return ret;
}

static int lcm_vgp_supply_disable_r(void)
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


static void lcd_power_en(unsigned char enabled)
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
		lcm_vgp_supply_enable_r();

    }
    else
    {   
        lcm_set_gpio_output(GPIO_LCM_RST, 0);
		lcm_vgp_supply_disable_r();
    }

    
}

static void init_lcm_registers(void)
{		
    unsigned int data_array[16];
    
data_array[0] = 0x00053902;
data_array[1] = 0x2555aaFF;
data_array[2] = 0x00000001;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);

data_array[0] = 0x166F1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x10F71500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00053902;
data_array[1] = 0x2555aaFF;
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
data_array[1] = 0x000180BC;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x000180BD;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00CA1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x04C01500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x35BE1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x002D2DB3;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x001919B4;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x004646B9;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x002626BA;
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

data_array[0] = 0x00113902;
data_array[1] = 0x000000B0;
data_array[2] = 0x00390018;
data_array[3] = 0x006D0053;
data_array[4] = 0x00B00092;
data_array[5] = 0x000000E2;
dsi_set_cmdq(data_array, 6, 1);
MDELAY(1);

data_array[0] = 0x00113902;
data_array[1] = 0x010A01B1;
data_array[2] = 0x017C014A;
data_array[3] = 0x020C02CC;
data_array[4] = 0x024A020E;
data_array[5] = 0x0000008D;
dsi_set_cmdq(data_array, 6, 1);
MDELAY(1);

data_array[0] = 0x00113902;
data_array[1] = 0x02B802B2;
data_array[2] = 0x031B03F3;
data_array[3] = 0x03740351;
data_array[4] = 0x03BC03A0;
data_array[5] = 0x000000DC;
dsi_set_cmdq(data_array, 6, 1);
MDELAY(1);

data_array[0] = 0x00053902;
data_array[1] = 0x03FA03B3;
data_array[2] = 0x000000FF;
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

data_array[0] = 0x116F1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x01F31500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0xFF511500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x2C531500;
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

	params->physical_height = 173;
	params->physical_width	= 108;
	

	params->dsi.mode   =BURST_VDO_MODE; //SYNC_PULSE_VDO_MODE;//BURST_VDO_MODE; 

	
	// DSI
	/* Command mode setting */
	params->dsi.LANE_NUM				= LCM_THREE_LANE; //LCM_FOUR_LANE;
	//The following defined the fomat for data coming from LCD engine.
	params->dsi.data_format.format		= LCM_DSI_FORMAT_RGB888;

	// Highly depends on LCD driver capability.
	// Not support in MT6573
	params->dsi.packet_size=256;

	// Video mode setting		
	params->dsi.intermediat_buffer_num = 2; //because DSI/DPI HW design change, this parameters should be 0 when video mode in MT658X; or memory leakage

	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
	
	params->dsi.word_count=FRAME_WIDTH*3; 

	

	params->dsi.vertical_sync_active 			    = 2; // 2; 
    params->dsi.vertical_backporch				    =14; //20;
    params->dsi.vertical_frontporch 				=16; // 20;
    params->dsi.vertical_active_line				= FRAME_HEIGHT; 

    params->dsi.horizontal_sync_active			    = 2; //10;
    params->dsi.horizontal_backporch				= 42; //80;
    params->dsi.horizontal_frontporch				= 44; //80;
	params->dsi.horizontal_active_pixel = FRAME_WIDTH;

	params->dsi.PLL_CLOCK = 300;//256;	// 256  251 


}


static void lcm_init(void)
{
   
    printk("=====lxl(lcm_init)=====\n");

    lcd_reset(0);
    lcd_power_en(0);
    MDELAY(10);
    lcd_power_en(1);
    MDELAY(20);//Must > 5ms
    lcd_reset(1);
    MDELAY(10);//Must > 5ms
    lcd_reset(0);
    MDELAY(10);//Must > 5ms
    lcd_reset(1);
    MDELAY(10);

	init_lcm_registers();
	//MDELAY(80);
	
}

static void lcm_suspend_jd9364(void)
{
    MDELAY(10);
    lcd_reset(0);
    MDELAY(10);
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
    lcd_power_en(0);
    MDELAY(10);
}

static void lcm_resume_power(void)
{
	  
    lcd_reset(0);
    lcd_power_en(0);
    MDELAY(50);
    lcd_power_en(1);
    MDELAY(50);//Must > 5ms
    lcd_reset(1);
    MDELAY(50);//Must > 5ms
    lcd_reset(0);
    MDELAY(50);//Must > 5ms
    lcd_reset(1);
	MDELAY(100);
}

static void lcm_init_power(void){

}
       
LCM_DRIVER c805_pxjt_nt35521s_inx_wxga_ips_101_lcm_drv= 
{
	.name			= "c805_pxjt_nt35521s_inx_wxga_ips_101",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend			=lcm_suspend_jd9364,
	.resume				=lcm_resume_jd9364,
	.suspend_power 	= lcm_suspend_power,
	.resume_power 		= lcm_resume_power,
	.init_power 		= lcm_init_power,
	
};

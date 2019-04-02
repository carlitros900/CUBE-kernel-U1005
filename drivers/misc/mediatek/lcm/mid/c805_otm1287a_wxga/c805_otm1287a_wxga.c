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
#include <mt-plat/upmu_common.h>

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
		pmic_config_interface(DIGLDO_CON29, 0x6, PMIC_RG_VGP2_VOSEL_MASK, PMIC_RG_VGP2_VOSEL_SHIFT);
		pmic_config_interface(DIGLDO_CON8, 0x1, PMIC_RG_VGP2_EN_MASK, PMIC_RG_VGP2_EN_SHIFT);	
		
		lcm_set_gpio_output(GPIO_LCM_RST, GPIO_OUT_ONE);
        MDELAY(20);

    }
    else
    {   
		pmic_config_interface(DIGLDO_CON29, 0x0, PMIC_RG_VGP2_VOSEL_MASK, PMIC_RG_VGP2_VOSEL_SHIFT);
		pmic_config_interface(DIGLDO_CON8, 0x0, PMIC_RG_VGP2_EN_MASK, PMIC_RG_VGP2_EN_SHIFT);	
		
        lcm_set_gpio_output(GPIO_LCM_RST, 0);
        MDELAY(20);      
    }

    
}

struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};

static void init_lcm_registers(void)
{
	unsigned int data_array[16];
data_array[0] = 0x00001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00043902;
data_array[1] = 0x018712FF;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x80001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x008712FF;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x92001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x000230FF;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0xA6001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x08B31500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x90001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00B31500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00053902;
data_array[1] = 0x0300002A;
data_array[2] = 0x0000001F;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);

data_array[0] = 0x92001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x40B31500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x80001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x01F61500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x80001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x25C11500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x92001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00C41500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x92001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x02B31500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x90001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0xB6B61500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x80001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x000a3902;
data_array[1] = 0x006F00C0;
data_array[2] = 0x64001010;
data_array[3] = 0x00001010;
dsi_set_cmdq(data_array, 4, 1);
MDELAY(1);

data_array[0] = 0x90001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00073902;
data_array[1] = 0x005C00C0;
data_array[2] = 0x00040001;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);

data_array[0] = 0xA2001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00043902;
data_array[1] = 0x000001C0;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0xB3001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x005500C0;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x81001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x55C11500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0xA0001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x000f3902;
data_array[1] = 0x041005C4;
data_array[2] = 0x11150502;
data_array[3] = 0x02071005;
data_array[4] = 0x00111505;
dsi_set_cmdq(data_array, 5, 1);
MDELAY(1);

data_array[0] = 0xB0001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x000000C4;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x91001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x00A24BC5;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x007C7CD8;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x00001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x59D91500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0xB3001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x84C51500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x97001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0xDDC51500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0xBB001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x8AC51500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x82001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x0AC41500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0xC6001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x03B01500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0xC2001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x40F51500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0xC3001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x85F51500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x87001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x18C41500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x40D01500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x000000D1;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0xB2001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x000000F5;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0xB6001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x000000F5;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0x94001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x000000F5;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0xD2001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00033902;
data_array[1] = 0x001506F5;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(1);

data_array[0] = 0xB4001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0xCCC51500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x90001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00053902;
data_array[1] = 0x021102F5;
data_array[2] = 0x00000015;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);

data_array[0] = 0x90001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x50C51500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x94001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x66C51500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x80001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x000c3902;
data_array[1] = 0x000000CB;
data_array[2] = 0x00000000;
data_array[3] = 0x00000000;
dsi_set_cmdq(data_array, 4, 1);
MDELAY(1);

data_array[0] = 0x90001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00103902;
data_array[1] = 0x000000CB;
data_array[2] = 0x00000000;
data_array[3] = 0x00000000;
data_array[4] = 0x00000000;
dsi_set_cmdq(data_array, 5, 1);
MDELAY(1);

data_array[0] = 0xA0001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00103902;
data_array[1] = 0x00FFFFCB;
data_array[2] = 0x00000000;
data_array[3] = 0x00000000;
data_array[4] = 0x00000000;
dsi_set_cmdq(data_array, 5, 1);
MDELAY(1);

data_array[0] = 0xB0001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00103902;
data_array[1] = 0x000000CB;
data_array[2] = 0x00000000;
data_array[3] = 0x0000FFFF;
data_array[4] = 0x00000000;
dsi_set_cmdq(data_array, 5, 1);
MDELAY(1);

data_array[0] = 0xC0001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00103902;
data_array[1] = 0x050505CB;
data_array[2] = 0x05050505;
data_array[3] = 0x00000005;
data_array[4] = 0x00000000;
dsi_set_cmdq(data_array, 5, 1);
MDELAY(1);

data_array[0] = 0xD0001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00103902;
data_array[1] = 0x050505CB;
data_array[2] = 0x00000505;
data_array[3] = 0x05050505;
data_array[4] = 0x05050505;
dsi_set_cmdq(data_array, 5, 1);
MDELAY(1);

data_array[0] = 0xE0001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x000f3902;
data_array[1] = 0x000000CB;
data_array[2] = 0x00000000;
data_array[3] = 0x05050505;
data_array[4] = 0x00000005;
dsi_set_cmdq(data_array, 5, 1);
MDELAY(1);

data_array[0] = 0xF0001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x000c3902;
data_array[1] = 0xFFFFFFCB;
data_array[2] = 0xFFFFFFFF;
data_array[3] = 0xFFFFFFFF;
dsi_set_cmdq(data_array, 4, 1);
MDELAY(1);

data_array[0] = 0x80001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00103902;
data_array[1] = 0x0B0909CC;
data_array[2] = 0x0F0D0D0B;
data_array[3] = 0x0000000F;
data_array[4] = 0x00000000;
dsi_set_cmdq(data_array, 5, 1);
MDELAY(1);

data_array[0] = 0x90001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00103902;
data_array[1] = 0x2A2929CC;
data_array[2] = 0x0000012A;
data_array[3] = 0x0B0B0909;
data_array[4] = 0x0F0F0D0D;
dsi_set_cmdq(data_array, 5, 1);
MDELAY(1);

data_array[0] = 0xA0001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x000f3902;
data_array[1] = 0x000000CC;
data_array[2] = 0x00000000;
data_array[3] = 0x2A2A2929;
data_array[4] = 0x00000001;
dsi_set_cmdq(data_array, 5, 1);
MDELAY(1);

data_array[0] = 0xB0001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00103902;
data_array[1] = 0x0B0909CC;
data_array[2] = 0x0F0D0D0B;
data_array[3] = 0x0000000F;
data_array[4] = 0x00000000;
dsi_set_cmdq(data_array, 5, 1);
MDELAY(1);

data_array[0] = 0xC0001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00103902;
data_array[1] = 0x2A2929CC;
data_array[2] = 0x0000012A;
data_array[3] = 0x0B0B0909;
data_array[4] = 0x0F0F0D0D;
dsi_set_cmdq(data_array, 5, 1);
MDELAY(1);

data_array[0] = 0xD0001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x000f3902;
data_array[1] = 0x000000CC;
data_array[2] = 0x00000000;
data_array[3] = 0x2A2A2929;
data_array[4] = 0x00000001;
dsi_set_cmdq(data_array, 5, 1);
MDELAY(1);

data_array[0] = 0x80001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x000d3902;
data_array[1] = 0x2A0182CE;
data_array[2] = 0x00000181;
data_array[3] = 0x00000000;
data_array[4] = 0x00000000;
dsi_set_cmdq(data_array, 5, 1);
MDELAY(1);

data_array[0] = 0x90001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x000f3902;
data_array[1] = 0x000000CE;
data_array[2] = 0x00000000;
data_array[3] = 0x00000000;
data_array[4] = 0x00000000;
dsi_set_cmdq(data_array, 5, 1);
MDELAY(1);

data_array[0] = 0xA0001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x000f3902;
data_array[1] = 0x050118CE;
data_array[2] = 0x00400000;
data_array[3] = 0x00050018;
data_array[4] = 0x00000000;
dsi_set_cmdq(data_array, 5, 1);
MDELAY(1);

data_array[0] = 0xB0001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x000f3902;
data_array[1] = 0x050018CE;
data_array[2] = 0x00400001;
data_array[3] = 0x02050010;
data_array[4] = 0x00000000;
dsi_set_cmdq(data_array, 5, 1);
MDELAY(1);

data_array[0] = 0xC0001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x000f3902;
data_array[1] = 0x050010CE;
data_array[2] = 0x00400002;
data_array[3] = 0x04050110;
data_array[4] = 0x00000000;
dsi_set_cmdq(data_array, 5, 1);
MDELAY(1);

data_array[0] = 0xD0001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x000f3902;
data_array[1] = 0x050110CE;
data_array[2] = 0x00400003;
data_array[3] = 0x06050210;
data_array[4] = 0x00000000;
dsi_set_cmdq(data_array, 5, 1);
MDELAY(1);

data_array[0] = 0x80001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x000f3902;
data_array[1] = 0x000000CF;
data_array[2] = 0x00000000;
data_array[3] = 0x00000000;
data_array[4] = 0x00000000;
dsi_set_cmdq(data_array, 5, 1);
MDELAY(1);

data_array[0] = 0x90001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x000f3902;
data_array[1] = 0x000000CF;
data_array[2] = 0x00000000;
data_array[3] = 0x00000000;
data_array[4] = 0x00000000;
dsi_set_cmdq(data_array, 5, 1);
MDELAY(1);

data_array[0] = 0xA0001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x000f3902;
data_array[1] = 0x000000CF;
data_array[2] = 0x00000000;
data_array[3] = 0x00000000;
data_array[4] = 0x00000000;
dsi_set_cmdq(data_array, 5, 1);
MDELAY(1);

data_array[0] = 0xB0001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x000f3902;
data_array[1] = 0x000000CF;
data_array[2] = 0x00000000;
data_array[3] = 0x00000000;
data_array[4] = 0x00000000;
dsi_set_cmdq(data_array, 5, 1);
MDELAY(1);

data_array[0] = 0xC0001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x000c3902;
data_array[1] = 0x206464CF;
data_array[2] = 0x01000020;
data_array[3] = 0x00000081;
dsi_set_cmdq(data_array, 4, 1);
MDELAY(1);

data_array[0] = 0xB5001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00073902;
data_array[1] = 0xFF803FC5;
data_array[2] = 0x00FF803F;
dsi_set_cmdq(data_array, 3, 1);
MDELAY(1);

data_array[0] = 0x00001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00153902;
data_array[1] = 0x1B1100E1;
data_array[2] = 0x47453628;
data_array[3] = 0x8C785F6F;
data_array[4] = 0x5E5B8876;
data_array[5] = 0x042C3C4D;
data_array[6] = 0x00000000;
dsi_set_cmdq(data_array, 7, 1);
MDELAY(1);

data_array[0] = 0x00001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00153902;
data_array[1] = 0x1B1100E2;
data_array[2] = 0x47453628;
data_array[3] = 0x8C785F6F;
data_array[4] = 0x5E5B8876;
data_array[5] = 0x042C3C4D;
data_array[6] = 0x00000000;
dsi_set_cmdq(data_array, 7, 1);
MDELAY(1);

data_array[0] = 0x00001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00043902;
data_array[1] = 0xFFFFFFFF;
dsi_set_cmdq(data_array, 2, 1);
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
	params->dsi.mode    = BURST_VDO_MODE;

	// DSI
	/* Command mode setting */
	params->dsi.LANE_NUM				= LCM_FOUR_LANE;
	params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;
	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

	params->dsi.vertical_sync_active = 4;//4; //4;
	params->dsi.vertical_backporch =  4;//4; //14;  //16;
	params->dsi.vertical_frontporch = 16;//8; //16; //15;
	params->dsi.vertical_active_line = FRAME_HEIGHT;

	params->dsi.horizontal_sync_active = 20;//8; //4; //4;
	params->dsi.horizontal_backporch =	20; //8; //4; //42;  //50;
	params->dsi.horizontal_frontporch = 20;//40; //105; //16; //44; //60;
	params->dsi.horizontal_active_pixel = FRAME_WIDTH;


	params->dsi.PLL_CLOCK=220; //
	
}


static void lcm_init(void)
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
    MDELAY(10);
	
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
      
LCM_DRIVER c805_otm1287a_wxga_lcm_drv= 
{
	.name			= "c805_otm1287a_wxga",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend			=lcm_suspend_jd9364,
	.resume				=lcm_resume_jd9364,	
};

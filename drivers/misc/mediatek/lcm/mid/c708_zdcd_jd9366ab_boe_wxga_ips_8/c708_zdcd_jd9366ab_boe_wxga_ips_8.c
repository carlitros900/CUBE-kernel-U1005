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


// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------


static void lcd_power_en(unsigned char enabled)
{
	if (enabled)
	{
		lcm_set_gpio_output(GPIO_LCM_EN, GPIO_OUT_ONE);
		MDELAY(10);
		lcm_set_gpio_output(GPIO_LCM_PWR, GPIO_OUT_ONE);
	}
	else
	{
		lcm_set_gpio_output(GPIO_LCM_EN, 0);
		MDELAY(10);
		lcm_set_gpio_output(GPIO_LCM_PWR, 0);
	}
	//MDELAY(50); 
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

static void init_lcm_registers(void)
{
	unsigned int data_array[16];
data_array[0] = 0x00E01500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x93E11500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x65E21500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0xF8E31500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x03801500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x93781500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x66791500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0xFF7A1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0xFF7B1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x01E01500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0xB0011500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00031500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0xB0041500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00171500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0xB1181500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x01191500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x001A1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0xB11B1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x011C1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x3E1F1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x28201500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x28211500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x0E221500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x19371500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x05381500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x08391500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x123A1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x783C1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0xFF3D1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0xFF3E1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0xFF3F1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x06401500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0xA0411500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x08431500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x07441500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x40451500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x044B1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00551500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x01561500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x89571500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x0A581500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x0A591500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x285A1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x145B1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x7C5D1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x665E1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x575F1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x49601500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x46611500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x36621500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x3A631500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x23641500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x3C651500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x38661500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x37671500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x53681500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x40691500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x476A1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x386B1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x316C1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x226D1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x116E1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x006F1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x7C701500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x66711500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x57721500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x49731500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x46741500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x36751500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x3A761500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x23771500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x3C781500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x38791500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x377A1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x537B1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x407C1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x477D1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x387E1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x317F1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x22801500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x11811500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00821500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x02E01500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x47001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x47011500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x45021500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x45031500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x4B041500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x4B051500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x49061500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x49071500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x41081500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F091500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F0A1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F0B1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F0C1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F0D1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F0E1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x430F1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F101500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F111500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F121500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F131500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F141500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F151500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x46161500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x46171500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x44181500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x44191500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x4A1A1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x4A1B1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x481C1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x481D1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x401E1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F1F1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F201500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F211500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F221500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F231500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F241500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x42251500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F261500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F271500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F281500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F291500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F2A1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F2B1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x112C1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x0F2D1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x0D2E1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x0B2F1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x09301500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x07311500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x05321500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x18331500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x17341500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F351500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x01361500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F371500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F381500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F391500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F3A1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F3B1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F3C1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F3D1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F3E1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x133F1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F401500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F411500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x10421500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x0E431500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x0C441500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x0A451500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x08461500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x06471500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x04481500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x18491500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x174A1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F4B1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x004C1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F4D1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F4E1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F4F1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F501500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F511500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F521500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F531500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F541500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x12551500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F561500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F571500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x40581500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x305B1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x025C1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x405D1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x015E1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x025F1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x62631500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x62641500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x74671500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x04681500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x62691500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x666A1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x086B1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x006C1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x006D1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x006E1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x086F1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x03E01500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x3E981500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x04E01500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x14091500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x2B2B1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x442E1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00E01500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x02E61500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x06E71500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x03E01500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x049B1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00E01500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00110500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(120);

data_array[0] = 0x00290500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(20);

data_array[0] = 0x00351500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);
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
    params->dsi.LANE_NUM                = LCM_FOUR_LANE;
    params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;
    params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

    params->dsi.vertical_sync_active = 4;
    params->dsi.vertical_backporch =  4;
    params->dsi.vertical_frontporch = 8;
    params->dsi.vertical_active_line = FRAME_HEIGHT;
    params->dsi.horizontal_sync_active = 16;
    params->dsi.horizontal_backporch =  16; 
    params->dsi.horizontal_frontporch = 48;
    params->dsi.horizontal_active_pixel = FRAME_WIDTH;

    params->dsi.PLL_CLOCK=200; 

}


static void lcm_init(void)
{
   
 	lcd_reset(0);
	lcd_power_en(0);
	MDELAY(20);

	lcd_power_en(1);
	MDELAY(50);

	lcd_reset(1);
	MDELAY(50);
	lcd_reset(0);
	MDELAY(50);
	lcd_reset(1);
	MDELAY(200);

    printk("\r\n =====(init_lcm_registers start)=====\r\n");
	init_lcm_registers();
    printk("\r\n =====(init_lcm_registers end)=====\r\n");
	MDELAY(100);
}

static void lcm_suspend_jd9364(void)
{
    lcd_reset(0);
    MDELAY(10);
    lcd_power_en(0);
    MDELAY(20);
	printk("*****(lcm_suspend)**********\r\n");
}

static void lcm_resume_jd9364(void)
{
	lcm_init();
}
       
LCM_DRIVER c708_zdcd_jd9366ab_boe_wxga_ips_8_lcm_drv= 
{
	.name			= "c708_zdcd_jd9366ab_boe_wxga_ips_8",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend			=lcm_suspend_jd9364,
	.resume				=lcm_resume_jd9364,	
};

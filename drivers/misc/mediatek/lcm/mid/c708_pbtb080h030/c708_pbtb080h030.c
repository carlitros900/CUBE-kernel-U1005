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

#ifdef BUILD_LK
		printf("%s(%d), LK \n", __func__, enabled?1:0);
#else
		printk("%s(%d), kernel", __func__, enabled?1:0);
#endif

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
#ifdef BUILD_LK
        printf("%s(%d), LK \n", __func__, enabled?1:0);
#else
        printk("%s(%d), kernel", __func__, enabled?1:0);
#endif

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

data_array[0] = 0x04E01500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x032D1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x01E01500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x6F011500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x740C1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00171500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0xF2181500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00191500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x001A1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0xF21B1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x001C1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x7E1F1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x24201500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x24211500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x0E221500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x09371500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x04381500;
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

data_array[0] = 0x7F3F1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x06401500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0xA0411500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x01551500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x01561500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x65571500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x08581500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x28591500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x2B5A1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x135B1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x705D1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x5E5E1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x505F1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x45601500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x42611500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x33621500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x37631500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x21641500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x39651500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x36661500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x36671500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x54681500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x41691500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x4A6A1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x3D6B1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x3A6C1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x2F6D1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F6E1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x026F1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x70701500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x5E711500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x50721500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x45731500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x42741500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x33751500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x37761500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x21771500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x39781500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x36791500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x367A1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x547B1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x417C1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x4A7D1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x3D7E1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x3A7F1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x2F801500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F811500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x02821500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x02E01500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1E001500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F011500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x17021500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x37031500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x08041500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x0A051500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x04061500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x06071500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00081500;
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

data_array[0] = 0x020F1500;
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

data_array[0] = 0x1E161500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x1F171500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x17181500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x37191500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x091A1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x0B1B1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x051C1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x071D1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x011E1500;
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

data_array[0] = 0x03251500;
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

data_array[0] = 0x10581500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00591500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x005A1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x305B1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x055C1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x305D1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x015E1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x025F1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x30601500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x03611500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x04621500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x03631500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x6A641500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x75651500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x0D661500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x73671500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x09681500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x06691500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x6A6A1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x086B1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x006C1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x0C6D1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x046E1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x886F1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00701500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00711500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x06721500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x7B731500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00741500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x80751500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00761500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x0D771500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x24781500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x00791500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x007A1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x007B1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x007C1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x037D1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x7B7E1500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x04E01500;
dsi_set_cmdq(data_array, 1, 1);
MDELAY(1);

data_array[0] = 0x10091500;
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

data_array[0] = 0x02E71500;
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
    printk("=====lxl(lcm_get_params)=====\r\n");
    memset(params, 0, sizeof(LCM_PARAMS));

    params->type   = LCM_TYPE_DSI;

    params->width  = FRAME_WIDTH;
    params->height = FRAME_HEIGHT;

    params->dsi.mode    = BURST_VDO_MODE;
    params->dsi.LANE_NUM                = LCM_FOUR_LANE;
    params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;
    params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
    //params->dsi.word_count=800*3; 

    params->dsi.vertical_sync_active = 4;
    params->dsi.vertical_backporch =  12;
    params->dsi.vertical_frontporch = 20;
    params->dsi.vertical_active_line = FRAME_HEIGHT;
    params->dsi.horizontal_sync_active = 18;
    params->dsi.horizontal_backporch =  18; 
    params->dsi.horizontal_frontporch = 18;
    params->dsi.horizontal_active_pixel = FRAME_WIDTH;

    params->dsi.PLL_CLOCK=202; 

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

static void lcm_suspend_power(void)
{
	lcd_reset(0);
    MDELAY(10);
    lcd_power_en(0);
    MDELAY(20);
}

static void lcm_resume_power(void)
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
	MDELAY(50);
}

static void lcm_init_power(void){
  printk("=====lxl(lcm_init_power)=====\r\n");
}
       
LCM_DRIVER c708_pbtb080h030_lcm_drv= 
{
	.name			= "c708_pbtb080h030",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend			=lcm_suspend_jd9364,
	.resume				=lcm_resume_jd9364,
	.suspend_power 	= lcm_suspend_power,
	.resume_power 		= lcm_resume_power,
	.init_power 		= lcm_init_power,
	
};

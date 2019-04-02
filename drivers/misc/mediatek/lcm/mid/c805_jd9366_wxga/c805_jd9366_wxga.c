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

    }
    else
    {   
        lcm_set_gpio_output(GPIO_LCM_RST, 0);     
    }

    
}

static void init_lcm_registers(void)
{       

	unsigned int data_array[16];

    data_array[0] = 0x00E01500;
    dsi_set_cmdq(data_array, 1, 1);
    
    
    data_array[0] = 0x93E11500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x65E21500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0xF8E31500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x03801500;
    dsi_set_cmdq(data_array, 1, 1);
    
    
    data_array[0] = 0x10701500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x13711500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x06721500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x03751500;
    dsi_set_cmdq(data_array, 1, 1);
    
    
    data_array[0] = 0x01E01500;
    dsi_set_cmdq(data_array, 1, 1);
    
    
    data_array[0] = 0x00001500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x5A011500;
    dsi_set_cmdq(data_array, 1, 1);
    
    data_array[0] = 0x00031500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x58041500;
    dsi_set_cmdq(data_array, 1, 1);
    
    
    data_array[0] = 0x00171500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0xEF181500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x01191500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x001A1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0xEF1B1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x011C1500;
    dsi_set_cmdq(data_array, 1, 1);
                    
    
    data_array[0] = 0x7A1F1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x24201500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x24211500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x4E221500;
    dsi_set_cmdq(data_array, 1, 1);
                      
    
    data_array[0] = 0x59371500;
    dsi_set_cmdq(data_array, 1, 1);
    
    data_array[0] = 0x2C351500;
    dsi_set_cmdq(data_array, 1, 1);
    
    
    data_array[0] = 0x05381500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x08391500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x103A1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x883C1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0xFF3D1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0xFF3E1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x7F3F1500;
    dsi_set_cmdq(data_array, 1, 1);
    
    
    data_array[0] = 0x06401500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0xA0411500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x14431500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x0F441500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x24451500;
    dsi_set_cmdq(data_array, 1, 1);
    
    
    data_array[0] = 0x0F551500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x01561500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x89571500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x0A581500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x0A591500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x395A1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x105B1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x165C1500;
    dsi_set_cmdq(data_array, 1, 1);
    
    
    data_array[0] = 0x7A5D1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x655E1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x565F1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x49601500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x43611500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x33621500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x35631500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1C641500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x33651500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x30661500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x2E671500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x4A681500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x36691500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x3D6A1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x2F6B1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x2D6C1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x236D1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x156E1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x046F1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x7A701500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x65711500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x56721500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x49731500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x43741500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x33751500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x35761500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1C771500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x33781500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x30791500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x2E7A1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x4A7B1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x367C1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x3D7D1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x2F7E1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x2D7F1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x23801500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x15811500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x04821500;
    dsi_set_cmdq(data_array, 1, 1);
    
    
    data_array[0] = 0x02E01500;
    dsi_set_cmdq(data_array, 1, 1);
    
    
    data_array[0] = 0x1E001500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F011500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x57021500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x58031500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x48041500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x4A051500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x44061500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x46071500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x40081500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F091500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F0A1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F0B1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F0C1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F0D1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F0E1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x420F1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F101500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F111500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F121500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F131500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F141500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F151500;
    dsi_set_cmdq(data_array, 1, 1);
    
    
    data_array[0] = 0x1E161500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F171500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x57181500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x58191500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x491A1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x4B1B1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x451C1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x471D1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x411E1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F1F1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F201500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F211500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F221500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F231500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F241500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x43251500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F261500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F271500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F281500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F291500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F2A1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F2B1500;
    dsi_set_cmdq(data_array, 1, 1);
                          
    
    data_array[0] = 0x1F2C1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1E2D1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x172E1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x182F1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x07301500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x05311500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x0B321500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x09331500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x03341500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F351500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F361500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F371500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F381500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F391500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F3A1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x013B1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F3C1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F3D1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F3E1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F3F1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F401500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F411500;
    dsi_set_cmdq(data_array, 1, 1);
     
    
    data_array[0] = 0x1F421500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1E431500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x17441500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x18451500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x06461500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x04471500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x0A481500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x08491500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x024A1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F4B1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F4C1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F4D1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F4E1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F4F1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F501500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x00511500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F521500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F531500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F541500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F551500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F561500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F571500;
    dsi_set_cmdq(data_array, 1, 1);
    
     
    data_array[0] = 0x40581500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x00591500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x005A1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x305B1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x055C1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x305D1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x015E1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x025F1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x30601500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x03611500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x04621500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x6A631500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x6A641500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x75651500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x0D661500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x73671500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x09681500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x6A691500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x6A6A1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x086B1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x006C1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x046D1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x006E1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x886F1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x00701500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x00711500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x06721500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x7B731500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x00741500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0xBC751500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x00761500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x0D771500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x23781500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x00791500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x007A1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x007B1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x007C1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x037D1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x7B7E1500;
    dsi_set_cmdq(data_array, 1, 1);
    
    
    data_array[0] = 0x04E01500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x2B2B1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x442E1500;
    dsi_set_cmdq(data_array, 1, 1);
    
    
    data_array[0] = 0x00E01500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x02E61500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x02E71500;
    dsi_set_cmdq(data_array, 1, 1);
    
    data_array[0] = 0x00110500;
    dsi_set_cmdq(data_array, 1, 1);
    MDELAY(120);
    
    data_array[0] = 0x00290500;
    dsi_set_cmdq(data_array, 1, 1);
    MDELAY(5);
    
    
    data_array[0] = 0x00351500;
    dsi_set_cmdq(data_array, 1, 1);

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
	params->dsi.vertical_backporch =  12;//4; //14;  //16;
	params->dsi.vertical_frontporch = 20;//8; //16; //15;
	params->dsi.vertical_active_line = FRAME_HEIGHT;

	params->dsi.horizontal_sync_active = 20;//8; //4; //4;
	params->dsi.horizontal_backporch =	20; //8; //4; //42;  //50;
	params->dsi.horizontal_frontporch = 30;//40; //105; //16; //44; //60;
	params->dsi.horizontal_active_pixel = FRAME_WIDTH;

	params->dsi.PLL_CLOCK=225; 
	
    params->dsi.cont_clock = 1;

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
    MDELAY(10);
    lcd_power_en(1);
    MDELAY(20);//Must > 5ms
    lcd_reset(1);
    MDELAY(10);//Must > 5ms
    lcd_reset(0);
    MDELAY(10);//Must > 5ms
    lcd_reset(1);
	MDELAY(20);
}

static void lcm_init_power(void){

}
       
LCM_DRIVER c805_jd9366_wxga_lcm_drv= 
{
	.name			= "c805_jd9366_wxga",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend			=lcm_suspend_jd9364,
	.resume				=lcm_resume_jd9364,
	.suspend_power 	= lcm_suspend_power,
	.resume_power 		= lcm_resume_power,
	.init_power 		= lcm_init_power,
	
};

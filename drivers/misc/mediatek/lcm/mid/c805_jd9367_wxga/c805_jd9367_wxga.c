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

#ifdef BUILD_LK
		printf("%s(%d), LK \n", __func__, enabled?1:0);
#else
		printk("%s(%d), kernel", __func__, enabled?1:0);
#endif

	if (enabled)
	{
	    lcm_set_gpio_output(GPIO_LCM_PWR, GPIO_OUT_ONE);
        MDELAY(20);

	}
	else
	{	
		lcm_set_gpio_output(GPIO_LCM_PWR, 0);
        MDELAY(20);      
	}

	
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
    
    data_array[0] = 0x93E11500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x65E21500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0xF8E31500;
    dsi_set_cmdq(data_array, 1, 1);
    
    
    data_array[0] = 0x00E01500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x10701500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x13711500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x06721500;
    dsi_set_cmdq(data_array, 1, 1);
    
    data_array[0] = 0x02E61500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x02E71500;
    dsi_set_cmdq(data_array, 1, 1);
    
    data_array[0] = 0x01E01500;
    dsi_set_cmdq(data_array, 1, 1);
    
    data_array[0] = 0x00001500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x7F011500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x00031500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x7F041500;
    dsi_set_cmdq(data_array, 1, 1);
    
    data_array[0] = 0x00171500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0xD7181500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x05191500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x001A1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0xD71B1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x051C1500;
    dsi_set_cmdq(data_array, 1, 1);
                    
    data_array[0] = 0x791F1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x2D201500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x2D211500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x0E221500;
    dsi_set_cmdq(data_array, 1, 1);
    
    data_array[0] = 0xF8241500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0xF1261500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x09371500;
    dsi_set_cmdq(data_array, 1, 1);
    
    data_array[0] = 0x04381500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x0C391500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x183A1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x783C1500;
    dsi_set_cmdq(data_array, 1, 1);
    
    data_array[0] = 0x06401500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0xA0411500;
    dsi_set_cmdq(data_array, 1, 1);
    
    data_array[0] = 0x01551500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x01561500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0xA9571500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x0A581500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x2A591500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x375A1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x195B1500;
    dsi_set_cmdq(data_array, 1, 1);
    
    data_array[0] = 0x705D1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x505E1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x3F5F1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x31601500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x2D611500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1D621500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x22631500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x0C641500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x25651500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x24661500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x24671500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x41681500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x2F691500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x366A1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x286B1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x266C1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1C6D1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x086E1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x026F1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x70701500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x50711500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x3F721500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x31731500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x2D741500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1D751500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x22761500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x0C771500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x25781500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x24791500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x247A1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x417B1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x2F7C1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x367D1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x287E1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x267F1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1C801500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x08811500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x02821500;
    dsi_set_cmdq(data_array, 1, 1);
    
    data_array[0] = 0x02E01500;
    dsi_set_cmdq(data_array, 1, 1);
    
    data_array[0] = 0x00001500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x04011500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x06021500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x08031500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x0A041500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x0C051500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x0E061500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x17071500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x37081500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F091500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x100A1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F0B1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F0C1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F0D1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F0E1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F0F1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F101500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F111500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F121500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x12131500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F141500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F151500;
    dsi_set_cmdq(data_array, 1, 1);
    
    data_array[0] = 0x01161500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x05171500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x07181500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x09191500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x0B1A1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x0D1B1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x0F1C1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x171D1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x371E1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F1F1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x11201500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F211500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F221500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F231500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F241500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F251500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F261500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F271500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F281500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x13291500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F2A1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x1F2B1500;
    dsi_set_cmdq(data_array, 1, 1);
    
    data_array[0] = 0x10581500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x00591500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x005A1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x105B1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x075C1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x305D1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x005E1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x005F1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x30601500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x03611500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x04621500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x03631500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x6A641500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x75651500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x0D661500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0xB3671500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x09681500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x06691500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x6A6A1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x046B1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x006C1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x046D1500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0] = 0x046E1500;
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
    data_array[0] = 0x2C781500;
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
    
    data_array[0] = 0x00110500;
    dsi_set_cmdq(data_array, 1, 1);
    MDELAY(120);
    
    data_array[0] = 0x00290500;
    dsi_set_cmdq(data_array, 1, 1);
    MDELAY(5);

    }

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}

static void lcm_get_params(LCM_PARAMS *params)
{
    printk("=====lxl(lcm_get_params)=====\n");
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

    params->dsi.horizontal_sync_active = 20;
    params->dsi.horizontal_backporch =  20; 
    params->dsi.horizontal_frontporch = 30;
    params->dsi.horizontal_active_pixel = FRAME_WIDTH;

    params->dsi.PLL_CLOCK=225; 
    
#if defined(BUILD_LK)
    printf("[LCD] PLL_CLOCK=%d\n",params->dsi.PLL_CLOCK);
#else
    printk("[LCD] PLL_CLOCK=%d\n",params->dsi.PLL_CLOCK);
#endif
    params->dsi.cont_clock = 1;

}


static void lcm_init(void)
{
   
    printk("=====lxl(lcm_init)=====\n");

    lcd_reset(0);
    lcd_power_en(0);
    lcd_power_en(1);
    MDELAY(20);//Must > 5ms
    lcd_reset(1);
    MDELAY(10);//Must > 5ms
    lcd_reset(0);
    MDELAY(10);//Must > 5ms
    lcd_reset(1);
    MDELAY(10);
	//push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
	init_lcm_registers();
	MDELAY(80);
	
}

static void lcm_suspend_jd9367(void)
{
    lcd_reset(0);
    MDELAY(10);
    lcd_power_en(0);
    MDELAY(20);
	printk("*****lxl(lcm_suspend)**********\n");
}

static void lcm_resume_jd9367(void)
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
  printk("=====lxl(lcm_init_power)=====\n");
}
       
LCM_DRIVER c805_jd9367_wxga_lcm_drv= 
{
	.name			= "c805_jd9367_wxga",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend			=lcm_suspend_jd9367,
	.resume				=lcm_resume_jd9367,
	.suspend_power 	= lcm_suspend_power,
	.resume_power 		= lcm_resume_power,
	.init_power 		= lcm_init_power,
	
};

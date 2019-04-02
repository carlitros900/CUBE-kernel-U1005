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
/*#include "ddp_irq.h"*/

#ifdef BUILD_LK

#ifdef GPIO_LCM_PWR
#define GPIO_LCD_PWR      GPIO_LCM_PWR
#else
#define GPIO_LCD_PWR      0xFFFFFFFF
#endif

#ifdef GPIO_LCM_RST
#define GPIO_LCD_RST      GPIO_LCM_RST
#else
#define GPIO_LCD_RST      0xFFFFFFFF
#endif

static void lcm_set_gpio_output(unsigned int GPIO, unsigned int output)
{
	mt_set_gpio_mode(GPIO, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO, (output > 0) ? GPIO_OUT_ONE : GPIO_OUT_ZERO);
}
#else

/*static unsigned int GPIO_LCD_PWR_EN;*/
//static struct regulator *lcm_vgp;
extern unsigned int GPIO_LCD_PWR_EN;
extern unsigned int GPIO_LCD_RST_EN;
extern unsigned int GPIO_USB1_5V_EN;
extern unsigned int GPIO_ACCDET_EN;
extern unsigned int GPIO_LCD_ID2_DECTET;//by eric.wang
/*
static void lcm_get_gpio_infor(void)
{
	static struct device_node *node;

	node = of_find_compatible_node(NULL, NULL, "mediatek,lcm");

	GPIO_LCD_PWR_EN = of_get_named_gpio(node, "lcm_power_gpio", 0);
	GPIO_LCD_RST_EN = of_get_named_gpio(node, "lcm_reset_gpio", 0);
      GPIO_USB1_5V_EN = of_get_named_gpio(node, "lcm_usb1_power_gpio", 0);
     GPIO_ACCDET_EN = of_get_named_gpio(node, "lcm_accdet_gpio", 0);  
	 GPIO_LCD_ID2_DECTET = of_get_named_gpio(node, "lcm_id2_gpio", 0);
	 printk("lcm_get_gpio_infor in zigzag by eric.wang\n");
}
*/
static void lcm_set_gpio_output(unsigned int GPIO, unsigned int output)
{
	gpio_direction_output(GPIO, output);
	gpio_set_value(GPIO, output);
}
#if 1
static int lcm_get_gpio_input(unsigned int GPIO)
{
	int st;
	gpio_direction_input(GPIO);
	st=gpio_get_value(GPIO);
	return st;
}
#endif
/*
static int lcm_get_vgp_supply(struct device *dev)
{
	int ret;
	struct regulator *lcm_vgp_ldo;

	pr_debug("LCM: lcm_get_vgp_supply is going\n");

	lcm_vgp_ldo = devm_regulator_get(dev, "reg-lcm");
	if (IS_ERR(lcm_vgp_ldo)) {
		ret = PTR_ERR(lcm_vgp_ldo);
		dev_err(dev, "failed to get reg-lcm LDO, %d\n", ret);
		return ret;
	}

	pr_debug("LCM: lcm get supply ok.\n");

	ret = regulator_enable(lcm_vgp_ldo);

	ret = regulator_get_voltage(lcm_vgp_ldo);
	pr_debug("lcm LDO voltage = %d in LK stage\n", ret);

	lcm_vgp = lcm_vgp_ldo;

	return ret;
}

int zig_lcm_vgp_supply_enable(void)
{
	int ret;
	unsigned int volt;

	pr_debug("LCM: lcm_vgp_supply_enable\n");

	if (NULL == lcm_vgp)
		return 0;

	pr_debug("LCM: set regulator voltage lcm_vgp voltage to 1.8V\n");

	ret = regulator_set_voltage(lcm_vgp, 1800000, 1800000);
	if (ret != 0) {
		pr_err("LCM: lcm failed to set lcm_vgp voltage: %d\n", ret);
		return ret;
	}


	volt = regulator_get_voltage(lcm_vgp);
	if (volt == 1800000)
		pr_err("LCM: check regulator voltage=1800000 pass!\n");
	else
		pr_err("LCM: check regulator voltage=1800000 fail! (voltage: %d)\n", volt);

	ret = regulator_enable(lcm_vgp);
	if (ret != 0) {
		pr_err("LCM: Failed to enable lcm_vgp: %d\n", ret);
		return ret;
	}

	return ret;
}

int zig_lcm_vgp_supply_disable(void)
{
	int ret = 0;
	unsigned int isenable;

	if (NULL == lcm_vgp)
		return 0;


	isenable = regulator_is_enabled(lcm_vgp);

	pr_debug("LCM: lcm query regulator enable status[0x%d]\n", isenable);

	if (isenable) {
		ret = regulator_disable(lcm_vgp);
		if (ret != 0) {
			pr_err("LCM: lcm failed to disable lcm_vgp: %d\n", ret);
			return ret;
		}

		isenable = regulator_is_enabled(lcm_vgp);
		if (!isenable)
			pr_err("LCM: lcm regulator disable pass\n");
	}

	return ret;
}

static int lcm_probe(struct device *dev)
{
	lcm_get_vgp_supply(dev);
	lcm_get_gpio_infor();
	printk("zigzag lcm_probe by eric.wang\n");

	return 0;
}

static const struct of_device_id lcm_of_ids[] = {
	{.compatible = "mediatek,lcm",},
	{}
};

static struct platform_driver lcm_driver = {
	.driver = {
		   .name = "mtk_lcm",
		   .owner = THIS_MODULE,
		   .probe = lcm_probe,
#ifdef CONFIG_OF
		   .of_match_table = lcm_of_ids,
#endif
		   },
};

static int __init lcm_init(void)
{
	pr_notice("LCM: Register lcm driver\n");
	if (platform_driver_register(&lcm_driver)) {
		pr_err("LCM: failed to register disp driver\n");
		return -ENODEV;
	}

	return 0;
}

static void __exit lcm_exit(void)
{
	platform_driver_unregister(&lcm_driver);
	pr_notice("LCM: Unregister lcm driver done\n");
}
late_initcall(lcm_init);
module_exit(lcm_exit);
MODULE_AUTHOR("mediatek");
MODULE_DESCRIPTION("Display subsystem Driver");
MODULE_LICENSE("GPL");
*/
#endif
/* --------------------------------------------------------------------------- */
/* Local Constants */
/* --------------------------------------------------------------------------- */

#define FRAME_WIDTH  (800)
#define FRAME_HEIGHT (1280)
#define REGFLAG_DELAY           0xFE //by eric.wang
#define REGFLAG_END_OF_TABLE    0xFF   //by eric.wang
/* --------------------------------------------------------------------------- */
/* Local Variables */
/* --------------------------------------------------------------------------- */

static LCM_UTIL_FUNCS lcm_util = { 0 };

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))

/* --------------------------------------------------------------------------- */
/* Local Functions */
/* --------------------------------------------------------------------------- */
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)	lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)							lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)		lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)							lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)	lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

#define   LCM_DSI_CMD_MODE	0
struct LCM_setting_table {
        unsigned int cmd;
        unsigned char count;
        unsigned char para_list[64];
};

#if 0
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


static struct LCM_setting_table lcm_initialization_setting[] =
{
{0xE0,1,{0x00}},

//--- PASSWORD  ----//
{0xE1,1,{0x93}},
{0xE2,1,{0x65}},
{0xE3,1,{0xF8}},
{0x80,1,{0x03}}, //4lanes

//Lane select by internal reg
{0xE0,1,{0x04}},
{0x2B,1,{0x2B}},  //DSI ESD protect option
{0x2D,1,{0x03}},  //4lanes
{0x2E,1,{0x44}},  //DSI ESD protect option	

//--- Page1  ----//
{0xE0,1,{0x01}},

//Set VCOM
{0x00,1,{0x00}},
{0x01,1,{0x6A}},   //0x9A

//Set Gamma Power, VGMP,VGMN,VGSP,VGSN
{0x17,1,{0x00}},  
{0x18,1,{0xC0}},  //VGMP
{0x19,1,{0x01}},  //VGSP
{0x1A,1,{0x00}},   
{0x1B,1,{0xC0}},  //VGMN
{0x1C,1,{0x01}},  //VGSN

//Set Gate Power
{0x1F,1,{0x6A}},	//VGH_REG=15V,0x6B(15.2)
{0x20,1,{0x23}},	//VGL_REG=-10V,0x33(-13.2)
{0x21,1,{0x23}},	//VGL_REG2=-10V,0x33(-13.2)
{0x22,1,{0x0E}},	//0x3E

{0x35,1,{0x28}},

//SET RGBCYC
{0x37,1,{0x59}},	  //SS=1, BGR=1, REV=0
{0x38,1,{0x05}},	  //JDT=100 column
{0x39,1,{0x04}},	  //RGB_N_EQ1,
{0x3A,1,{0x08}},	  //RGB_N_EQ2,
{0x3B,1,{0x08}},	  //RGB_N_EQ2,
{0x3C,1,{0x7C}},	  //SET EQ3 for TE_H
{0x3D,1,{0xFF}},	  //SET CHGEN_ON, 
{0x3E,1,{0xFF}},	  //SET CHGEN_OFF, 
{0x3F,1,{0xFF}},	  //SET CHGEN_OFF2,

//Set TCON
{0x40,1,{0x06}},	//RSO 06h=800,
{0x41,1,{0xA0}},	//LN=640->1280 line

{0x43,1,{0x08}},	//VFP
{0x44,1,{0x0B}},	//VBP
{0x45,1,{0x88}},	//HBP

//SSD_Single(0x4A,0x35);  //BIST

//--- power voltage  ----//
{0x55,1,{0x0F}},	//DCDCM=1111, No output
{0x56,1,{0x01}},
{0x57,1,{0xAD}},	//VGH,0xAC
{0x58,1,{0x0A}},	//AVDD
{0x59,1,{0x0A}},	//VCL &AVEE,0xFA
{0x5A,1,{0x28}},	//VGH ,15V
{0x5B,1,{0x1E}},	//VGL,-10V 

//--- Gamma  ----//
{0x5D,1,{0x5C}},  //255
{0x5E,1,{0x40}},  //251
{0x5F,1,{0x30}},  //247
{0x60,1,{0x24}},  //243
{0x61,1,{0x21}},  //235
{0x62,1,{0x13}},  //227
{0x63,1,{0x1A}},  //211
{0x64,1,{0x07}},  //191
{0x65,1,{0x23}},  //159
{0x66,1,{0x25}},  //128
{0x67,1,{0x27}},  //96 
{0x68,1,{0x47}},  //64 
{0x69,1,{0x37}},  //44 
{0x6A,1,{0x3F}},  //28 
{0x6B,1,{0x32}},  //20 
{0x6C,1,{0x2E}},  //12 
{0x6D,1,{0x22}},  //8  
{0x6E,1,{0x11}},  //4  
{0x6F,1,{0x00}},  //0  
{0x70,1,{0x5C}},  //255 
{0x71,1,{0x40}},  //251 
{0x72,1,{0x30}},  //247 
{0x73,1,{0x24}},  //243 
{0x74,1,{0x21}},  //235 
{0x75,1,{0x13}},  //227 
{0x76,1,{0x1A}},  //211 
{0x77,1,{0x07}},  //191 
{0x78,1,{0x23}},  //159 
{0x79,1,{0x25}},  //128 
{0x7A,1,{0x27}},  //96  
{0x7B,1,{0x47}},  //64  
{0x7C,1,{0x37}},  //44  
{0x7D,1,{0x3F}},  //28  
{0x7E,1,{0x32}},  //20  
{0x7F,1,{0x2E}},  //12  
{0x80,1,{0x22}},  //8   
{0x81,1,{0x11}},  //4   
{0x82,1,{0x00}},  //0   

//Page2, for GIP
{0xE0,1,{0x02}},

//GIP_L Pin mapping
{0x00,1,{0x44}},
{0x01,1,{0x44}},
{0x02,1,{0x45}},
{0x03,1,{0x45}},
{0x04,1,{0x46}},
{0x05,1,{0x46}},
{0x06,1,{0x47}},
{0x07,1,{0x47}},
{0x08,1,{0x1D}},
{0x09,1,{0x1D}},
{0x0A,1,{0x1D}},
{0x0B,1,{0x1D}},
{0x0C,1,{0x1D}},
{0x0D,1,{0x1D}},
{0x0E,1,{0x1D}},
{0x0F,1,{0x57}},
{0x10,1,{0x57}},
{0x11,1,{0x58}},
{0x12,1,{0x58}},
{0x13,1,{0x40}},
{0x14,1,{0x55}},
{0x15,1,{0x55}},

//GIP_R Pin mapping   
{0x16,1,{0x44}},
{0x17,1,{0x44}},
{0x18,1,{0x45}},
{0x19,1,{0x45}},
{0x1A,1,{0x46}},
{0x1B,1,{0x46}},
{0x1C,1,{0x47}},
{0x1D,1,{0x47}},
{0x1E,1,{0x1D}},
{0x1F,1,{0x1D}},
{0x20,1,{0x1D}},
{0x21,1,{0x1D}},
{0x22,1,{0x1D}},
{0x23,1,{0x1D}},
{0x24,1,{0x1D}},
{0x25,1,{0x57}},
{0x26,1,{0x57}},
{0x27,1,{0x58}},
{0x28,1,{0x58}},
{0x29,1,{0x40}},
{0x2A,1,{0x55}},
{0x2B,1,{0x55}},

//GIP Timing	      
{0x58,1,{0x40}},
{0x59,1,{0x00}},
{0x5A,1,{0x00}},
{0x5B,1,{0x00}},
{0x5C,1,{0x0A}},
{0x5D,1,{0x10}},
{0x5E,1,{0x01}},
{0x5F,1,{0x02}},
{0x60,1,{0x10}},
{0x61,1,{0x01}},
{0x62,1,{0x02}},
{0x63,1,{0x0B}},
{0x64,1,{0x54}},
{0x65,1,{0x45}},
{0x66,1,{0x07}},
{0x67,1,{0x31}},
{0x68,1,{0x0B}},
{0x69,1,{0x1E}},
{0x6A,1,{0x54}},
{0x6B,1,{0x04}},
{0x6C,1,{0x00}},
{0x6D,1,{0x04}},
{0x6E,1,{0x00}},
{0x6F,1,{0x88}},
{0x70,1,{0x00}},
{0x71,1,{0x00}},
{0x72,1,{0x06}},
{0x73,1,{0x7B}},
{0x74,1,{0x00}},
{0x75,1,{0xF8}}, 
{0x76,1,{0x00}}, 
{0x77,1,{0x0D}}, 
{0x78,1,{0x14}}, 
{0x79,1,{0x00}}, 
{0x7A,1,{0x00}}, 
{0x7B,1,{0x00}}, 
{0x7C,1,{0x00}}, 
{0x7D,1,{0x03}}, 
{0x7E,1,{0x7B}},                                                                                                    

//Page0
{0xE0,1,{0x00}},
{0xE6,1,{0x02}},  //enable watch dog timer
{0xE7,1,{0x06}},  //setting watch doe timer

//SLP OUT
{0x01,0,{0x0}},
{0x11,0,{0x0}},  	// SLPOUT

{REGFLAG_DELAY, REGFLAG_DELAY, {120}},
//DISP ON
{0x01,0,{0x0}},
{0x29,0,{0x0}},  	// DSPON
{REGFLAG_DELAY, REGFLAG_DELAY, {50}},

//--- TE----//
{0x35,1,{0x00}}	
};	
#endif

#if 1
static void lcm_initial_registers(void)
{
    unsigned int data_array[16];
    
#ifdef BUILD_LK
    //printf("[IND][K] y_____0%s\n", __func__);
#else
    //printk("[IND][K] y_____1%s\n", __func__);
#endif

       data_array[0]=0x00E01500;dsi_set_cmdq(data_array, 1, 1);
       
       data_array[0]=0x93E11500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x65E21500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0xF8E31500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x03801500;dsi_set_cmdq(data_array, 1, 1);
       
       data_array[0]=0x04E01500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x2B2B1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x032D1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x442E1500;dsi_set_cmdq(data_array, 1, 1);
       
       data_array[0]=0x01E01500;dsi_set_cmdq(data_array, 1, 1);
       
       data_array[0]=0x00001500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x6A011500;dsi_set_cmdq(data_array, 1, 1);
       
       data_array[0]=0x00171500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0xC0181500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x01191500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x001A1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0xC01B1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x011C1500;dsi_set_cmdq(data_array, 1, 1);
       
       data_array[0]=0x6A1F1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x23201500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x23211500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x0E221500;dsi_set_cmdq(data_array, 1, 1);
       
       data_array[0]=0x28351500;dsi_set_cmdq(data_array, 1, 1);
       
       data_array[0]=0x59371500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x05381500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x04391500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x083A1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x083B1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x7C3C1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0xFF3D1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0xFF3E1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0xFF3F1500;dsi_set_cmdq(data_array, 1, 1);
       
       data_array[0]=0x06401500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0xA0411500;dsi_set_cmdq(data_array, 1, 1);
       
       data_array[0]=0x08431500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x0B441500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x88451500;dsi_set_cmdq(data_array, 1, 1);
       
       
       data_array[0]=0x0F551500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x01561500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0xAD571500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x0A581500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x0A591500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x285A1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x1E5B1500;dsi_set_cmdq(data_array, 1, 1);
       
       data_array[0]=0x5C5D1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x405E1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x305F1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x24601500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x21611500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x13621500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x1A631500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x07641500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x23651500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x25661500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x27671500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x47681500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x37691500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x3F6A1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x326B1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x2E6C1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x226D1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x116E1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x006F1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x5C701500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x40711500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x30721500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x24731500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x21741500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x13751500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x1A761500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x07771500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x23781500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x25791500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x277A1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x477B1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x377C1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x3F7D1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x327E1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x2E7F1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x22801500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x11811500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x00821500;dsi_set_cmdq(data_array, 1, 1);
       
       data_array[0]=0x02E01500;dsi_set_cmdq(data_array, 1, 1);
       
       data_array[0]=0x44001500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x44011500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x45021500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x45031500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x46041500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x46051500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x47061500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x47071500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x1D081500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x1D091500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x1D0A1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x1D0B1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x1D0C1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x1D0D1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x1D0E1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x570F1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x57101500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x58111500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x58121500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x40131500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x55141500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x55151500;dsi_set_cmdq(data_array, 1, 1);
       
       data_array[0]=0x44161500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x44171500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x45181500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x45191500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x461A1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x461B1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x471C1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x471D1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x1D1E1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x1D1F1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x1D201500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x1D211500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x1D221500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x1D231500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x1D241500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x57251500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x57261500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x58271500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x58281500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x40291500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x552A1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x552B1500;dsi_set_cmdq(data_array, 1, 1);
       
       data_array[0]=0x40581500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x00591500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x005A1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x005B1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x0A5C1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x105D1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x015E1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x025F1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x10601500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x01611500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x02621500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x0B631500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x54641500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x45651500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x07661500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x31671500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x0B681500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x1E691500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x546A1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x046B1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x006C1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x046D1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x006E1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x886F1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x00701500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x00711500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x06721500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x7B731500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x00741500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0xF8751500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x00761500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x0D771500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x14781500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x00791500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x007A1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x007B1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x007C1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x037D1500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x7B7E1500;dsi_set_cmdq(data_array, 1, 1);
       
       
       data_array[0]=0x00E01500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x02E61500;dsi_set_cmdq(data_array, 1, 1);
       data_array[0]=0x06E71500;dsi_set_cmdq(data_array, 1, 1);

       data_array[0] = 0x00110500;dsi_set_cmdq(data_array, 1, 1); 
       MDELAY(150);
       data_array[0] = 0x00290500;dsi_set_cmdq(data_array, 1, 1); 
       MDELAY(20);



}
#endif







static void lcm_init_power(void)
{
#ifdef BUILD_LK
	printf("[LK/LCM] lcm_init_power() enter\n");
	lcm_set_gpio_output(GPIO_LCD_PWR, GPIO_OUT_ONE);
	 lcm_set_gpio_output(GPIO_USB1_5V_EN, 1);
        lcm_set_gpio_output(GPIO_ACCDET_EN, 1);
	MDELAY(20);
	//upmu_set_rg_vgp3_vosel(3);
	//upmu_set_rg_vgp3_en(0x1);

#else
	//lcm_get_gpio_infor();//by eric.wang
	lcm_set_gpio_output(GPIO_LCD_PWR_EN, 1);
	MDELAY(10);
	pr_debug("[Kernel/LCM] lcm_init_power() enter\n");
#endif
}
#ifdef CONFIG_EARPHONE_TEMP_DETECT
int lcm2_g_is_temp_detect_plugin = 0;
extern int accdet_is_temp_detect_plugin(void);
#endif

static void lcm_suspend_power(void)
{
#ifdef BUILD_LK
	printf("[LK/LCM] lcm_suspend_power() enter\n");
	lcm_set_gpio_output(GPIO_LCD_PWR, GPIO_OUT_ZERO);
	MDELAY(20);

	//upmu_set_rg_vgp3_vosel(0);
	//upmu_set_rg_vgp3_en(0x0);

#else
	pr_debug("[Kernel/LCM] lcm_suspend_power() enter\n");
	lcm_set_gpio_output(GPIO_LCD_PWR_EN, 0);
	MDELAY(20);

	//zig_lcm_vgp_supply_disable();
	//MDELAY(20);
	lcm_set_gpio_output(GPIO_USB1_5V_EN, 0);
	lcm_set_gpio_output(GPIO_ACCDET_EN, 0);
     
#endif
}

static void lcm_resume_power(void)
{
#ifdef BUILD_LK
	printf("[LK/LCM] lcm_resume_power() enter\n");
	lcm_set_gpio_output(GPIO_LCD_PWR, GPIO_OUT_ONE);
	MDELAY(20);
	//upmu_set_rg_vgp3_vosel(3);
	//upmu_set_rg_vgp3_en(0x1);

#else
	pr_debug("[Kernel/LCM] lcm_resume_power() enter\n");
	lcm_set_gpio_output(GPIO_LCD_PWR_EN, 0);
	//MDELAY(20);

	//zig_lcm_vgp_supply_enable();
	MDELAY(20);
      lcm_set_gpio_output(GPIO_USB1_5V_EN, 1);
      lcm_set_gpio_output(GPIO_ACCDET_EN, 1);  
#endif
#ifdef CONFIG_EARPHONE_TEMP_DETECT
lcm2_g_is_temp_detect_plugin = accdet_is_temp_detect_plugin();
if (lcm2_g_is_temp_detect_plugin) {
          //  printk("temp_dectect_enable\n");
         lcm_set_gpio_output(GPIO_ACCDET_EN, 1);          

        }
        else
        	{
        	       //    printk("ear_dectect_enable\n");	
          lcm_set_gpio_output(GPIO_ACCDET_EN, 0); 	           
        		
        		}

#endif


}

/* --------------------------------------------------------------------------- */
/* LCM Driver Implementations */
/* --------------------------------------------------------------------------- */
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
		
    #if (LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
    #else
		params->dsi.mode   = BURST_VDO_MODE;//BURST_VDO_MODE;	//SYNC_EVENT_VDO_MODE;		//SYNC_PULSE_VDO_MODE;
    #endif
	
		// DSI
		/* Command mode setting */
		// Three lane or Four lane
	  params->dsi.LANE_NUM				= LCM_FOUR_LANE;//LCM_TWO_LANE;
	  //params->dsi.LANE_NUM				= LCM_THREE_LANE;
		//params->dsi.LANE_NUM				= LCM_TWO_LANE;
		//The following defined the fomat for data coming from LCD engine.
#if 1
  	//params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
  	//params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
		//params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;
#endif
		// Highly depends on LCD driver capability.
		// Not support in MT6573
		//params->dsi.packet_size=256;

		// Video mode setting		
	//params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
//	    params->dsi.intermediat_buffer_num = 0;

		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888; //mask in mt8321 platform
    //   params->dsi.word_count=FRAME_WIDTH*3;
		#if 1
		params->dsi.vertical_sync_active				= 4;//4// by eric.wang
		params->dsi.vertical_backporch					= 8;//8 by eric.wang 23
		params->dsi.vertical_frontporch					= 8;//6 by eric.wang 12 
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 

		params->dsi.horizontal_sync_active				= 4;//4
		params->dsi.horizontal_backporch				= 132;//132 by eric.wangc 160
		params->dsi.horizontal_frontporch				= 24;//24 by eric.wang 160
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
		#endif
		#if 0
		params->dsi.vertical_sync_active				= 4;//4// by eric.wang
		params->dsi.vertical_backporch					= 10;//8 by eric.wang 23
		params->dsi.vertical_frontporch					= 20;//6 by eric.wang 12 
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 

		params->dsi.horizontal_sync_active				= 18;//4
		params->dsi.horizontal_backporch				= 18;//132 by eric.wangc 160
		params->dsi.horizontal_frontporch				= 18;//24 by eric.wang 160
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
		#endif
		
		//params->dsi.horizontal_blanking_pixel = 60;

		
		// Bit rate calculation
		// Every lane speed
		//params->dsi.pll_div1=0;				// div1=0,1,2,3;div1_real=1,2,4,4
		//params->dsi.pll_div2=0;				// div2=0,1,2,3;div1_real=1,2,4,4	
		//params->dsi.fbk_div =0x12;    // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)	
		//params->dsi.cont_clock 	= 0;//0
		//params->dsi.ssc_disable = 1;//0
		params->dsi.PLL_CLOCK 	=210;//210;// 220;//245 by eric.wang
		//params->dsi.clk_lp_per_line_enable   = 1; //1
		
}

static void lcm_init_lcm(void)
{
#ifdef BUILD_LK
	printf("[LK/LCM] lcm_init() enter\n");
	lcm_set_gpio_output(GPIO_LCD_RST, GPIO_OUT_ZERO);
	lcm_set_gpio_output(GPIO_LCD_PWR, GPIO_OUT_ZERO);
	
	//lcm_set_gpio_output(GPIO_LCM_STB, GPIO_OUT_ZERO);

	MDELAY(90);
	
	lcm_set_gpio_output(GPIO_LCD_PWR, GPIO_OUT_ONE);

	MDELAY(30);
	//lcm_set_gpio_output(GPIO_LCM_STB, GPIO_OUT_ONE);
	MDELAY(30);
	
	lcm_set_gpio_output(GPIO_LCD_RST, GPIO_OUT_ONE);
	MDELAY(10);
	
	lcm_set_gpio_output(GPIO_LCD_RST, GPIO_OUT_ZERO);
	MDELAY(30);
	lcm_set_gpio_output(GPIO_LCD_RST, GPIO_OUT_ONE);
	MDELAY(120);//Must > 5ms
	//lcd_bl_en(1);
	
	lcm_initial_registers();
	//push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1); 
	//MDELAY(400);




#else
	pr_debug("[Kernel/LCM] lcm_init() enter\n");
#endif





}

static void lcm_suspend(void)
{
#ifdef BUILD_LK
	printf("[LK/LCM]anx6585 lcm_suspend() enter\n");

	lcm_set_gpio_output(GPIO_LCD_RST, 1);
	MDELAY(10);

	lcm_set_gpio_output(GPIO_LCD_RST, 0);
	MDELAY(10);
       lcm_set_gpio_output(GPIO_LCD_PWR, GPIO_OUT_ZERO);
#else
 	unsigned int data_array[16];
	printk("[Kernel/ anx6585LCM] lcm_suspend() enter\n");
	data_array[0] = 0x00280500;  //display off                        
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(20);
	data_array[0] = 0x00100500;
	dsi_set_cmdq(data_array,1,1);//enter into sleep
	MDELAY(120);


	lcm_set_gpio_output(GPIO_LCD_RST_EN, 1);
	MDELAY(10);

	lcm_set_gpio_output(GPIO_LCD_RST_EN, 0);
	MDELAY(10);
       lcm_set_gpio_output(GPIO_LCD_PWR_EN, 0);
#endif
}

static void lcm_resume(void)
{
#ifdef BUILD_LK
	printf("[LK/LCM] anx6585 lcm_resume() enter\n");

	lcm_set_gpio_output(GPIO_LCD_RST, 1);
	MDELAY(20);

	lcm_set_gpio_output(GPIO_LCD_RST, 0);
	MDELAY(20);

	lcm_set_gpio_output(GPIO_LCD_RST, 1);
	MDELAY(20);

#else
	printk("[Kernel/LCM] anx6585 lcm_resume() enter\n");

	lcm_set_gpio_output(GPIO_LCD_RST_EN, 0);
	lcm_set_gpio_output(GPIO_LCD_PWR_EN, 0);
	
	//avdd_enable(0);
//	lcm_set_gpio_output(GPIO_LCM_STB, 0);//by eric.wang
	MDELAY(90);
	
	lcm_set_gpio_output(GPIO_LCD_PWR_EN, 1);
	
	MDELAY(30);
	//lcm_set_gpio_output(GPIO_LCM_STB, 1);//by eric.wang
	MDELAY(30);
	
	lcm_set_gpio_output(GPIO_LCD_RST_EN, 1);
	MDELAY(10);
	
	lcm_set_gpio_output(GPIO_LCD_RST_EN, 0);
	MDELAY(10);
	lcm_set_gpio_output(GPIO_LCD_RST_EN, 1);
	MDELAY(130);//Must > 5ms
	//lcd_bl_en(1);
	
	lcm_initial_registers();
	//push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1); 
	MDELAY(400);
	
#endif
}

#if (LCM_DSI_CMD_MODE)
static void lcm_update(unsigned int x, unsigned int y, unsigned int width, unsigned int height)
{
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0 >> 8) & 0xFF);
	unsigned char x0_LSB = (x0 & 0xFF);
	unsigned char x1_MSB = ((x1 >> 8) & 0xFF);
	unsigned char x1_LSB = (x1 & 0xFF);
	unsigned char y0_MSB = ((y0 >> 8) & 0xFF);
	unsigned char y0_LSB = (y0 & 0xFF);
	unsigned char y1_MSB = ((y1 >> 8) & 0xFF);
	unsigned char y1_LSB = (y1 & 0xFF);

	unsigned int data_array[16];

	data_array[0] = 0x00053902;
	data_array[1] = (x1_MSB << 24) | (x0_LSB << 16) | (x0_MSB << 8) | 0x2a;
	data_array[2] = (x1_LSB);
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0] = 0x00053902;
	data_array[1] = (y1_MSB << 24) | (y0_LSB << 16) | (y0_MSB << 8) | 0x2b;
	data_array[2] = (y1_LSB);
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0] = 0x00290508;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x002c3909;
	dsi_set_cmdq(data_array, 1, 0);
}
#endif

#if 1
static unsigned int lcm_compare_id(void){
	int st;
	st=lcm_get_gpio_input(GPIO_LCD_ID2_DECTET);
	printk("guoxian lcm:st=%d,by eric.wang in kernel\n",st);
	if(st==1)		
		return 1; //guoxian lcm id2 :0	
	else		
		return 0;	

}
#endif

LCM_DRIVER es6311_anx6585_zigzag_wxga_lcm_drv = {
	.name = "es6311_anx6585_zigzag_wxga",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params = lcm_get_params,
	.init = lcm_init_lcm,
	.suspend = lcm_suspend,
	.resume = lcm_resume,
	.init_power = lcm_init_power,
	.resume_power = lcm_resume_power,
	.suspend_power = lcm_suspend_power,
	.compare_id = lcm_compare_id,
#if (LCM_DSI_CMD_MODE)
	.update = lcm_update,
#endif
};

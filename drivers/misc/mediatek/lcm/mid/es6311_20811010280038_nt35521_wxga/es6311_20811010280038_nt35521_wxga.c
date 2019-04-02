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
	 printk("lcm_get_gpio_infor in nt35521 by eric.wang\n");
        printk("luyiGPIO_ACCDET_EN = %d \n", GPIO_ACCDET_EN);
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

int lcm_vgp_supply_enable(void)
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

int lcm_vgp_supply_disable(void)
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
	printk("nt35521 lcm_probe by eric.wang\n");
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

static void lcm_initial_registers(void)
{
    unsigned int data_array[16];
    
#ifdef BUILD_LK
    //printf("[IND][K] y_____0%s\n", __func__);
#else
    //printk("[IND][K] y_____1%s\n", __func__);
#endif

/*

	data_array[0] = 0x10b21500;
	dsi_set_cmdq(&data_array, 1, 1);
	MDELAY(1);	
*/	

	//=======================================
	
data_array[0]= 0x00053902;
data_array[1]= 0xA555AAFF;
data_array[2]= 0x00000125;
dsi_set_cmdq(data_array, 3, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x000020FC;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(2);

data_array[0]= 0x00033902;
data_array[1]= 0x000000FC;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(2);

data_array[0]= 0x00033902;
data_array[1]= 0x000020FC;
dsi_set_cmdq(data_array, 2, 1);
MDELAY(2);

data_array[0]= 0x00053902;
data_array[1]= 0x2555AAFF;
data_array[2]= 0x00000000;
dsi_set_cmdq(data_array, 3, 1);

data_array[0]= 0x00063902;
data_array[1]= 0x52AA55F0;
data_array[2]= 0x00000008;
dsi_set_cmdq(data_array, 3, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x000168B1;
dsi_set_cmdq(data_array, 2, 1);


data_array[0]= 0x00023902;
data_array[1]= 0x000008B6;
dsi_set_cmdq(data_array, 2, 1);


data_array[0]= 0x00043902;
data_array[1]= 0x080201B8;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x004444BB;
dsi_set_cmdq(data_array, 2, 1);


data_array[0]= 0x00033902;
data_array[1]= 0x000000BC;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00063902;
data_array[1]= 0x106802BD;
data_array[2]= 0x00000010;
dsi_set_cmdq(data_array, 3, 1);

data_array[0]= 0x00023902;
data_array[1]= 0x000080C8;
dsi_set_cmdq(data_array, 2, 1);
//Page 1 Enable
data_array[0]= 0x00063902;
data_array[1]= 0x52AA55F0;
data_array[2]= 0x00000108;
dsi_set_cmdq(data_array, 3, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x004F4FB3;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x001010B4;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x000505B5;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x003535B9;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x002525BA;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x000068BC;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x000068BD;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00023902;
data_array[1]= 0x000033BE;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00023902;
data_array[1]= 0x00000CC0;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00023902;
data_array[1]= 0x000000CA;
dsi_set_cmdq(data_array, 2, 1);
//PAGE 2
data_array[0]= 0x00063902;
data_array[1]= 0x52AA55F0;
data_array[2]= 0x00000208;
dsi_set_cmdq(data_array, 3, 1);

data_array[0]= 0x00023902;
data_array[1]= 0x000001EE;
dsi_set_cmdq(data_array, 2, 1);


data_array[0]= 0x00113902;
data_array[1]= 0x000000B0;
data_array[2]= 0x002A000F;
data_array[3]= 0x00540040;
data_array[4]= 0x00930076;
data_array[5]= 0x000000C5;
dsi_set_cmdq(data_array, 6, 1);

data_array[0]= 0x00113902;
data_array[1]= 0x01F000B1;
data_array[2]= 0x01660132;
data_array[3]= 0x02FF01BB;
data_array[4]= 0x02420201;
data_array[5]= 0x00000085;
dsi_set_cmdq(data_array, 6, 1);

data_array[0]= 0x00113902;
data_array[1]= 0x02AF02B2;
data_array[2]= 0x030503E0;
data_array[3]= 0x03540335;
data_array[4]= 0x03A00384;
data_array[5]= 0x000000C4;
dsi_set_cmdq(data_array, 6, 1);

data_array[0]= 0x00053902;
data_array[1]= 0x03F203B3;
data_array[2]= 0x000000FF;
dsi_set_cmdq(data_array, 3, 1);
//PAGE 3
data_array[0]= 0x00063902;
data_array[1]= 0x52AA55F0;
data_array[2]= 0x00000308;
dsi_set_cmdq(data_array, 3, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x000000B0;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x000000B1;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00063902;
data_array[1]= 0x170008B2;
data_array[2]= 0x00000000;
dsi_set_cmdq(data_array, 3, 1);

data_array[0]= 0x00063902;
data_array[1]= 0x000005B6;
data_array[2]= 0x00000000;
dsi_set_cmdq(data_array, 3, 1);

data_array[0]= 0x00063902;
data_array[1]= 0xA00053BA;
data_array[2]= 0x00000000;
dsi_set_cmdq(data_array, 3, 1);

data_array[0]= 0x00063902;
data_array[1]= 0xA00053BB;
data_array[2]= 0x00000000;
dsi_set_cmdq(data_array, 3, 1);

data_array[0]= 0x00053902;
data_array[1]= 0x000000C0;
data_array[2]= 0x00000000;
dsi_set_cmdq(data_array, 3, 1);

data_array[0]= 0x00053902;
data_array[1]= 0x000000C1;
data_array[2]= 0x00000000;
dsi_set_cmdq(data_array, 3, 1);

data_array[0]= 0x00023902;
data_array[1]= 0x000060C4;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00023902;
data_array[1]= 0x0000C0C5;
dsi_set_cmdq(data_array, 2, 1);
//PAGE 5
data_array[0]= 0x00063902;
data_array[1]= 0x52AA55F0;
data_array[2]= 0x00000508;
dsi_set_cmdq(data_array, 3, 1);


data_array[0]= 0x00033902;
data_array[1]= 0x000617B0;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x000617B1;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x000617B2;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x000617B3;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x000617B4;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x000617B5;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00023902;
data_array[1]= 0x00000CB8;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00023902;
data_array[1]= 0x000000B9;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00023902;
data_array[1]= 0x000000BA;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00023902;
data_array[1]= 0x00000ABB;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00023902;
data_array[1]= 0x000002BC;
dsi_set_cmdq(data_array, 2, 1);


data_array[0]= 0x00063902;
data_array[1]= 0x010103BD;
data_array[2]= 0x00000303;
dsi_set_cmdq(data_array, 3, 1);

data_array[0]= 0x00023902;
data_array[1]= 0x000007C0;
dsi_set_cmdq(data_array, 2, 1);


data_array[0]= 0x00023902;
data_array[1]= 0x0000A2C4;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x002003C8;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x002101C9;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00043902;
data_array[1]= 0x010000CC;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00043902;
data_array[1]= 0x010000CD;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00063902;
data_array[1]= 0xFC0400D1;
data_array[2]= 0x00001407;
dsi_set_cmdq(data_array, 3, 1);

data_array[0]= 0x00063902;
data_array[1]= 0x000510D2;
data_array[2]= 0x00001603;
dsi_set_cmdq(data_array, 3, 1);



data_array[0]= 0x00023902;
data_array[1]= 0x000006E5;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00023902;
data_array[1]= 0x000006E6;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00023902;
data_array[1]= 0x000006E7;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00023902;
data_array[1]= 0x000006E8;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00023902;
data_array[1]= 0x000006E9;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00023902;
data_array[1]= 0x000006EA;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00023902;
data_array[1]= 0x000030ED;
dsi_set_cmdq(data_array, 2, 1);
//PAGE6 
data_array[0]= 0x00063902;
data_array[1]= 0x52AA55F0;
data_array[2]= 0x00000608;
dsi_set_cmdq(data_array, 3, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x001117B0;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x001016B1;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x001812B2;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x001913B3;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x003100B4;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x003431B5;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x002934B6;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x00332AB7;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x002D2EB8;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x003408B9;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x000834BA;
dsi_set_cmdq(data_array, 2, 1);


data_array[0]= 0x00033902;
data_array[1]= 0x002E2DBB;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x002A34BC;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x003429BD;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x003134BE;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x000031BF;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x001319C0;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x001218C1;
dsi_set_cmdq(data_array, 2, 1);


data_array[0]= 0x00033902;
data_array[1]= 0x001610C2;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x001711C3;
dsi_set_cmdq(data_array, 2, 1);


data_array[0]= 0x00033902;
data_array[1]= 0x003434E5;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x001812C4;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x001913C5;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x001117C6;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x001016C7;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x003108C8;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x003431C9;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x002934CA;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x00332ACB;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x002E2DCC;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x003400CD;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x000034CE;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x002D2ECF;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x002A34D0;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x003429D1;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x003134D2;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x000831D3;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x001610D4;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x001711D5;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x001319D6;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x001218D7;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00033902;
data_array[1]= 0x003434E6;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00063902;
data_array[1]= 0x000000D8;
data_array[2]= 0x00000000;
dsi_set_cmdq(data_array, 3, 1);

data_array[0]= 0x00063902;
data_array[1]= 0x000000D9;
data_array[2]= 0x00000000;
dsi_set_cmdq(data_array, 3, 1);

data_array[0]= 0x00023902;
data_array[1]= 0x000000E7;
dsi_set_cmdq(data_array, 2, 1);

data_array[0] = 0x00110500;        //exit sleep mode 
dsi_set_cmdq(data_array, 1, 1); 
MDELAY(120); 
  
data_array[0] = 0x00290500;        //exit sleep mode 
dsi_set_cmdq(data_array, 1, 1); 
MDELAY(120);

data_array[0]= 0x00023902;
data_array[1]= 0x00000035;
dsi_set_cmdq(data_array, 2, 1);

data_array[0]= 0x00063902;
data_array[1]= 0x52AA55F0;
data_array[2]= 0x00000108;
dsi_set_cmdq(data_array, 3, 1);


data_array[0] = 0x00110500;        //exit sleep mode 
dsi_set_cmdq(data_array, 1, 1); 
MDELAY(200); 
  
data_array[0] = 0x00290500;        //exit sleep mode 
dsi_set_cmdq(data_array, 1, 1); 
MDELAY(100);


}







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
int lcm_g_is_temp_detect_plugin = 0;
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

	//lcm_vgp_supply_disable();
	//MDELAY(20);
	lcm_set_gpio_output(GPIO_USB1_5V_EN, 0);
	lcm_set_gpio_output(GPIO_ACCDET_EN, 0);
     printk("luyi[LK/LCM] lcm_suspend_power() enter\n \n");
   //  lcm_set_gpio_output(GPIO_HALL_KEY_EN, 1);
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
       printk("luyi[LK/LCM] lcm_suspend_power() enter\n \n");
	pr_debug("[Kernel/LCM] lcm_resume_power() enter\n");
	lcm_set_gpio_output(GPIO_LCD_PWR_EN, 1);
	//MDELAY(20);

	//lcm_vgp_supply_enable();
	MDELAY(20);
      lcm_set_gpio_output(GPIO_USB1_5V_EN, 1);


      lcm_set_gpio_output(GPIO_ACCDET_EN, 1);  
	// lcm_set_gpio_output(GPIO_HALL_KEY_EN, 1);  
  //  gpio_direction_input(GPIO_HALL_KEY_EN);  

#endif
#ifdef CONFIG_EARPHONE_TEMP_DETECT
lcm_g_is_temp_detect_plugin = accdet_is_temp_detect_plugin();
if (lcm_g_is_temp_detect_plugin) {
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
		params->dsi.vertical_sync_active				= 5;//4// by eric.wang
		params->dsi.vertical_backporch					= 3;//8 by eric.wang 23
		params->dsi.vertical_frontporch					= 8;//6 by eric.wang 12 
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 

		params->dsi.horizontal_sync_active				= 5;//4
		params->dsi.horizontal_backporch				= 59;//132 by eric.wangc 160
		params->dsi.horizontal_frontporch				= 16;//24 by eric.wang 160
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
		#endif
		#if 0
		params->dsi.vertical_sync_active				= 5;//4// by eric.wang
		params->dsi.vertical_backporch					= 12;//8 by eric.wang 23
		params->dsi.vertical_frontporch					= 12;//6 by eric.wang 12 
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 

		params->dsi.horizontal_sync_active				= 10;//4
		params->dsi.horizontal_backporch				= 59;//132 by eric.wangc 160
		params->dsi.horizontal_frontporch				= 32;//24 by eric.wang 160
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
	MDELAY(10);
	lcm_set_gpio_output(GPIO_LCD_RST, GPIO_OUT_ONE);
	MDELAY(130);//Must > 5ms
	//lcd_bl_en(1);
	
	lcm_initial_registers();
	MDELAY(400);




#else
	pr_debug("[Kernel/LCM] lcm_init() enter\n");
#endif





}

void lcm_suspend(void)
{
#ifdef BUILD_LK
	printf("[LK/LCM]20811010280038 lcm_suspend() enter\n");

	lcm_set_gpio_output(GPIO_LCD_RST, 1);
	MDELAY(10);

	lcm_set_gpio_output(GPIO_LCD_RST, 0);
	MDELAY(10);
	 lcm_set_gpio_output(GPIO_LCD_PWR, GPIO_OUT_ZERO);
#else
	printk("[Kernel/LCM] 20811010280038lcm_suspend() enter\n");

	lcm_set_gpio_output(GPIO_LCD_RST_EN, 1);
	MDELAY(10);

	lcm_set_gpio_output(GPIO_LCD_RST_EN, 0); 
	MDELAY(10);
	lcm_set_gpio_output(GPIO_LCD_PWR_EN, 0);
#endif
}

void lcm_resume(void)
{
#ifdef BUILD_LK
	printf("[LK/LCM] 20811010280038lcm_resume() enter\n");

	lcm_set_gpio_output(GPIO_LCD_RST, 1);
	MDELAY(20);

	lcm_set_gpio_output(GPIO_LCD_RST, 0);
	MDELAY(20);

	lcm_set_gpio_output(GPIO_LCD_RST, 1);
	MDELAY(20);

#else
	printk("[Kernel/LCM]20811010280038 lcm_resume() enter\n");

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
	printk("star lcm:st=%d,by eric.wang in kernel\n",st);
	if(st==0)		
		return 1; //star lcm id2 :0	
	else		
		return 0;	

}
#endif
LCM_DRIVER es6311_20811010280038_nt35521_wxga_lcm_drv = {
	.name = "es6311_20811010280038_nt35521_wxga",
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

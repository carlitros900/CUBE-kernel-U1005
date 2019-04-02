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
 unsigned int GPIO_LCM_33V;
unsigned int GPIO_LCM_PWR;
unsigned int GPIO_LCM_RST;
unsigned int GPIO_LCM_VDD;
unsigned int GPIO_LVDS_POWER;
unsigned int GPIO_LVDS_RST;
unsigned int GPIO_LVDS_STBY;

unsigned int GPIO_IT6151_1V8;
unsigned int GPIO_IT6151_BL;
unsigned int GPIO_IT6151_STB;
unsigned int GPIO_IT6151_RST;
unsigned int GPIO_IT6151_INT;
unsigned int GPIO_VLED_EN;

struct regulator *lcm_vgp;

 unsigned int GPIO_LCD_PWR_EN;
 unsigned int GPIO_LCD_RST_EN;
 unsigned int GPIO_USB1_5V_EN;
 unsigned int GPIO_ACCDET_EN;
 unsigned int GPIO_LCD_ID2_DECTET;//by eric.wang

struct pinctrl *pinctrlv;
struct pinctrl_state *pinctrl_lcmvsy;
struct pinctrl_state *pinctrl_lcmvsy_low;
struct pinctrl_state *pinctrl_lcmvsy_high;
struct pinctrl_state *pinctrl_lcmhsy_low;
struct pinctrl_state *pinctrl_lcmhsy_high;
struct pinctrl_state *pinctrl_lcmdpi_low;
struct pinctrl_state *pinctrl_lcmdpi_high;
struct pinctrl_state *pinctrl_lcmde_low;
struct pinctrl_state *pinctrl_lcmde_high;

//---------------------------------------------------------------------------
//LocalConstants
//---------------------------------------------------------------------------
//LocalVariables
//---------------------------------------------------------------------------


//void lcm_get_gpio_infor(void)
void lcm_get_gpio_infor(struct device *dev)
{
        static struct device_node *node;

        node = of_find_compatible_node(NULL, NULL, "mediatek,lcm");
        GPIO_LCM_33V = of_get_named_gpio(node, "lcm_power_33v", 0);
        GPIO_LCM_PWR = of_get_named_gpio(node, "lcm_power_gpio", 0);
        GPIO_LCM_RST = of_get_named_gpio(node, "lcm_reset_gpio", 0);
		GPIO_LCM_VDD = of_get_named_gpio(node, "lcm_vdd_gpio", 0);

		GPIO_LVDS_POWER = of_get_named_gpio(node, "lvds_power_gpio", 0);
		GPIO_LVDS_RST = of_get_named_gpio(node, "lvds_rst_gpio", 0);
		GPIO_LVDS_STBY = of_get_named_gpio(node, "lvds_stby_gpio", 0);
		
	 	GPIO_LCD_PWR_EN = of_get_named_gpio(node, "lcm_power_gpio", 0);
	 	GPIO_LCD_RST_EN = of_get_named_gpio(node, "lcm_reset_gpio", 0);
     	GPIO_USB1_5V_EN = of_get_named_gpio(node, "lcm_usb1_power_gpio", 0);
     	GPIO_ACCDET_EN = of_get_named_gpio(node, "lcm_accdet_gpio", 0);  
	 	GPIO_LCD_ID2_DECTET = of_get_named_gpio(node, "lcm_id2_gpio", 0);

        GPIO_IT6151_1V8 = of_get_named_gpio(node, "lcm_it6151_1v8", 0);
        GPIO_IT6151_BL = of_get_named_gpio(node, "lcm_it6151_bl", 0);
        GPIO_IT6151_STB = of_get_named_gpio(node, "lcm_it6151_stb", 0);
        GPIO_IT6151_RST = of_get_named_gpio(node, "lcm_it6151_rst", 0);
        GPIO_IT6151_INT = of_get_named_gpio(node, "lcm_it6151_int", 0);
        GPIO_VLED_EN = of_get_named_gpio(node, "lcm_vled_en", 0);

		printk("====lxl(lcm_get_gpio_infor  GPIO_LCM_PWR=%d )========\n",GPIO_LCM_PWR);
		printk("====lxl(lcm_get_gpio_infor  GPIO_LCM_RST=%d )========\n",GPIO_LCM_RST);
		printk("====lxl(lcm_get_gpio_infor  GPIO_LCM_VDD=%d )========\n",GPIO_LCM_VDD);
#ifdef CONFIG_USE_RGB_LCM
        pinctrlv = devm_pinctrl_get(dev);
        pinctrl_lcmvsy = pinctrl_lookup_state(pinctrlv, "lcmvsy_init");
        pinctrl_lcmvsy_low = pinctrl_lookup_state(pinctrlv, "lcmvsy_low");
        pinctrl_lcmvsy_high = pinctrl_lookup_state(pinctrlv, "lcmvsy_high");
        pinctrl_lcmhsy_low = pinctrl_lookup_state(pinctrlv, "lcmhsy_low");
        pinctrl_lcmhsy_high = pinctrl_lookup_state(pinctrlv, "lcmhsy_high");
        pinctrl_lcmdpi_low = pinctrl_lookup_state(pinctrlv, "lcmdpi_low");
        pinctrl_lcmdpi_high = pinctrl_lookup_state(pinctrlv, "lcmdpi_high");
        pinctrl_lcmde_low = pinctrl_lookup_state(pinctrlv, "lcmde_low");
        pinctrl_lcmde_high = pinctrl_lookup_state(pinctrlv, "lcmde_high");
        pinctrl_select_state(pinctrlv, pinctrl_lcmvsy);
#endif
}

/* get LDO supply */
int lcm_get_vgp_supply(struct device *dev)
{
        int ret;
        struct regulator *lcm_vgp_ldo;

        printk("LCM: lcm_get_vgp_supply is going\n");

        lcm_vgp_ldo = devm_regulator_get(dev, "reg-lcm");
        if (IS_ERR(lcm_vgp_ldo)) {
                ret = PTR_ERR(lcm_vgp_ldo);
                printk("failed to get reg-lcm LDO, %d\n", ret);
                return ret;
        }

        printk("LCM: lcm get supply ok.\n");

        ret = regulator_enable(lcm_vgp_ldo);
        /* get current voltage settings */
        ret = regulator_get_voltage(lcm_vgp_ldo);
        printk("lcm LDO voltage = %d in LK stage\n", ret);

        lcm_vgp = lcm_vgp_ldo;
	
         if(lcm_vgp == NULL){
			 printk("==========lxl(failed lcm_vgp)============\n");
		 }
        return ret;
}




static int lcm_probe(struct device *dev)
{
        lcm_get_vgp_supply(dev);
        //lcm_get_gpio_infor();
		lcm_get_gpio_infor(dev);
        printk("=====lxl(lcm_probe)=====\n");
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


static int __init lcm_init_p(void)
{
        printk("LCM: Register lcm driver\n");
		printk("=======lxl(lcm_init_p)=======\n");
        if (platform_driver_register(&lcm_driver)) {
                printk("LCM: failed to register disp driver\n");
                return -ENODEV;
        }

        return 0;
}

static void __exit lcm_exit(void)
{
        platform_driver_unregister(&lcm_driver);
        pr_notice("LCM: Unregister lcm driver done\n");
}
late_initcall(lcm_init_p);
module_exit(lcm_exit);
MODULE_AUTHOR("mediatek");
MODULE_DESCRIPTION("Display subsystem Driver");
MODULE_LICENSE("GPL");

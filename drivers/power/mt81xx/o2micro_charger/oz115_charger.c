/*
 *
 */


#define pr_fmt(fmt) "[oz115 charger]" fmt

#include <linux/init.h>
#include <linux/module.h>
#include <linux/reboot.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/printk.h>

#include "../mt_charging.h"
#include "../mt_battery_meter_hal.h"
#include "../mt_battery_common.h"
#include <mt-plat/upmu_common.h>
#include <mt-plat/mt_reboot.h>
#include <mt-plat/mt_boot.h>
#include "bluewhale_charger.h"

#define DRVNAME "oz115_charger"
#define STATUS_OK	0
#define STATUS_UNSUPPORTED	-1
#define GETARRAYNUM(array) (sizeof(array)/sizeof(array[0]))
static u32 charging_get_charger_det_status(void *data);


#if defined(CONFIG_MTK_PUMP_EXPRESS_PLUS_SUPPORT)
static inline bool mtk_pep_get_is_connect(void)
{	
	return is_ta_connect;
}
#endif
#undef CONFIG_MTK_PUMP_EXPRESS_PLUS_20_SUPPORT

/* for MT6391 */
static const u32 VCDT_HV_VTH[] = {
	BATTERY_VOLT_04_000000_V, BATTERY_VOLT_04_100000_V, BATTERY_VOLT_04_150000_V,
	    BATTERY_VOLT_04_200000_V,
	BATTERY_VOLT_04_250000_V, BATTERY_VOLT_04_300000_V, BATTERY_VOLT_04_350000_V,
	    BATTERY_VOLT_04_400000_V,
	BATTERY_VOLT_04_450000_V, BATTERY_VOLT_04_500000_V, BATTERY_VOLT_04_550000_V,
	    BATTERY_VOLT_04_600000_V,
	BATTERY_VOLT_07_000000_V, BATTERY_VOLT_07_500000_V, BATTERY_VOLT_08_500000_V,
	    BATTERY_VOLT_10_500000_V
};

static u32 bmt_find_closest_level(const u32 *pList, u32 number, u32 level)
{
	u32 i;
	u32 max_value_in_last_element;

	if (pList[0] < pList[1])
		max_value_in_last_element = true;
	else
		max_value_in_last_element = false;

	if (max_value_in_last_element == true) {
		for (i = (number - 1); i != 0; i--) {	/* max value in the last element */
			if (pList[i] <= level)
				return pList[i];
		}

		battery_log(BAT_LOG_CRTI, "Can't find closest level, small value first \r\n");
		return pList[0];
	}

	for (i = 0; i < number; i++) {	/* max value in the first element */
		if (pList[i] <= level)
			return pList[i];
	}

	battery_log(BAT_LOG_CRTI, "Can't find closest level, large value first \r\n");
	return pList[number - 1];
}

static u32 charging_parameter_to_value(const u32 *parameter, const u32 array_size, const u32 val)
{
	u32 i;

	for (i = 0; i < array_size; i++) {
		if (val == *(parameter + i))
			return i;
	}

	battery_log(BAT_LOG_CRTI, "NO register value match. val=%d\r\n", val);
	/* TODO: ASSERT(0);      // not find the value */
	return 0;
}

static u32 charging_hw_init(void *data)
{
	u32 status = STATUS_OK;

	bluewhale_init_extern();
#if defined(CONFIG_OZ115_DUAL_SUPPORT)
	oz115_sub_init_extern();
#endif

	return status;
}

static u32 charging_dump_register(void *data)
{
	u32 status = STATUS_OK;
	
	battery_log(BAT_LOG_FULL,"[charging_dump_register]\n");
    	bluewhale_dump_register();
#if defined(CONFIG_OZ115_DUAL_SUPPORT)
	oz115_sub_dump_register();
#endif

	return status;
}
//extern struct mt_battery_charging_custom_data *p_bat_charging_data;
//extern struct PMU_ChargerStruct BMT_status;
static u32 charging_enable(void *data)
{
	u32 status = STATUS_OK;
	u32 enable = *(u32 *) (data);

	battery_log(BAT_LOG_CRTI, "get_boot_mode=%d\n", get_boot_mode());
	if (get_boot_mode() == META_BOOT || 
		get_boot_mode() == ADVMETA_BOOT) {
		o2_suspend_charger_extern();
#ifdef CONFIG_OZ115_DUAL_SUPPORT
		oz115_sub_stop_charge_extern(0); //suspend sub-oz115 charger
#endif
		return STATUS_OK;
	}

    	battery_log(BAT_LOG_FULL,"[charging_enable] %d\n", enable);

    	if (true == enable) {
		o2_start_charge_extern();
	} else {
		if (BMT_status.temperature >=55)
			o2_suspend_charger_extern();
		else
#if defined(CONFIG_USB_MTK_HDRC_HCD)
		if (mt_usb_is_device())
#endif
		{
			charging_get_charger_det_status(&enable);
			if (enable == 0) {
				battery_log(BAT_LOG_FULL,"[charging_enable] cable out\n");
				o2_stop_charge_extern(1);
			}
			else
				o2_stop_charge_extern(0);
		}
	}

#ifdef CONFIG_OZ115_DUAL_SUPPORT
#if defined(CONFIG_MTK_PUMP_EXPRESS_PLUS_20_SUPPORT) || \
		defined(CONFIG_MTK_PUMP_EXPRESS_PLUS_SUPPORT)
	if ( 
#if defined(CONFIG_MTK_PUMP_EXPRESS_PLUS_20_SUPPORT)	
			mtk_pep20_get_is_connect() == false &&
#endif
#if defined(CONFIG_MTK_PUMP_EXPRESS_PLUS_SUPPORT)	
			 mtk_pep_get_is_connect() == false &&
#endif
		     true == enable )
	{
		oz115_sub_stop_charge_extern(0); //set SUS to disable charger
		return 1;
	}
#endif
	if (!oz115_sub_get_charger_current_extern() && true == enable) {
		oz115_sub_stop_charge_extern(0); //set SUS to disable charger
		return 1;
	}

    	if (true == enable) {
		oz115_sub_start_charge_extern();
	} else {
#if defined(CONFIG_USB_MTK_HDRC_HCD)
		if (mt_usb_is_device())
#endif
		{
			charging_get_charger_det_status(&enable);
			if (enable == 0) {
				battery_log(BAT_LOG_FULL,"[charging_enable] cable out\n");
				oz115_sub_stop_charge_extern(1);
			} else {
				oz115_sub_stop_charge_extern(0);
			}
		}
	}
#endif

	return status;
}

static u32 charging_set_cv_voltage(void *data)
{
	u32 status = STATUS_OK;
	unsigned int cv_value = *(unsigned int *) data / 1000; //mV

	battery_log(BAT_LOG_CRTI,"[charging_set_cv_voltage] set CV cv_value: %dmV\n", cv_value);

	//in case that CC = 0, Vsys is too high
	if (bluewhale_get_charger_current_extern() == 0)
		bluewhale_set_chg_volt_extern(4200);
	else 
		bluewhale_set_chg_volt_extern(cv_value);

#if defined(CONFIG_OZ115_DUAL_SUPPORT)
	oz115_sub_set_chg_volt_extern(bluewhale_get_chg_volt_extern());
#endif
    	return status;
}

static u32 charging_get_current(void *data)
{
	u32 status = STATUS_OK;
	unsigned int val = 0;

	val = 100 * bluewhale_get_charger_current_extern();
#if defined(CONFIG_OZ115_DUAL_SUPPORT)
	val += 100 * oz115_sub_get_charger_current_extern();
#endif

	*(u32 *)data = val;

    return status;
}



static u32 charging_set_current(void *data)
{
	u32 status = STATUS_OK;
	unsigned int current_value = *(unsigned int *) data / 100; //mA

#if defined(CONFIG_OZ115_DUAL_SUPPORT)
	unsigned int sub_current_value = 0;
	//unsigned int current_value_sum  = current_value;

    	//oz115_set_charge_current_sum_extern(current_value_sum);

#if defined(CONFIG_MTK_PUMP_EXPRESS_PLUS_20_SUPPORT) || defined(CONFIG_MTK_PUMP_EXPRESS_PLUS_SUPPORT)
	if(
#if defined(CONFIG_MTK_PUMP_EXPRESS_PLUS_20_SUPPORT)	
		(mtk_pep20_get_is_connect() == true) || //3A
#endif

#if defined(CONFIG_MTK_PUMP_EXPRESS_PLUS_SUPPORT)	
	    (mtk_pep_get_is_connect() == true) ||
#endif
	    false) //not TA charger
#else //don't support PE+/PE20
	//if(current_value >= 2000)
	if(0)
#endif
	{
		if (current_value < 1200) {
			sub_current_value = current_value;
			current_value = 0;//the min value is 600mA
		}
		else {
			sub_current_value = current_value / 2;
			sub_current_value /= 200; //resgister step: 200mA
			sub_current_value *= 200;
			current_value = current_value - sub_current_value; 
			if (current_value % 200)
				current_value = (current_value / 200) * 200 + 200;
		}
	}

	battery_log(BAT_LOG_CRTI,"charging set charge current %d mA,sub=%d mA\n",current_value,sub_current_value);
#endif
	bluewhale_set_charger_current_extern(current_value);
	//in case that CC = 0, Vsys is too high
	if (current_value == 0)
		bluewhale_set_chg_volt_extern(4200);

#if defined(CONFIG_OZ115_DUAL_SUPPORT)
	if(!sub_current_value)
		oz115_sub_stop_charge_extern(0); //set SUS to disable charger

	oz115_sub_set_charger_current_extern(sub_current_value);
#endif

    return status;
}


static u32 charging_set_input_current(void *data)
{
	u32 status = STATUS_OK;
	unsigned int current_value = *(unsigned int *) data / 100; //mA

#if defined(CONFIG_OZ115_DUAL_SUPPORT)
	unsigned int sub_current_value = 0;

#if defined(CONFIG_MTK_PUMP_EXPRESS_PLUS_20_SUPPORT) || defined(CONFIG_MTK_PUMP_EXPRESS_PLUS_SUPPORT)
	if(
#if defined(CONFIG_MTK_PUMP_EXPRESS_PLUS_20_SUPPORT)	
		(mtk_pep20_get_is_connect() == true && (current_value >= 200)) || //2A
#endif
#if defined(CONFIG_MTK_PUMP_EXPRESS_PLUS_SUPPORT)	
	    (mtk_pep_get_is_connect() == true && (current_value >= 200)) ||
#endif
	    false) //not TA charger
#else //don't support PE+/PE20
	//if(current_value >= 2000)
	if(0)
#endif
	{
		sub_current_value = current_value/2;
		current_value = current_value - sub_current_value; 
		if (sub_current_value == 1000) sub_current_value += 200; 
	}

	battery_log(BAT_LOG_CRTI,"charging set_input_current %d mA,sub=%d mA\n",current_value,sub_current_value);

#endif
	bluewhale_set_vbus_current_extern(current_value);

#if defined(CONFIG_OZ115_DUAL_SUPPORT)
	if(!sub_current_value)
		oz115_sub_stop_charge_extern(0); //set SUS to disable charger

	oz115_sub_set_vbus_current_extern(sub_current_value);
#endif

    return status;
}

static u32 charging_get_input_current(void *data)
{
	int ret = 0;
	unsigned int current_value = 0;

	current_value = bluewhale_get_vbus_current_extern();

#if defined(CONFIG_OZ115_DUAL_SUPPORT)
	if (oz115_sub_is_enabled())
		current_value += oz115_sub_get_vbus_current_extern();
#endif
	*((u32 *)data) = current_value * 100;

	return ret;
}

static u32 charging_get_charging_status(void *data)
{
	u32 status = STATUS_OK;
	if (bluewhale_get_charging_status_extern() == 1)
		*(u32 *)data = true; //soc 100
	else
		*(u32 *)data = false;

#if defined(CONFIG_OZ115_DUAL_SUPPORT)
	oz115_sub_get_charging_status_extern();
#endif

    return status;
}

static u32 charging_reset_watch_dog_timer(void *data)
{
	u32 status = STATUS_OK;

	//set nothing temp  3-21

	return status;
}

static u32 charging_set_hv_threshold(void *data)
{
	u32 status = STATUS_OK;

	u32 set_hv_voltage;
	u32 array_size;
	u16 register_value;
	u32 voltage = *(u32 *) (data);

	array_size = GETARRAYNUM(VCDT_HV_VTH);
	set_hv_voltage = bmt_find_closest_level(VCDT_HV_VTH, array_size, voltage);
	register_value = charging_parameter_to_value(VCDT_HV_VTH, array_size, set_hv_voltage);
	//pmic_set_register_value(PMIC_RG_VCDT_HV_VTH, register_value);
	upmu_set_rg_vcdt_hv_vth(register_value);
	return status;
}

static unsigned int charging_get_hv_status(void *data)
{
	u32 status = STATUS_OK;

	*(bool *) (data) = upmu_get_rgs_vcdt_hv_det();
	return status;
}

static unsigned int charging_get_battery_status(void *data)
{
	u32 status = STATUS_OK;

	/* upmu_set_baton_tdet_en(1); */
	upmu_set_rg_baton_en(1);
	*(bool *) (data) = upmu_get_rgs_baton_undet();

	return status;
}

static u32 charging_get_charger_det_status(void *data)
{
	u32 status = STATUS_OK;

	*(bool *) (data) = upmu_get_rgs_chrdet();

	return status;
}

extern int hw_charger_type_detection(void);
static u32 charging_get_charger_type(void *data)
{
	u32 status = STATUS_OK;

	*(int *) (data) = hw_charger_type_detection();

	return status;
}

#if 1
static u32 charging_get_is_pcm_timer_trigger(void *data)
{
	u32 status = STATUS_OK;
	*(int *)(data) = false;
	return status;
}
#else
static u32 charging_get_is_pcm_timer_trigger(void *data)
{
	u32 status = STATUS_OK;

	if (slp_get_wake_reason() == 3)
		*(bool *) (data) = true;
	else
		*(bool *) (data) = false;

	battery_log(BAT_LOG_CRTI, "slp_get_wake_reason=%d\n", slp_get_wake_reason());

	*(bool *) (data) = false;
	return status;
}
#endif

static u32 charging_set_platform_reset(void *data)
{
	u32 status = STATUS_OK;

	battery_log(BAT_LOG_CRTI, "charging_set_platform_reset\n");

	if (system_state == SYSTEM_BOOTING)
		arch_reset(0, NULL);
	else
		orderly_reboot(true);

	return status;
}

static u32 charging_get_platform_boot_mode(void *data)
{
	unsigned int status = STATUS_OK;

	*(unsigned int *) (data) = get_boot_mode();

	battery_log(BAT_LOG_CRTI, "get_boot_mode=%d\n", get_boot_mode());

	return status;
}

static u32 charging_boost_enable(void *data)
{
	int enable;

	enable = *(int *)data;
	battery_log(BAT_LOG_CRTI, "%s: %d\n", __FUNCTION__, enable);

	if (enable)
		bluewhale_enable_otg_extern(1);
	else
		bluewhale_enable_otg_extern(0);

	return STATUS_OK;
}

static u32 charging_set_ta_current_pattern(void *data)
{
#if !defined(CONFIG_MTK_PUMP_EXPRESS_PLUS_SUPPORT)
	return STATUS_UNSUPPORTED;
#else
	unsigned int status = STATUS_OK;
	unsigned int increase = *(unsigned int*)(data);

	battery_log(BAT_LOG_CRTI, "charging_set_ta_current_pattern\n");

#ifdef CONFIG_OZ115_DUAL_SUPPORT
	oz115_sub_stop_charge_extern(0);
	//oz115_sub_dump_register();
#endif
	//bluewhale_dump_register();
	bluewhale_set_ta_current_pattern_extern(increase);

	return status;	
#endif
}

static u32(*charging_func[CHARGING_CMD_NUMBER]) (void *data);
s32 oz115_charger_control_interface(int cmd, void *data)
{
	s32 status;

	static bool is_init;

	if (is_init == false) {
		charging_func[CHARGING_CMD_INIT] = charging_hw_init;
		charging_func[CHARGING_CMD_DUMP_REGISTER] = charging_dump_register;
		charging_func[CHARGING_CMD_ENABLE] = charging_enable;
		charging_func[CHARGING_CMD_SET_CV_VOLTAGE] = charging_set_cv_voltage;
		charging_func[CHARGING_CMD_GET_CURRENT] = charging_get_current;
		charging_func[CHARGING_CMD_SET_CURRENT] = charging_set_current;
		charging_func[CHARGING_CMD_GET_INPUT_CURRENT] = charging_get_input_current;
		charging_func[CHARGING_CMD_SET_INPUT_CURRENT] = charging_set_input_current;
		charging_func[CHARGING_CMD_GET_CHARGING_STATUS] = charging_get_charging_status;
		charging_func[CHARGING_CMD_RESET_WATCH_DOG_TIMER] = charging_reset_watch_dog_timer;
		charging_func[CHARGING_CMD_SET_HV_THRESHOLD] = charging_set_hv_threshold;
		charging_func[CHARGING_CMD_GET_HV_STATUS] = charging_get_hv_status;
		charging_func[CHARGING_CMD_GET_BATTERY_STATUS] = charging_get_battery_status;
		charging_func[CHARGING_CMD_GET_CHARGER_DET_STATUS] =
		    charging_get_charger_det_status;
		charging_func[CHARGING_CMD_GET_CHARGER_TYPE] = charging_get_charger_type;
		charging_func[CHARGING_CMD_GET_IS_PCM_TIMER_TRIGGER] =
		    charging_get_is_pcm_timer_trigger;
		charging_func[CHARGING_CMD_SET_PLATFORM_RESET] = charging_set_platform_reset;
		charging_func[CHARGING_CMD_GET_PLATFORM_BOOT_MODE] =
		    	charging_get_platform_boot_mode;
		charging_func[CHARGING_CMD_BOOST_ENABLE] = charging_boost_enable;
		charging_func[CHARGING_CMD_SET_TA_CURRENT_PATTERN] = 
			charging_set_ta_current_pattern;
		is_init = true;
	}

	if (cmd < CHARGING_CMD_NUMBER && charging_func[cmd])
		status = charging_func[cmd] (data);
	else {
		pr_err("Unsupported charging command:%d!\n", cmd);
		return STATUS_UNSUPPORTED;
	}

	return status;
}

static int oz115_charger_probe(struct platform_device *pdev)
{
	int ret;
	pr_notice("%s\n", __FUNCTION__);
	
	ret = bat_charger_register(oz115_charger_control_interface);
	if (ret) {
		pr_err("bar_charger_register fail!\n");
		return ret;
	}
	return 0;
}

static int oz115_charger_remove(struct platform_device *pdev)
{
	pr_notice("%s\n", __FUNCTION__);
	return 0;
}

static void oz115_charger_shutdown(struct platform_device *pdev)
{
	pr_notice("%s\n", __FUNCTION__);
}

static const struct of_device_id of_oz115_charger_id[] = {
	{.compatible = "o2micro,oz115_charger"},
	{}
};

static struct platform_driver oz115_charger_pdrv = {
	.probe = oz115_charger_probe,
	.remove = oz115_charger_remove,
	.shutdown = oz115_charger_shutdown,
	.driver = {
		.name = DRVNAME,
		.of_match_table = of_oz115_charger_id,
	},
} ;

static int __init oz115_charger_mod_init(void)
{
	int ret = 0;

	ret = platform_driver_register(&oz115_charger_pdrv);
	if (ret) {
		pr_err("platform driver register fail!\n");
		return ret;
	}

	return ret;
}

static void __exit oz115_charger_mod_exit(void)
{
	platform_driver_unregister(&oz115_charger_pdrv);
}

module_init(oz115_charger_mod_init);
module_exit(oz115_charger_mod_exit);

MODULE_AUTHOR("Round R. Robin From O2micro");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("simple module for integrating "
		"oz115 driver into mt8176 platform");

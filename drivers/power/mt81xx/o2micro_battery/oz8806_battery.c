/*****************************************************************************
* Copyright(c) O2Micro, 2013. All rights reserved.
*	
* O2Micro oz8806 battery gauge driver
* File: oz8806_battery.c

* Author: Eason.yuan
* $Source: /data/code/CVS
* $Revision: 4.00.01 $
*
* This program is free software and can be edistributed and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*	
* This Source Code Reference Design for O2MICRO oz8806 access (\u201cReference Design\u201d) 
* is sole for the use of PRODUCT INTEGRATION REFERENCE ONLY, and contains confidential 
* and privileged information of O2Micro International Limited. O2Micro shall have no 
* liability to any PARTY FOR THE RELIABILITY, SERVICEABILITY FOR THE RESULT OF PRODUCT 
* INTEGRATION, or results from: (i) any modification or attempted modification of the 
* Reference Design by any party, or (ii) the combination, operation or use of the 
* Reference Design with non-O2Micro Reference Design.
*****************************************************************************/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/power_supply.h>
#include <linux/i2c.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/wakelock.h>
#include <linux/suspend.h>

//you must add these here for O2MICRO
#include "parameter.h"
#include "table.h"
#include "battery_config.h"
#include "oz8806_regdef.h"


#ifdef O2_CHARGER_SUPPORT
#include "../o2micro_charger/bluewhale_charger.h"
#endif

#ifdef MTK_MACH_SUPPORT
#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,18,0))

#if defined(CONFIG_ARCH_MT8173)
#define KAL_TRUE true
#include "../mt_charging.h"
#include "../mt_battery_custom_data.h"
#include "../mt_battery_common.h"
#include "../mt_battery_meter.h"
#else
#include <mt-plat/battery_common.h>
#include <mt-plat/mtk_rtc.h>
#include <mt-plat/upmu_common.h>
#endif

#else
#include <mach/battery_common.h>
#include <mach/mtk_rtc.h>
#endif
#endif 

#ifdef INTEL_MACH

#include "../power_supply.h"
#include <linux/acpi.h>
#define batt_dbg(fmt, args...) pr_err("[oz8806]:"pr_fmt(fmt)"\n", ## args)

#else

#define batt_dbg(fmt, args...) printk(KERN_ERR"[oz8806]:"pr_fmt(fmt)"\n", ## args)

#endif


//#define OZ8806_TEMP_SIMULATION 
//#define AUTO_RESUME_TEST

#ifdef AUTO_RESUME_TEST
#include <linux/android_alarm.h>
#endif

/*****************************************************************************
* static variables/functions section 
****************************************************************************/
static int fg_hw_init_done = 0;
static uint8_t 	bmu_init_done = 0;
static int oz8806_suspend = 0;
static DEFINE_MUTEX(update_mutex);
#ifdef OZ8806_API
struct mutex *update_mutex_ptr = &update_mutex;
bmu_data_t 	*batt_info_ptr = NULL;
#else
static bmu_data_t 	*batt_info_ptr = NULL;
#endif
static gas_gauge_t *gas_gauge_ptr = NULL;
static uint8_t charger_finish = 0;
static unsigned long ic_wakeup_time = 0;

static void  (*bmu_polling_loop_callback)(void) = NULL;
static void  (*bmu_wake_up_chip_callback)(void) = NULL;
static void  (*bmu_power_down_chip_callback)(void) = NULL;
static void  (*bmu_charge_end_process_callback)(void) = NULL;
static void  (*bmu_discharge_end_process_callback)(void) = NULL;
static int32_t (*oz8806_temp_read_callback)(int32_t *voltage) = NULL;
static int32_t (*oz8806_current_read_callback)(int32_t *voltage) = NULL;
static int32_t (*oz8806_voltage_read_callback)(int32_t *voltage) = NULL;

static int oz8806_update_batt_info(struct oz8806_data *data);
static int oz8806_update_batt_temp(struct oz8806_data *data);
static int32_t oz8806_write_byte(struct oz8806_data *data, uint8_t index, uint8_t dat);
static int32_t oz8806_read_byte(struct oz8806_data *data, uint8_t index, uint8_t *dat);

static struct oz8806_data *the_oz8806;
static struct wake_lock battery_wake_lock;
static int8_t adapter_status = O2_CHARGER_BATTERY;
static int32_t rsoc_pre;
static uint8_t ext_thermal_read;
static int capacity_init_ok = 0;
static struct power_supply *battery_psy = NULL;

#ifndef MTK_MACH_SUPPORT
static struct power_supply *ac_psy = NULL;
static struct power_supply *usb_psy = NULL;
#endif
static int create_sys = -EINVAL;
static int save_capacity = INIT_CAP;

#ifdef OZ8806_TEMP_SIMULATION 
static int batt_temp_sim = 27;
#endif

#ifdef AUTO_RESUME_TEST
struct alarm  auto_resume_alarm;
struct timespec ts;
struct delayed_work auto_resume_work;
static struct wake_lock auto_resume_wake_lock;
static int auto_resume_interval = 180; //s

void oz8806_setup_alarm(void);
#endif

#ifdef O2_GAUGE_POWER_SUPPOLY_SUPPORT 
static enum power_supply_property oz8806_battery_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN,
	POWER_SUPPLY_PROP_CHARGE_NOW,
	POWER_SUPPLY_PROP_ENERGY_FULL_DESIGN,
	POWER_SUPPLY_PROP_ENERGY_NOW,
	//POWER_SUPPLY_PROP_TIME_TO_EMPTY_NOW,
	//POWER_SUPPLY_PROP_TIME_TO_EMPTY_AVG,
	//POWER_SUPPLY_PROP_TIME_TO_FULL_NOW,
};
#endif


#ifdef O2_GAUGE_POWER_SUPPOLY_SUPPORT 
/*-------------------------------------------------------------------------*/
/*****************************************************************************
* Description:
*		below function is linux power section
* Parameters:
*		description for each argument, new argument starts at new line
* Return:
*		what does this function returned?
*****************************************************************************/
/*-------------------------------------------------------------------------*/

static int oz8806_battery_get_property(struct power_supply *psy,
					enum power_supply_property psp,
					union power_supply_propval *val)
{
	int ret = 0;
	
	struct oz8806_data *data = container_of(psy, struct oz8806_data, bat);

	switch (psp) {
	
	case POWER_SUPPLY_PROP_STATUS:
		
		if (adapter_status == O2_CHARGER_BATTERY)
		{
			val->intval = POWER_SUPPLY_STATUS_DISCHARGING; /*discharging*/
		}
		else if(adapter_status == O2_CHARGER_USB ||
		        adapter_status == O2_CHARGER_AC )
		{
			if (data->batt_info.batt_soc == 100)
				val->intval = POWER_SUPPLY_STATUS_FULL;
			else
				val->intval = POWER_SUPPLY_STATUS_CHARGING;	/*charging*/

		}
		else
			val->intval = POWER_SUPPLY_STATUS_DISCHARGING;
		
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		val->intval = data->batt_info.batt_voltage * 1000;
		break;
	case POWER_SUPPLY_PROP_PRESENT:
		val->intval = 1;
		break;
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		val->intval = data->batt_info.batt_current * 1000;
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		val->intval = data->batt_info.batt_soc;
		break;
	case POWER_SUPPLY_PROP_TECHNOLOGY:
		val->intval = POWER_SUPPLY_TECHNOLOGY_LION;	
		break;
	case POWER_SUPPLY_PROP_HEALTH:
		val->intval = POWER_SUPPLY_HEALTH_GOOD;
		break;

	case POWER_SUPPLY_PROP_TEMP:
		val->intval = data->batt_info.batt_temp * 10;
		break;

	case POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN:
		val->intval = data->batt_info.batt_fcc_data;
		break;

	case POWER_SUPPLY_PROP_CHARGE_NOW:
		val->intval = data->batt_info.batt_capacity;
		break;

	case POWER_SUPPLY_PROP_ENERGY_FULL_DESIGN:
		val->intval = data->batt_info.batt_fcc_data;
		break;

	case POWER_SUPPLY_PROP_ENERGY_NOW:
		val->intval = data->batt_info.batt_capacity;
		break;

	default:
		return -EINVAL;
	}

	return ret;
}

#ifdef INTEL_MACH
static int oz8806_battery_set_property(struct power_supply *psy,
					enum power_supply_property psp,
					union power_supply_propval *val)
{
	struct oz8806_data *data = container_of(psy,
			struct oz8806_data, bat);
	int ret = 0;
	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		if (data->status != val->intval) {
			data->status = val->intval;
#ifdef O2_GAUGE_WORK_SUPPORT
			schedule_delayed_work(&data->work,0);
#endif
		}
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}
#endif

static void oz8806_external_power_changed(struct power_supply *psy)
{
	struct oz8806_data *data = container_of(psy,
			struct oz8806_data, bat);

	power_supply_changed(&data->bat);
#ifdef O2_GAUGE_WORK_SUPPORT
	cancel_delayed_work(&data->work);
	schedule_delayed_work(&data->work, 0);
#endif
}

static void oz8806_powersupply_init(struct oz8806_data *data)
{
	data->bat.name = "battery";
	data->bat.type = POWER_SUPPLY_TYPE_BATTERY;
	data->bat.properties = oz8806_battery_props;
	data->bat.num_properties = ARRAY_SIZE(oz8806_battery_props);
	data->bat.get_property = oz8806_battery_get_property;
	data->bat.external_power_changed = oz8806_external_power_changed;
#ifdef INTEL_MACH
	data->bat.set_property = oz8806_battery_set_property;
#endif
}
#endif

/*****************************************************************************
 *write 0x20 into register 0x09
 *example:echo 0920 > /sys/class/i2c-dev/i2c-2/device/2-002f/registers
 *****************************************************************************/

static ssize_t oz8806_register_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t _count)
{
	char data[num_3];
	uint8_t address;
	uint8_t value = 0;
	char *endp; 
	int len;
	struct oz8806_data *oz8806;

    len = strlen(buf);
	if(len < 5)
		return -EINVAL;
	
	if (&(the_oz8806->myclient->dev) == dev)
	    oz8806 = dev_get_drvdata(dev);
	else //this device is the private device of power supply 
	    oz8806 = dev_get_drvdata(dev->parent);

	data[num_0] = buf[num_0];
 	data[num_1] = buf[num_1];
 	data[num_2] = num_0;

	address = simple_strtoul(data, &endp, 16); 

	if (address != OZ8806_OP_PEC_CTRL
	 && address != OZ8806_OP_CTRL
	 && address != OZ8806_OP_CAR
	 && address != OZ8806_OP_CAR+1
	 && address != OZ8806_OP_BOARD_OFFSET
	 && address != OZ8806_OP_BOARD_OFFSET+1)
	{
		pr_err("register[0x%.2x] is read-only\n", address);
		return _count;
	}

	data[num_0] = buf[num_2];
	data[num_1] = buf[num_3];
	data[num_2] = num_0;

	value = simple_strtoul(data, &endp, 16); 

	oz8806_write_byte(oz8806, address, value);

  	pr_err("write 0x%.2x into register[0x%.2x]\n", value, address);

	return _count;
}

/*
	example:cat /sys/class/i2c-dev/i2c-2/device/2-002f/registers
*/
static ssize_t oz8806_register_show(struct device *dev, struct device_attribute *attr,char *buf)
{	
	struct oz8806_data *oz8806;
	u8 i = 0;
	u8 data = 0;
	int result = 0;
	
	if (&(the_oz8806->myclient->dev) == dev)
	    oz8806 = dev_get_drvdata(dev);
	else //this device is the private device of power supply 
	    oz8806 = dev_get_drvdata(dev->parent);

	//CHIP ID and Reversion
	oz8806_read_byte(oz8806, OZ8806_OP_IDREV, &data);
	result += sprintf(buf + result, "[0x%.2x] = 0x%.2x\n", OZ8806_OP_IDREV, data);

	//I2C config register
	oz8806_read_byte(oz8806, OZ8806_OP_I2CCONFIG, &data);
	result += sprintf(buf + result, "[0x%.2x] = 0x%.2x\n", OZ8806_OP_I2CCONFIG, data);

    for (i=OZ8806_OP_PEC_CTRL; i<=0x19; i++)
    {
		oz8806_read_byte(oz8806, i, &data);
		result += sprintf(buf + result, "[0x%.2x] = 0x%.2x\n", i, data);
	}
	
	return result;
}

static ssize_t oz8806_debug_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	return sprintf(buf,"%d\n", config_data.debug);
}
/*****************************************************************************
 * Description:
 *		oz8806_debug_store
 * Parameters:
 *		write example: echo 1 > debug ---open debug
 *****************************************************************************/
static ssize_t oz8806_debug_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t _count)
{
	int val;
	if (kstrtoint(buf, 10, &val))
		return -EINVAL;

	if(val == 1)
	{
		config_data.debug = 1;
		pr_err("DEBUG ON \n");
	}
	else if (val == 0)
	{
		config_data.debug = 0;
		pr_err("DEBUG CLOSE \n");
	}
	else
	{
		pr_err("invalid command\n");
		return -EINVAL;
	}

	return _count;
}

#ifdef AUTO_RESUME_TEST
static ssize_t oz8806_resume_interval_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	return sprintf(buf,"%d\n", auto_resume_interval);
}

static ssize_t oz8806_resume_interval_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t _count)
{
	int val;
	if (kstrtoint(buf, 10, &val))
		return -EINVAL;

	if (val == 0)
	{
		auto_resume_interval = 0;
		pr_err("auto_resume_interval OFF \n");
	}
	else
	{
		auto_resume_interval = val;
		pr_err("auto_resume_interval ON : %d\n", auto_resume_interval);
	}
	return _count;
}
#endif

static DEVICE_ATTR(registers, S_IRUGO | (S_IWUSR|S_IWGRP), oz8806_register_show, oz8806_register_store);
static DEVICE_ATTR(debug, S_IRUGO | (S_IWUSR|S_IWGRP), oz8806_debug_show, oz8806_debug_store);
#ifdef AUTO_RESUME_TEST
static DEVICE_ATTR(auto_resume_interval, S_IRUGO | (S_IWUSR|S_IWGRP), oz8806_resume_interval_show, oz8806_resume_interval_store);
#endif
static struct attribute *oz8806_attributes[] = {
	&dev_attr_registers.attr,
	&dev_attr_debug.attr,
#ifdef AUTO_RESUME_TEST
	&dev_attr_auto_resume_interval.attr,
#endif
	NULL,
};

static struct attribute_group oz8806_attribute_group = {
	.attrs = oz8806_attributes,
};
static ssize_t oz8806_bmu_init_done_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	return sprintf(buf,"%d\n", capacity_init_ok);
}

static ssize_t oz8806_save_capacity_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	if (capacity_init_ok && gas_gauge_ptr && gas_gauge_ptr->stored_capacity)
		return sprintf(buf,"%d\n", gas_gauge_ptr->stored_capacity);

	return sprintf(buf,"%d\n", save_capacity);
}
#ifdef USERSPACE_READ_SAVED_CAP
static ssize_t oz8806_save_capacity_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int val;
	struct oz8806_data *oz8806;

	if (&(the_oz8806->myclient->dev) == dev)
	    oz8806 = dev_get_drvdata(dev);
	else //this device is the private device of power supply 
	    oz8806 = dev_get_drvdata(dev->parent);

	if (kstrtoint(buf, 10, &val))
		return -EINVAL;
	save_capacity = val;
	batt_dbg("save_capacity:%d\n", save_capacity);

	if (oz8806 && !bmu_init_done)                                       
	{
		cancel_delayed_work(&oz8806->work);
		schedule_delayed_work(&oz8806->work, 0);
	}

	return count;
}
#endif

#ifdef OZ8806_TEMP_SIMULATION 
static ssize_t batt_temp_store(struct device *dev, struct device_attribute *attr, 
               const char *buf, size_t count) 
{
	int val;
	struct oz8806_data *oz8806;

	if (kstrtoint(buf, 10, &val))
		return -EINVAL;

	if (&(the_oz8806->myclient->dev) == dev)
	    oz8806 = dev_get_drvdata(dev);
	else //this device is the private device of power supply 
	    oz8806 = dev_get_drvdata(dev->parent);

	batt_temp_sim = val;

	batt_dbg("batt_temp_simulation: %d ", batt_temp_sim);
#ifdef O2_GAUGE_WORK_SUPPORT
	cancel_delayed_work(&oz8806->work);
	schedule_delayed_work(&oz8806->work, 0);
#endif

	return count;
}

static ssize_t batt_temp_show(struct device *dev, struct device_attribute *attr,char *buf)
{
	return sprintf(buf, "%d\n", batt_temp_sim);
}
#endif


static DEVICE_ATTR(bmu_init_done, S_IRUGO, oz8806_bmu_init_done_show, NULL);

#ifdef USERSPACE_READ_SAVED_CAP
static DEVICE_ATTR(save_capacity, S_IRUGO | (S_IWUSR|S_IWGRP), oz8806_save_capacity_show, oz8806_save_capacity_store);
#else
static DEVICE_ATTR(save_capacity, S_IRUGO, oz8806_save_capacity_show, NULL);
#endif

#ifdef OZ8806_TEMP_SIMULATION 
static DEVICE_ATTR(batt_temp_sim, S_IRUGO | (S_IWUSR|S_IWGRP), batt_temp_show, batt_temp_store);
#endif


static struct attribute *battery_psy_attributes[] = {
	&dev_attr_bmu_init_done.attr,
	&dev_attr_save_capacity.attr,
#ifdef OZ8806_TEMP_SIMULATION 
	&dev_attr_batt_temp_sim.attr,
#endif
	NULL,
};

static struct attribute_group battery_psy_attribute_group = {
	.attrs = battery_psy_attributes,
};

static int oz8806_create_sys(struct device *dev, const struct attribute_group * grp)
{
	int err;

	batt_dbg("oz8806_create_sysfs");
	
	if(NULL == dev){
		pr_err("[BATT]: failed to register battery\n");
		return -EINVAL;
	}

	err = sysfs_create_group(&(dev->kobj), grp);

	if (!err) 
	{
		batt_dbg("creat oz8806 sysfs group ok");	
	} 
	else 
	{
		pr_err("creat oz8806 sysfs group fail\n");
		return -EIO;
	}
	return err;
}

static int get_battery_psy(void)
{
	int ret = -EINVAL;

	battery_psy = power_supply_get_by_name ("battery"); 

	if (battery_psy)
		ret = oz8806_create_sys(battery_psy->dev, &battery_psy_attribute_group);
	else
		pr_err("%s: failed to get battery power_supply\n", __func__);

	return ret;
}

static int32_t oz8806_read_byte(struct oz8806_data *data, uint8_t index, uint8_t *dat)
{
	int32_t ret;
	uint8_t i;
	struct i2c_client *client = data->myclient;

	for(i = 0; i < 4; i++){
		ret = i2c_smbus_read_byte_data(client, index);

		if(ret >= 0) break;
		else
			dev_err(&client->dev, "%s: err %d, %d times\n", __func__, ret, i);
	}
	if(i >= 4)
	{
		return ret;
	} 
	*dat = (uint8_t)ret;

	return ret;
}

static int32_t oz8806_write_byte(struct oz8806_data *data, uint8_t index, uint8_t dat)
{
	int32_t ret;
	uint8_t i;
	struct i2c_client *client = data->myclient;
	
	for(i = 0; i < 4; i++){
		ret = i2c_smbus_write_byte_data(client, index, dat);
		if(ret >= 0) break;
		else
			dev_err(&client->dev, "%s: err %d, %d times\n", __func__, ret, i);
	}
	if(i >= 4)
	{
		return ret;
	}

	return ret;
}

static int32_t oz8806_update_bits(struct oz8806_data *data, uint8_t reg, uint8_t mask, uint8_t dat)
{
	int32_t ret;
	uint8_t tmp;

	ret = oz8806_read_byte(data, reg, &tmp);
	if (ret < 0)
		return ret;

	if ((tmp & mask) != dat)
	{
		tmp &= ~mask;
		tmp |= dat & mask;
		return oz8806_write_byte(data, reg, tmp);

	}
	else
		return 0;
}
#if 0
static int oz8806_check_battery_status(struct oz8806_data *data)
{
	uint8_t val;
	int ret = 0;

	ret = oz8806_read_byte(data, OZ8806_OP_BATTERY_ID, &val);

	pr_err("battery ID: 0x%02x\n", val);

	return ret;
}
#endif
int oz8806_wakeup_full_power(void)
{
	uint8_t val;
	int ret = 0;
	struct oz8806_data *data = the_oz8806;

	ret = oz8806_read_byte(data, OZ8806_OP_CTRL, &val);
	pr_err("OZ8806_OP_CTRL: 0x%02x\n", val);

	if ((val & SLEEP_MODE) != 0<<6)
	{
		ret = oz8806_update_bits(data, OZ8806_OP_CTRL, SLEEP_MODE, 0 << 6);

		ret = oz8806_read_byte(data, OZ8806_OP_CTRL, &val);

		pr_err("OZ8806_OP_CTRL: 0x%02x after writing\n", val);

		if ((val & SLEEP_MODE) != (0 << 6))
			pr_err("fail to wake up oz8806 to full power mode\n");
		else {
			if (oz8806_get_boot_up_time() < 2000)
				ic_wakeup_time = jiffies + msecs_to_jiffies(2000 - oz8806_get_boot_up_time());
			else
				ic_wakeup_time = jiffies;
			pr_err("wake up oz8806 to full power mode\n");
		}
	}

	return ret;
}
EXPORT_SYMBOL(oz8806_wakeup_full_power);

static void discharge_end_fun(struct oz8806_data *data)
{
	//End discharge
	//this may jump 2%
	if (!gas_gauge_ptr || !batt_info_ptr)
		return;
  /*  
    if(batt_info_ptr->fCellTemp < -10)
		config_data.discharge_end_voltage = OZ8806_EOD + 350;
	else if (batt_info_ptr->fCellTemp < 0)
		config_data.discharge_end_voltage = OZ8806_EOD + 300;
	else if (batt_info_ptr->fCellTemp < 5)
		config_data.discharge_end_voltage = OZ8806_EOD + 200;
	else if (batt_info_ptr->fCellTemp < 10)
		config_data.discharge_end_voltage = OZ8806_EOD + 100;
    else
        config_data.discharge_end_voltage = OZ8806_EOD;

*/

	if(batt_info_ptr->fVolt < (config_data.discharge_end_voltage - 100))
    {

        if(batt_info_ptr->fRSOC == 1)
        {
            batt_info_ptr->fRSOC = 0;
            batt_info_ptr->sCaMAH = data->batt_info.batt_fcc_data / num_100 -1;
            if (bmu_discharge_end_process_callback)
                bmu_discharge_end_process_callback();
        }
        else if(batt_info_ptr->fRSOC > 0){
            batt_info_ptr->sCaMAH = batt_info_ptr->fRSOC * data->batt_info.batt_fcc_data / num_100 - 1;
            batt_info_ptr->fRSOC--;
		}
    }
}

static int check_charger_full(void)
{
	int ret = 0;

#if defined (MTK_MACH_SUPPORT)
	if (BMT_status.bat_full == 1)
		ret = 1;

#elif defined (O2_CHARGER_SUPPORT)
	if (bluewhale_get_charging_status_extern() == 1)//chg_full
		ret = 1;
#endif
	return ret;
}

static void charge_end_fun(void)
{
#define CHG_END_PERIOD_MS	(MSEC_PER_SEC * 300) //300 s = 5 minutes 
#define FORCE_FULL_MS	(MSEC_PER_SEC * 600) //600 s = 10 minutes 
#define CHG_END_PURSUE_STEP    (MSEC_PER_SEC * 30) //30s

	static unsigned int time_accumulation;
	static unsigned long start_jiffies;
	int oz8806_eoc = 0;
	static unsigned long chgr_full_soc_pursue_start;
	static unsigned int chgr_full_soc_pursue_accumulation;

#ifdef ENABLE_10MIN_END_CHARGE_FUN
	static unsigned long start_record_jiffies;
	static uint8_t start_record_flag = 0;
#endif
	
	if (!gas_gauge_ptr || !batt_info_ptr)
		return;

	if (adapter_status != O2_CHARGER_BATTERY && check_charger_full())
	{
		charger_finish = 1;
		batt_dbg("charger is full, enter external charger finish");
		goto charger_full;
	}

	if((adapter_status == O2_CHARGER_BATTERY) ||
		(batt_info_ptr->fCurr < DISCH_CURRENT_TH) || 
		(batt_info_ptr->fCurr >  config_data.charge_end_current))
	{
		charger_finish   = 0;
		time_accumulation = 0;
		start_jiffies = 0;
		chgr_full_soc_pursue_start = 0;
#ifdef ENABLE_10MIN_END_CHARGE_FUN
		start_record_jiffies = 0;
		start_record_flag = 0;
#endif
		return;
	}	
	/*
	if(adapter_status == CHARGER_USB)
	{
		gas_gauge_ptr->fast_charge_step = 1;
	}
	else
	{
		gas_gauge_ptr->fast_charge_step = 2;
	}
	*/
	
	if(adapter_status == O2_CHARGER_USB)
		oz8806_eoc = 100;
	else if(adapter_status == O2_CHARGER_AC)
		oz8806_eoc = config_data.charge_end_current;

	if((batt_info_ptr->fVolt >= (config_data.charge_cv_voltage - 50))&&(batt_info_ptr->fCurr >= DISCH_CURRENT_TH) &&
		(batt_info_ptr->fCurr < oz8806_eoc)&& (!gas_gauge_ptr->charge_end))
	{
		if (!start_jiffies)
			start_jiffies = jiffies;

		time_accumulation = jiffies_to_msecs(jiffies - start_jiffies);

		//time accumulation is over 5 minutes
		if (time_accumulation >= CHG_END_PERIOD_MS)
		{
			charger_finish	 = 1;
			batt_dbg("enter external charger finish");
		}
		else
		{
			charger_finish	= 0;
		}
	}
	else
	{
		time_accumulation = 0;
		start_jiffies = 0;
		charger_finish = 0;
	}

charger_full:
	batt_dbg("%s, time_accumulation:%d, charger_finish:%d",
			__func__, time_accumulation, charger_finish);

	batt_dbg("voltage:%d, cv:%d, fcurr:%d, 8806 eoc:%d, gas_gauge_ptr->charge_end:%d",
			batt_info_ptr->fVolt, config_data.charge_cv_voltage,
			batt_info_ptr->fCurr, oz8806_eoc, gas_gauge_ptr->charge_end);
	
#ifdef ENABLE_10MIN_END_CHARGE_FUN
	if((batt_info_ptr->fRSOC == 99) &&(!start_record_flag) &&(batt_info_ptr->fCurr > oz8806_eoc))
	{
		start_record_jiffies = jiffies;
		start_record_flag = 1;
		batt_dbg("start_record_flag: %d, at %d ms",start_record_flag, jiffies_to_msecs(jiffies));
	}
	if((batt_info_ptr->fRSOC != 99) ||(batt_info_ptr->fCurr < oz8806_eoc))
	{
		start_record_flag = 0;
	}

	if((start_record_flag) && (batt_info_ptr->fCurr > oz8806_eoc))
	{
		if(jiffies_to_msecs(jiffies - start_record_jiffies) > FORCE_FULL_MS)
		{
			batt_dbg("start_record_flag: %d, at %d ms",start_record_flag, jiffies_to_msecs(jiffies));
			charger_finish	 = 1;
			start_record_flag = 0;
			batt_dbg("enter charge timer finish");
		}
	}
#endif

	if(charger_finish)
	{
		if(!gas_gauge_ptr->charge_end)
		{
			if(batt_info_ptr->fRSOC < 100)
			{
				static int32_t fRSOC_extern = 0;			

				if(batt_info_ptr->fRSOC <= rsoc_pre){
					batt_dbg("fRSOC_extern:%d, soc:%d ",fRSOC_extern,batt_info_ptr->fRSOC);

					if (!chgr_full_soc_pursue_start || fRSOC_extern != batt_info_ptr->fRSOC) {
						chgr_full_soc_pursue_start = jiffies;
						fRSOC_extern = batt_info_ptr->fRSOC ;
					}


					chgr_full_soc_pursue_accumulation =
						jiffies_to_msecs(jiffies - chgr_full_soc_pursue_start);

					if (chgr_full_soc_pursue_accumulation >= CHG_END_PURSUE_STEP){

						chgr_full_soc_pursue_start = 0;
						batt_info_ptr->fRSOC++;
						batt_info_ptr->sCaMAH = batt_info_ptr->fRSOC * the_oz8806->batt_info.batt_fcc_data / num_100;
						batt_info_ptr->sCaMAH ++;	
					}
				}

				if(batt_info_ptr->fRSOC > 100) {
					batt_info_ptr->fRSOC = 100;
					batt_info_ptr->sCaMAH = batt_info_ptr->fRSOC * the_oz8806->batt_info.batt_fcc_data / num_100;
					batt_info_ptr->sCaMAH ++;	
				}

				//update fRSOC_extern
				fRSOC_extern = batt_info_ptr->fRSOC ;

				batt_dbg("enter charger finsh update soc:%d",batt_info_ptr->fRSOC);
			}
			else
			{
				batt_dbg("enter charger charge end");
				gas_gauge_ptr->charge_end = 1;

				if (bmu_charge_end_process_callback)
					bmu_charge_end_process_callback();

				charger_finish = 0;
				chgr_full_soc_pursue_start = 0;
			}

		}
		else {
			charger_finish = 0;
			chgr_full_soc_pursue_start = 0;
		}

	} else chgr_full_soc_pursue_start = 0;

}


//this is very important code customer need change
//customer should change charge discharge status according to your system
//O2micro
static void system_charge_discharge_status(struct oz8806_data *data)
{
	int8_t adapter_status_temp = O2_CHARGER_BATTERY;

#if defined (MTK_MACH_SUPPORT)
	batt_dbg("BMT_status.charger_exist: %d",BMT_status.charger_exist);
	if(BMT_status.charger_exist == KAL_TRUE)
	{
        if ( (BMT_status.charger_type == NONSTANDARD_CHARGER) || 
			 (BMT_status.charger_type == STANDARD_CHARGER) )
			adapter_status_temp = O2_CHARGER_AC;
		else if ( (BMT_status.charger_type == STANDARD_HOST) ||
                  (BMT_status.charger_type == CHARGING_HOST) )
			adapter_status_temp = O2_CHARGER_USB;
		else
			adapter_status_temp = O2_CHARGER_AC;
	}
	else
	{
		adapter_status_temp = O2_CHARGER_BATTERY;
	}
#else
//---------------------------------------------
	union power_supply_propval val_ac;
	union power_supply_propval val_usb;

	if (!ac_psy)
		ac_psy = power_supply_get_by_name ("ac"); 

	if (!usb_psy)
		usb_psy= power_supply_get_by_name ("usb"); 

	if (ac_psy && !ac_psy->get_property (ac_psy, POWER_SUPPLY_PROP_ONLINE, &val_ac))
	{
		if (val_ac.intval)
			adapter_status_temp = O2_CHARGER_AC;
	}
	else
		val_ac.intval = 0;

	if (usb_psy && !usb_psy->get_property (usb_psy, POWER_SUPPLY_PROP_ONLINE, &val_usb))
	{
		if (val_usb.intval)
			adapter_status_temp = O2_CHARGER_USB;
	}
	else
		val_usb.intval = 0;

	if (val_ac.intval == 0 && val_usb.intval == 0)
		adapter_status_temp = O2_CHARGER_BATTERY;

#ifdef INTEL_MACH
	if (val_ac.intval || val_usb.intval)
	{
		if (ac_psy && !ac_psy->get_property (ac_psy, POWER_SUPPLY_PROP_CABLE_TYPE, &val_ac))
		{
			if (val_ac.intval == POWER_SUPPLY_TYPE_MAINS)
				adapter_status_temp = O2_CHARGER_AC;
			else
				adapter_status_temp = O2_CHARGER_USB;
		}
		else
			val_ac.intval = 0;
		if (usb_psy && !usb_psy->get_property (usb_psy, POWER_SUPPLY_PROP_CABLE_TYPE, &val_usb))
		{
			if (val_usb.intval == POWER_SUPPLY_TYPE_MAINS)
				adapter_status_temp = O2_CHARGER_AC;
			else
				adapter_status_temp = O2_CHARGER_USB;
		}
		else
			val_usb.intval = 0;

		if (val_ac.intval == 0 && val_usb.intval == 0)
			pr_err("fail to get POWER_SUPPLY_PROP_CABLE_TYPE\n");
	}
//---------------------------------------------
#elif defined (QUALCOMM_MACH_SUPPORT)
//---------------------------------------------
#endif

#endif
//---------------------------------------------	

	adapter_status = adapter_status_temp;
	batt_dbg("adapter_status:%d", adapter_status);
}

#if defined (O2_CHARGER_SUPPORT) && !defined(MTK_MACH_SUPPORT)
static void charging_algo(struct oz8806_data *data)
{
	int cc_current = 0;

	//change charger setting if battery temperature is not normal
	cc_current = temp_adjust_setting_extern(data->batt_info.batt_temp, &(config_data.charge_cv_voltage));
	o2_charging_algo();
}
#endif

static void oz8806_battery_func(struct oz8806_data *data)
{

	unsigned long time_since_last_update_ms;
	static unsigned long cur_jiffies;

	pr_err("%s: !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", __func__);
	time_since_last_update_ms = jiffies_to_msecs(jiffies - cur_jiffies);
	cur_jiffies = jiffies;

	if (create_sys)
		create_sys = get_battery_psy();

	//get charging type: battery, AC, or USB
	system_charge_discharge_status(data);

	rsoc_pre = data->batt_info.batt_soc;

	//you must add this code here for O2MICRO
	//Notice: don't nest mutex

	/**************mutex_lock*********************/
	mutex_lock(&update_mutex);

#ifdef EXT_THERMAL_READ
	oz8806_update_batt_temp(data);
#endif

	if (bmu_polling_loop_callback && !oz8806_suspend)
		bmu_polling_loop_callback();

	oz8806_update_batt_info(data);

	charge_end_fun();

	if(adapter_status == O2_CHARGER_BATTERY)
		discharge_end_fun(data);

	mutex_unlock(&update_mutex);
	/**************mutex_unlock*********************/


#if 1
	if(adapter_status == O2_CHARGER_AC)
	{	if(!wake_lock_active(&battery_wake_lock))
		{
			wake_lock(&battery_wake_lock);
		}
	}
	else
	{
		if(wake_lock_active(&battery_wake_lock))
		{
			wake_unlock(&battery_wake_lock);
		}
	}
#endif

#if defined (O2_CHARGER_SUPPORT) && !defined(MTK_MACH_SUPPORT)
	if (adapter_status != O2_CHARGER_BATTERY)
		charging_algo(data);
	else
		o2_stop_charge_extern(1);
#endif

	batt_dbg("l=%d v=%d t=%d c=%d ch=%d",
			data->batt_info.batt_soc, data->batt_info.batt_voltage, 
			data->batt_info.batt_temp, data->batt_info.batt_current, adapter_status);

#ifdef O2_GAUGE_POWER_SUPPOLY_SUPPORT
	power_supply_changed(&data->bat);
#endif
	
	batt_dbg("since last batt update = %lu ms", time_since_last_update_ms);
}

extern void wake_up_bat_bmu(void); 
static void oz8806_battery_work(struct work_struct *work)
{
	struct oz8806_data *data = container_of(work, struct oz8806_data, work.work);
#ifndef O2_GAUGE_WORK_SUPPORT
	union power_supply_propval val;

	if (bmu_init_done) return;
#endif
	oz8806_battery_func(data);

	/* reschedule for the next time */
#ifndef O2_GAUGE_WORK_SUPPORT
	if (!capacity_init_ok && bmu_init_done) {
#if  defined (MTK_MACH_SUPPORT)
		wake_up_bat_bmu(); //don't use wake_up_dat(), this is for charger
#endif
		do {
			msleep(50);
			if (battery_psy 
				&& !battery_psy->get_property (battery_psy, POWER_SUPPLY_PROP_CAPACITY, &val))
			{
				if (val.intval == data->batt_info.batt_soc)
					capacity_init_ok = 1;
			}
			else
			{
				pr_err(KERN_ERR"fail to get val of POWER_SUPPLY_PROP_CAPACITY\n");
				break;
			}
		} while (!capacity_init_ok);
	}
#endif

	if (!bmu_init_done) 
	{
		data->interval = 2000;
        schedule_delayed_work(&data->work, msecs_to_jiffies(2 * 1000));
	} 
	else 
	{
		data->interval = BATTERY_WORK_INTERVAL * 1000;
#if defined(O2_GAUGE_WORK_SUPPORT)
        schedule_delayed_work(&data->work, msecs_to_jiffies(data->interval));
#endif
	}
	batt_dbg("interval:%d ms", data->interval);
}

static int oz8806_suspend_notifier(struct notifier_block *nb,
				unsigned long event,
				void *dummy)
{
	struct oz8806_data *data = container_of(nb, struct oz8806_data, pm_nb);
	
	switch (event) {

	case PM_SUSPEND_PREPARE:
		pr_err("oz8806 PM_SUSPEND_PREPARE \n");
#ifdef O2_GAUGE_WORK_SUPPORT
		cancel_delayed_work_sync(&data->work);
#endif
			
		system_charge_discharge_status(data);
		oz8806_suspend = 1;
#ifdef AUTO_RESUME_TEST
		if (auto_resume_interval)
			oz8806_setup_alarm();
#endif
		return NOTIFY_OK;
	case PM_POST_SUSPEND:
        pr_err("oz8806 PM_POST_SUSPEND \n");

		system_charge_discharge_status(data);

		mutex_lock(&update_mutex);
		// if AC charge can't wake up every 1 min,you must remove the if.
		if(adapter_status == O2_CHARGER_BATTERY)
			if (bmu_wake_up_chip_callback)
				bmu_wake_up_chip_callback();

		oz8806_update_batt_info(data);
		mutex_unlock(&update_mutex);

#ifdef O2_GAUGE_WORK_SUPPORT	
		schedule_delayed_work(&data->work, 0);
#endif

		//this code must be here,very carefull.
		if(adapter_status == O2_CHARGER_BATTERY)
		{
			if(data->batt_info.batt_current >= data->batt_info.discharge_current_th)
			{
				if (batt_info_ptr) batt_info_ptr->fCurr = -20;

				data->batt_info.batt_current = -20;
				pr_err("drop current\n");
			}
		}

		oz8806_suspend = 0;
		
		return NOTIFY_OK;

	default:
		return NOTIFY_DONE;
	}
}

static int oz8806_init_batt_info(struct oz8806_data *data)
{
	data->batt_info.batt_soc = 50; 
	data->batt_info.batt_voltage = 3999;
	data->batt_info.batt_current = -300;
	data->batt_info.batt_temp = 27;
	data->batt_info.batt_capacity = 4000;

	data->batt_info.batt_fcc_data = config_data.design_capacity;
	data->batt_info.discharge_current_th = DISCH_CURRENT_TH;
	data->batt_info.charge_end = 0;

	return 0;
}
static int oz8806_update_batt_info(struct oz8806_data *data)
{
	int ret = 0;

	//Notice: before call this function, use mutex_lock(&update_mutex)
	//Notice: don't nest mutex
	if (!batt_info_ptr || !gas_gauge_ptr)
	{
		ret = -EINVAL;
		goto end;
	}

	data->batt_info.batt_soc = batt_info_ptr->fRSOC; 
	data->batt_info.batt_voltage = batt_info_ptr->fVolt;
	data->batt_info.batt_current = batt_info_ptr->fCurr;
	data->batt_info.batt_capacity = batt_info_ptr->sCaMAH;

	data->batt_info.charge_end = gas_gauge_ptr->charge_end;
	bmu_init_done = gas_gauge_ptr->bmu_init_ok;

	oz8806_update_batt_temp(data);

end:
	return ret;
}

static int oz8806_update_batt_temp(struct oz8806_data *data)
{
	int ret = 0;

	if (!batt_info_ptr || !gas_gauge_ptr)
	{
		ret = -EINVAL;
		goto end;
	}

#ifdef INTEL_MACH
	data->batt_info.batt_temp = 27;

#elif defined (CONFIG_ARCH_S5PV210)  
#ifdef OZ8806_TEMP_SIMULATION 
	data->batt_info.batt_temp = batt_temp_sim;
#else
	data->batt_info.batt_temp = 27;
#endif

#elif defined (MTK_MACH_SUPPORT) && defined(EXT_THERMAL_READ)
	data->batt_info.batt_temp = (BMT_status.temperature);
	pr_notice("robin: %d\n", data->batt_info.batt_temp);

#elif defined (QUALCOMM_MACH_SUPPORT)
	data->batt_info.batt_temp = ROUND_SINGLE_DIGITS(oz8806_battery_get_temp(data));

#else
	data->batt_info.batt_temp = batt_info_ptr->fCellTemp;
#endif

	if (batt_info_ptr->fCellTemp != data->batt_info.batt_temp)
		batt_info_ptr->fCellTemp = data->batt_info.batt_temp;

end:
	return ret;
}

int oz8806_get_remaincap(void)
{
	int ret = -1;

	mutex_lock(&update_mutex);

	if (batt_info_ptr)
		ret = batt_info_ptr->sCaMAH;

	mutex_unlock(&update_mutex);

	return ret;
}
EXPORT_SYMBOL(oz8806_get_remaincap);

#if defined (MTK_MACH_SUPPORT)
static int rtc_soc = -1;
#endif

int oz8806_get_soc_from_ext(void)
{
	int ret = -1;
#if defined (MTK_MACH_SUPPORT)
	//read soc from RTC.
	//if no back-up battery for RTC, return 0
	if (rtc_soc == -1) //only read one time
		rtc_soc = get_rtc_spare_fg_value();

	// store soc+1 in RTC in order to tell if RTC is reset and SOC is 0.
	if (rtc_soc) ret = rtc_soc - 1;

	batt_dbg("rtc_soc:%d", ret);
#endif
	return ret;
}
EXPORT_SYMBOL(oz8806_get_soc_from_ext);

int oz8806_get_soc(void)
{
	int ret = -1;

	mutex_lock(&update_mutex);

	if (!bmu_init_done)
	{
		ret = -1;
#if defined (MTK_MACH_SUPPORT)
		//read soc from RTC.
		//if no back-up battery for RTC, return 0
		if (rtc_soc == -1) //only read one time
			rtc_soc = get_rtc_spare_fg_value();

		batt_dbg("rtc_soc:%d", rtc_soc);
#endif
	}
	else if (batt_info_ptr)
		ret = batt_info_ptr->fRSOC;

	mutex_unlock(&update_mutex);

	return ret;
}
EXPORT_SYMBOL(oz8806_get_soc);

int oz8806_get_battry_current(void)
{
	int ret = -1;

	if (!bmu_init_done) return -1;

	mutex_lock(&update_mutex);

	if (oz8806_current_read_callback && batt_info_ptr)
		ret = oz8806_current_read_callback(&batt_info_ptr->fCurr);

	if (ret < 0)
	{
		ret = -1;
		batt_dbg("%s:oz8806 current adc error", __func__);
	}

	if (batt_info_ptr && ret >= 0)
		ret = batt_info_ptr->fCurr;

	mutex_unlock(&update_mutex);

	return ret;
}
EXPORT_SYMBOL(oz8806_get_battry_current);

int oz8806_get_battery_voltage(void)
{
	int ret = -1;

	if (!bmu_init_done) return -1;

	mutex_lock(&update_mutex);

	if (oz8806_voltage_read_callback && batt_info_ptr)
		ret = oz8806_voltage_read_callback(&batt_info_ptr->fVolt);

	if (ret < 0)
	{
		ret = -1;
		batt_dbg("%s:oz8806 voltage adc error", __func__);
	}

	if (batt_info_ptr && ret >= 0)
		ret = batt_info_ptr->fVolt;

	mutex_unlock(&update_mutex);

	return ret;
}
EXPORT_SYMBOL(oz8806_get_battery_voltage);

int oz8806_get_battery_temp(void)
{
	int ret = -1;

	if (!bmu_init_done) return -1;

	mutex_lock(&update_mutex);

	if (batt_info_ptr)
		ret = batt_info_ptr->fCellTemp;

	mutex_unlock(&update_mutex);

	return ret;
}
EXPORT_SYMBOL(oz8806_get_battery_temp);

void oz8806_battery_update_data(void)
{
	batt_dbg("enter %s", __func__);
	if (fg_hw_init_done && the_oz8806 && bmu_init_done)
		oz8806_battery_func(the_oz8806);
}
EXPORT_SYMBOL(oz8806_battery_update_data);

int32_t oz8806_vbus_voltage(void)
{
	int32_t vubs_voltage = 0;
	int ret = -1;

	if (oz8806_temp_read_callback)
		ret = oz8806_temp_read_callback(&vubs_voltage);

	if (ret < 0)
	{
		batt_dbg("%s:oz8806 temp adc error", __func__);
		return ret;
	}

	//batt_dbg("voltage from oz8806:%d", vubs_voltage);
	vubs_voltage =  vubs_voltage * (RPULL + RDOWN) / RDOWN;

	return vubs_voltage;
}
EXPORT_SYMBOL(oz8806_vbus_voltage);

int32_t oz8806_get_simulated_temp(void)
{
#ifdef OZ8806_TEMP_SIMULATION 
	return batt_temp_sim;
#else
	return 25;
#endif
}
EXPORT_SYMBOL(oz8806_get_simulated_temp);

int32_t oz8806_get_init_status(void)
{
	return bmu_init_done;
}
EXPORT_SYMBOL(oz8806_get_init_status);
	
int32_t gauge_int_done(void)
{
    return fg_hw_init_done; 
}
EXPORT_SYMBOL(gauge_int_done);

struct i2c_client * oz8806_get_client(void)
{
	if (the_oz8806)
		return the_oz8806->myclient;
	else
	{
		pr_err("the_oz8806 is NULL, oz8806_probe didn't call\n");
		return NULL;

	}
}
EXPORT_SYMBOL(oz8806_get_client);


int8_t get_adapter_status(void)
{
	return adapter_status;
}
EXPORT_SYMBOL(get_adapter_status);

void oz8806_register_bmu_callback(void *bmu_polling_loop_func,
		void *bmu_wake_up_chip_func,
		void *bmu_power_down_chip_func,
		void *charge_end_process_func,
		void *discharge_end_process_func,
		void *oz8806_temp_read_func,
		void *oz8806_current_read_func,
		void *oz8806_voltage_read_func)
{
	mutex_lock(&update_mutex);

	bmu_polling_loop_callback = bmu_polling_loop_func;
	bmu_wake_up_chip_callback = bmu_wake_up_chip_func;
	bmu_power_down_chip_callback = bmu_power_down_chip_func;
	bmu_charge_end_process_callback = charge_end_process_func;
	bmu_discharge_end_process_callback = discharge_end_process_func;

	oz8806_temp_read_callback = oz8806_temp_read_func;
	oz8806_current_read_callback = oz8806_current_read_func;
	oz8806_voltage_read_callback = oz8806_voltage_read_func;

	if(bmu_polling_loop_callback)
	{
		bmu_polling_loop_callback();
		schedule_delayed_work(&the_oz8806->work, 0);
	}

	mutex_unlock(&update_mutex);
}
EXPORT_SYMBOL(oz8806_register_bmu_callback);

void unregister_bmu_callback(void)
{
	mutex_lock(&update_mutex);

	bmu_polling_loop_callback = NULL;
	bmu_wake_up_chip_callback = NULL;
	bmu_power_down_chip_callback = NULL;
	bmu_charge_end_process_callback = NULL;
	bmu_discharge_end_process_callback = NULL;

	oz8806_temp_read_callback = NULL;
	oz8806_current_read_callback = NULL;
	oz8806_voltage_read_callback = NULL;

	oz8806_init_batt_info(the_oz8806);

	batt_info_ptr = NULL;
	gas_gauge_ptr = NULL;

	mutex_unlock(&update_mutex);
}
EXPORT_SYMBOL(unregister_bmu_callback);

void oz8806_set_batt_info_ptr(bmu_data_t  *batt_info)
{
	mutex_lock(&update_mutex);

	if (!batt_info)
	{
		pr_err("%s: batt_info NULL\n", __func__);
		return;
	}

	batt_info_ptr = batt_info;

	mutex_unlock(&update_mutex);
}
EXPORT_SYMBOL(oz8806_set_batt_info_ptr);

void oz8806_set_gas_gauge(gas_gauge_t *gas_gauge)
{
	mutex_lock(&update_mutex);

	if (!gas_gauge)
	{
		pr_err("%s: gas_gauge NULL\n", __func__);
		return;
	}

	gas_gauge_ptr = gas_gauge;

	if (gas_gauge_ptr->fcc_data != 0)
		the_oz8806->batt_info.batt_fcc_data = gas_gauge_ptr->fcc_data;

	if (gas_gauge_ptr->discharge_current_th != 0)
		the_oz8806->batt_info.discharge_current_th = gas_gauge_ptr->discharge_current_th;

	batt_dbg("batt_fcc_data:%d, discharge_current_th:%d", 
			the_oz8806->batt_info.batt_fcc_data, the_oz8806->batt_info.discharge_current_th);

	gas_gauge_ptr->ext_temp_measure = ext_thermal_read;

	mutex_unlock(&update_mutex);
}
EXPORT_SYMBOL(oz8806_set_gas_gauge);

int oz8806_get_save_capacity(void)
{
	return save_capacity;
}
EXPORT_SYMBOL(oz8806_get_save_capacity);

unsigned long oz8806_get_boot_up_time(void)
{
	unsigned long long t;

	t = cpu_clock(smp_processor_id());
	//do_div(t, 1000000000);
	do_div(t, 1000000);
	pr_err("boot up time: %lu ms\n", (unsigned long) t);

	return t;
}
EXPORT_SYMBOL(oz8806_get_boot_up_time);

unsigned long oz8806_get_power_on_time(void)
{
	unsigned int t;

	t = jiffies_to_msecs(jiffies - ic_wakeup_time);

	pr_err("IC wakeup time: %u ms\n", t);

	return (unsigned long)t;
}
EXPORT_SYMBOL(oz8806_get_power_on_time);

#ifdef AUTO_RESUME_TEST
static void auto_resume_work_func(struct work_struct *work)
{
	getnstimeofday(&ts);
	pr_err("%s, ts.tv_sec: %d\n", __func__, ts.tv_sec);
	if(wake_lock_active(&auto_resume_wake_lock))
	{
		wake_unlock(&auto_resume_wake_lock);
		pr_err("%s:wake_unlock\n", __func__);
	}
}
static void auto_resume_callback(struct alarm *alarm)
{
	//struct oz8806_data *data = container_of(alarm, struct oz8806_data, &auto_resume_alarm);
	getnstimeofday(&ts);
	pr_err("%s, ts.tv_sec: %d\n", __func__, ts.tv_sec);

	if(!wake_lock_active(&auto_resume_wake_lock))
	{
		wake_lock(&auto_resume_wake_lock);
		pr_err("%s: wake_lock \n", __func__);
	}

	schedule_delayed_work(&auto_resume_work, msecs_to_jiffies(30000)); //30s
}

void oz8806_setup_alarm(void)
{
	getnstimeofday(&ts);
	pr_err("%s, ts.tv_sec: %d, wakeup after %ds \n",
			__func__, ts.tv_sec, auto_resume_interval);
	ts.tv_sec += auto_resume_interval;

	alarm_start_range(&auto_resume_alarm,
			timespec_to_ktime(ts),

			timespec_to_ktime(ts));
}
#endif

static int oz8806_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	int ret;
	struct oz8806_data *data;

	data = devm_kzalloc(&client->dev, sizeof(struct oz8806_data), GFP_KERNEL);
	if (!data) {
		dev_err(&client->dev, "Can't alloc oz8806_data struct\n");
		return -ENOMEM;
	}

	// Init real i2c_client 
	i2c_set_clientdata(client, data);

	the_oz8806 = data;
	data->myclient = client;
#ifdef INTEL_MACH
	data->myclient->addr = 0x2f;
#endif
	data->interval = BATTERY_WORK_INTERVAL * 1000;

	//init battery information
	oz8806_init_batt_info(data);

	/*
	 * Wake up oz8806 to full power mode here.
	 * Then, after at least 2.5s, detection of current, voltage, and temperature can be stable
	 *
	 * when IC start with start-up mode, after at least 2s, IC can enter Full Power Mode.
	 */
	if (oz8806_get_boot_up_time() < 2000)
		ic_wakeup_time = jiffies + msecs_to_jiffies(2000 - oz8806_get_boot_up_time());
	else
		ic_wakeup_time = jiffies;

	oz8806_wakeup_full_power();

#ifdef O2_GAUGE_POWER_SUPPOLY_SUPPORT 
	oz8806_powersupply_init(data);
	
	ret = power_supply_register(&client->dev, &the_oz8806->bat);
	
	if (ret) {
		pr_err("failed to register power_supply battery\n");
		goto register_fail;
	}
#endif

	/*
	 * /sys/class/i2c-dev/i2c-2/device/2-002f/
	 */
	ret = oz8806_create_sys(&(client->dev), &oz8806_attribute_group);
	if(ret){
		pr_err("[BATT]: Err failed to creat oz8806 attributes\n");
		goto bat_failed;
	}

	batt_dbg("oz8806 oz8806_probe 2");
	
   //alternative suspend/resume method
	data->pm_nb.notifier_call = oz8806_suspend_notifier;
	register_pm_notifier(&data->pm_nb);

	wake_lock_init(&battery_wake_lock, WAKE_LOCK_SUSPEND, "oz8806_wake");

	//just for test
	//wake_lock(&battery_wake_lock);

#ifdef EXT_THERMAL_READ
	ext_thermal_read = 1;
#else
	ext_thermal_read = 0;
#endif

    fg_hw_init_done = 1; 

	INIT_DELAYED_WORK(&data->work, oz8806_battery_work);
	//schedule_work(&data->work.work);
#ifdef AUTO_RESUME_TEST
	wake_lock_init(&auto_resume_wake_lock, WAKE_LOCK_SUSPEND, "auto_resume_wake");
	INIT_DELAYED_WORK(&auto_resume_work, auto_resume_work_func);
	alarm_init(&auto_resume_alarm, ANDROID_ALARM_RTC_WAKEUP,
			            auto_resume_callback);
#endif

	return 0;					//return Ok
bat_failed:
#ifdef O2_GAUGE_POWER_SUPPOLY_SUPPORT 
	power_supply_unregister(&the_oz8806->bat);
register_fail:
#endif
	pr_err("%s() fail: return %d\n", __func__, ret);
	the_oz8806 = NULL;
	return ret;
}


static int oz8806_remove(struct i2c_client *client)
{
	struct oz8806_data *data = i2c_get_clientdata(client);

#ifdef O2_GAUGE_WORK_SUPPORT
	cancel_delayed_work(&data->work);
#endif

	if (battery_psy && !create_sys)
		sysfs_remove_group(&(battery_psy->dev->kobj), &battery_psy_attribute_group);

	sysfs_remove_group(&(data->myclient->dev.kobj), &oz8806_attribute_group);

#ifdef O2_GAUGE_POWER_SUPPOLY_SUPPORT 
	power_supply_unregister(&data->bat);
#endif

	mutex_lock(&update_mutex);

	if (bmu_power_down_chip_callback)
		bmu_power_down_chip_callback();

	oz8806_update_batt_info(data);

	mutex_unlock(&update_mutex);

	return 0;
}

static void oz8806_shutdown(struct i2c_client *client)
{
	struct oz8806_data *data = i2c_get_clientdata(client);
	pr_err("oz8806 shutdown\n");

#ifdef O2_GAUGE_WORK_SUPPORT
	cancel_delayed_work(&data->work);
#endif

	if (battery_psy && !create_sys)
		sysfs_remove_group(&(battery_psy->dev->kobj), &battery_psy_attribute_group);

	sysfs_remove_group(&(data->myclient->dev.kobj), &oz8806_attribute_group);

#ifdef O2_GAUGE_POWER_SUPPOLY_SUPPORT 
	power_supply_unregister(&data->bat);
#endif
	mutex_lock(&update_mutex);
	if (bmu_power_down_chip_callback)
		bmu_power_down_chip_callback();

	oz8806_update_batt_info(data);
	mutex_unlock(&update_mutex);
}
/*-------------------------------------------------------------------------*/

static const struct i2c_device_id oz8806_id[] = {
#ifdef INTEL_MACH
	{"EXCG0000", 0},
#else
	{ MYDRIVER, 0 },							//string, id??
#endif
	{ }
};
MODULE_DEVICE_TABLE(i2c, oz8806_id);

#ifdef CONFIG_ACPI //this defined by INTEL MACH
static struct acpi_device_id extgauge_acpi_match[] = {
        {"EXFG0000", 0 },
        {}
};
MODULE_DEVICE_TABLE(acpi, extgauge_acpi_match);
#endif

#ifdef CONFIG_OF
static const struct of_device_id oz8806_of_match[] = {
	{.compatible = "o2micro,oz8806",},
	{},
};

MODULE_DEVICE_TABLE(of, oz8806_of_match);
#else
static struct i2c_board_info __initdata i2c_oz8806={ I2C_BOARD_INFO("oz8806", 0x2f)};
#endif

static struct i2c_driver oz8806_driver = {
	.driver = {
		.name	= MYDRIVER,
#ifdef CONFIG_ACPI
		.acpi_match_table = ACPI_PTR(extgauge_acpi_match),
#endif
#ifdef CONFIG_OF
		.of_match_table = oz8806_of_match,
#endif
	},
	.probe			= oz8806_probe,
	//.resume			= oz8806_resume,
	.remove         = oz8806_remove,
	.shutdown		= oz8806_shutdown,
	.id_table		= oz8806_id,
};


static int __init oz8806_init(void)
{
	int ret = 0;
#ifndef CONFIG_OF
	struct i2c_adapter *i2c_adap;
	static struct i2c_client *oz8806_client; //Notice: this is static pointer

	pr_err("%s\n", __func__);

	i2c_adap = i2c_get_adapter(OZ88106_I2C_NUM);
	if (i2c_adap)
	{
		pr_err("create I2C devices\n");
		oz8806_client = i2c_new_device(i2c_adap, &i2c_oz8806);
		i2c_put_adapter(i2c_adap);
	}
	else {
		pr_err("failed to get adapter %d.\n", OZ88106_I2C_NUM);
		pr_err("statically declare I2C devices\n");

		ret = i2c_register_board_info(OZ88106_I2C_NUM, &i2c_oz8806, 1);

		if(ret != 0)
			pr_err("failed to register i2c_board_info.\n");
	}
#endif

	ret = i2c_add_driver(&oz8806_driver);

    if(ret != 0)
        pr_err("failed to register oz8806 i2c driver.\n");
    else
        pr_err("Success to register oz8806 i2c driver.\n");

	return ret;
}

static void __exit oz8806_exit(void)
{
	pr_err("%s\n", __func__);
	i2c_del_driver(&oz8806_driver);
}

/*-------------------------------------------------------------------------*/

MODULE_DESCRIPTION("oz8806 Battery Monitor IC Driver");
MODULE_LICENSE("GPL");

//subsys_initcall_sync(oz8806_init);
module_init(oz8806_init);
module_exit(oz8806_exit);

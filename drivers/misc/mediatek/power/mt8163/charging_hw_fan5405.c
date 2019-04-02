#include <linux/types.h>
#include <mt-plat/charging.h>
#include <mt-plat/upmu_common.h>
#include <linux/delay.h>
#include <linux/reboot.h>
#include <mt-plat/mt_boot.h>
#include <mt-plat/battery_meter.h>
#include <mach/mt_battery_meter.h>
#include <mach/mt_charging.h>
#include <mach/mt_pmic.h>
#include "fan5405.h"
#include <mt-plat/mt_gpio.h>
#include <mt-plat/charging.h>
#include <mt-plat/battery_common.h>

#ifdef CONFIG_MTK_DUAL_INPUT_CHARGER_SUPPORT
#include <mt-plat/diso.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#ifdef MTK_DISCRETE_SWITCH
#include <mt-plat/mt_gpio.h>
#endif
#if !defined(MTK_AUXADC_IRQ_SUPPORT)
#include <linux/kthread.h>
#include <linux/wakelock.h>
#include <linux/mutex.h>
#include <linux/hrtimer.h>
#include <linux/workqueue.h>
#endif
#endif
#include <linux/gpio.h>   // tongjun@mid Add for __gpio_get_value()
/* ============================================================ // */
/* Define */
/* ============================================================ // */
#define STATUS_OK	0
#define STATUS_FAIL	1
#define STATUS_UNSUPPORTED	-1
#define GETARRAYNUM(array) (sizeof(array)/sizeof(array[0]))


/* ============================================================ // */
/* Global variable */
/* ============================================================ // */

#if defined(MTK_WIRELESS_CHARGER_SUPPORT)
#define WIRELESS_CHARGER_EXIST_STATE 0
int wireless_charger_gpio_number = (168 | 0x80000000);
#endif

const u32 VBAT_CV_VTH[] = {
	BATTERY_VOLT_03_500000_V, BATTERY_VOLT_03_520000_V, BATTERY_VOLT_03_540000_V,
	    BATTERY_VOLT_03_560000_V,
	BATTERY_VOLT_03_580000_V, BATTERY_VOLT_03_600000_V, BATTERY_VOLT_03_620000_V,
	    BATTERY_VOLT_03_640000_V,
	BATTERY_VOLT_03_660000_V, BATTERY_VOLT_03_680000_V, BATTERY_VOLT_03_700000_V,
	    BATTERY_VOLT_03_720000_V,
	BATTERY_VOLT_03_740000_V, BATTERY_VOLT_03_760000_V, BATTERY_VOLT_03_780000_V,
	    BATTERY_VOLT_03_800000_V,
	BATTERY_VOLT_03_820000_V, BATTERY_VOLT_03_840000_V, BATTERY_VOLT_03_860000_V,
	    BATTERY_VOLT_03_880000_V,
	BATTERY_VOLT_03_900000_V, BATTERY_VOLT_03_920000_V, BATTERY_VOLT_03_940000_V,
	    BATTERY_VOLT_03_960000_V,
	BATTERY_VOLT_03_980000_V, BATTERY_VOLT_04_000000_V, BATTERY_VOLT_04_020000_V,
	    BATTERY_VOLT_04_040000_V,
	BATTERY_VOLT_04_060000_V, BATTERY_VOLT_04_080000_V, BATTERY_VOLT_04_100000_V,
	    BATTERY_VOLT_04_120000_V,
	BATTERY_VOLT_04_140000_V, BATTERY_VOLT_04_160000_V, BATTERY_VOLT_04_180000_V,
	    BATTERY_VOLT_04_200000_V,
	BATTERY_VOLT_04_220000_V, BATTERY_VOLT_04_240000_V, BATTERY_VOLT_04_260000_V,
	    BATTERY_VOLT_04_280000_V,
	BATTERY_VOLT_04_300000_V, BATTERY_VOLT_04_320000_V, BATTERY_VOLT_04_340000_V,
	    BATTERY_VOLT_04_360000_V,
	BATTERY_VOLT_04_380000_V, BATTERY_VOLT_04_400000_V, BATTERY_VOLT_04_420000_V,
	    BATTERY_VOLT_04_440000_V
};

#ifdef CONFIG_SUPPORT_HL7007
const u32 CS_VTH[] = {
	CHARGE_CURRENT_550_00_MA, CHARGE_CURRENT_650_00_MA, CHARGE_CURRENT_750_00_MA, CHARGE_CURRENT_850_00_MA,
	CHARGE_CURRENT_1050_00_MA, CHARGE_CURRENT_1150_00_MA, CHARGE_CURRENT_1350_00_MA, CHARGE_CURRENT_1450_00_MA,
	CHARGE_CURRENT_1500_00_MA, CHARGE_CURRENT_1650_00_MA, CHARGE_CURRENT_1750_00_MA, CHARGE_CURRENT_1825_00_MA,
	CHARGE_CURRENT_2050_00_MA, CHARGE_CURRENT_2150_00_MA, CHARGE_CURRENT_2350_00_MA, CHARGE_CURRENT_2500_00_MA
};
#else
const u32 CS_VTH[] = {
	CHARGE_CURRENT_550_00_MA, CHARGE_CURRENT_650_00_MA, CHARGE_CURRENT_750_00_MA,
	    CHARGE_CURRENT_850_00_MA,
	CHARGE_CURRENT_1050_00_MA, CHARGE_CURRENT_1150_00_MA, CHARGE_CURRENT_1350_00_MA,
	    CHARGE_CURRENT_1500_00_MA
};
#endif

const u32 INPUT_CS_VTH[] = {
	CHARGE_CURRENT_100_00_MA, CHARGE_CURRENT_500_00_MA, CHARGE_CURRENT_800_00_MA,
	    CHARGE_CURRENT_MAX
};

const u32 VCDT_HV_VTH[] = {
	BATTERY_VOLT_04_200000_V, BATTERY_VOLT_04_250000_V, BATTERY_VOLT_04_300000_V,
	    BATTERY_VOLT_04_350000_V,
	BATTERY_VOLT_04_400000_V, BATTERY_VOLT_04_450000_V, BATTERY_VOLT_04_500000_V,
	    BATTERY_VOLT_04_550000_V,
	BATTERY_VOLT_04_600000_V, BATTERY_VOLT_06_000000_V, BATTERY_VOLT_06_500000_V,
	    BATTERY_VOLT_07_000000_V,
	BATTERY_VOLT_07_500000_V, BATTERY_VOLT_08_500000_V, BATTERY_VOLT_09_500000_V,
	    BATTERY_VOLT_10_500000_V
};

#if defined(CONFIG_MTK_DUAL_INPUT_CHARGER_SUPPORT)
#if !defined(MTK_AUXADC_IRQ_SUPPORT)
#define SW_POLLING_PERIOD 100 /*100 ms*/
#define MSEC_TO_NSEC(x)		(x * 1000000UL)

static DEFINE_MUTEX(diso_polling_mutex);
static DECLARE_WAIT_QUEUE_HEAD(diso_polling_thread_wq);
static struct hrtimer diso_kthread_timer;
static kal_bool diso_thread_timeout = KAL_FALSE;
static struct delayed_work diso_polling_work;
static void diso_polling_handler(struct work_struct *work);
static DISO_Polling_Data DISO_Polling;
#else
DISO_IRQ_Data DISO_IRQ;
#endif
int g_diso_state	= 0;

static char *DISO_state_s[8] = {
		  "IDLE",
		  "OTG_ONLY",
		  "USB_ONLY",
		  "USB_WITH_OTG",
		  "DC_ONLY",
		  "DC_WITH_OTG",
		  "DC_WITH_USB",
		  "DC_USB_OTG",
};
#endif
static unsigned int charging_error;
//static unsigned int charging_get_error_state(void);
static unsigned int charging_set_error_state(void *data);
u32 charging_value_to_parameter(const u32 *parameter, const u32 array_size, const u32 val)
{
	if (val < array_size)
		return parameter[val];
	battery_log(BAT_LOG_CRTI, "Can't find the parameter \r\n");
	return parameter[0];
}

u32 charging_parameter_to_value(const u32 *parameter, const u32 array_size, const u32 val)
{
	u32 i;

	for (i = 0; i < array_size; i++)
		if (val == *(parameter + i))
			return i;

	battery_log(BAT_LOG_CRTI, "NO register value match \r\n");

	return 0;
}


static u32 bmt_find_closest_level(const u32 *pList, u32 number, u32 level)
{
	u32 i;
	u32 max_value_in_last_element;

	if (pList[0] < pList[1])
		max_value_in_last_element = KAL_TRUE;
	else
		max_value_in_last_element = KAL_FALSE;

	if (max_value_in_last_element == KAL_TRUE) {
		for (i = (number - 1); i != 0; i--)	/* max value in the last element */
			if (pList[i] <= level)
				return pList[i];

		battery_log(BAT_LOG_CRTI, "Can't find closest level, small value first \r\n");
		return pList[0];
		/* return CHARGE_CURRENT_0_00_MA; */
	} else {
		for (i = 0; i < number; i++)	/* max value in the first element */
			if (pList[i] <= level)
				return pList[i];

		battery_log(BAT_LOG_CRTI, "Can't find closest level, large value first \r\n");
		return pList[number - 1];
		/* return CHARGE_CURRENT_0_00_MA; */
	}
}

static u32 charging_hw_init(void *data)
{
	u32 status = STATUS_OK;
	static bool charging_init_flag = KAL_FALSE;

#if defined(MTK_WIRELESS_CHARGER_SUPPORT)
	mt_set_gpio_mode(wireless_charger_gpio_number, 0);	/* 0:GPIO mode */
	mt_set_gpio_dir(wireless_charger_gpio_number, 0);	/* 0: input, 1: output */
#endif

	fan5405_reg_config_interface(0x06, 0x70);

	fan5405_reg_config_interface(0x00, 0xC0);	/* kick chip watch dog */
	fan5405_reg_config_interface(0x01, 0xf8);	//0xf8  // 0xb8 /* TE=1, CE=0, HZ_MODE=0, OPA_MODE=0 */
	fan5405_reg_config_interface(0x05, 0x03);    //0x02  // 0x03
	if (!charging_init_flag) {
		fan5405_reg_config_interface(0x04, 0x1A);	/* 146mA */
		charging_init_flag = KAL_TRUE;
	}
	return status;
}


static u32 charging_dump_register(void *data)
{
	u32 status = STATUS_OK;

	fan5405_dump_register();

	return status;
}


static u32 charging_enable(void *data)
{
	u32 status = STATUS_OK;
	u32 enable = *(u32 *) (data);

	if (KAL_TRUE == enable) {
		fan5405_set_ce(0);
		fan5405_set_hz_mode(0);
		fan5405_set_opa_mode(0);
	} else {

#if defined(CONFIG_USB_MTK_HDRC_HCD)
		if (mt_usb_is_device())
#endif

			fan5405_set_ce(1);
	}

	return status;
}


static u32 charging_set_cv_voltage(void *data)
{
	u32 status = STATUS_OK;
	u16 register_value;

	register_value =
	    charging_parameter_to_value(VBAT_CV_VTH, GETARRAYNUM(VBAT_CV_VTH), *(u32 *) (data));
	fan5405_set_oreg(register_value);

	return status;
}


static u32 charging_get_current(void *data)
{
	u32 status = STATUS_OK;
	u32 array_size;
	u8 reg_value;

	/* Get current level */
	array_size = GETARRAYNUM(CS_VTH);
	fan5405_read_interface(0x1, &reg_value, 0x3, 0x6);	/* IINLIM */
	*(u32 *) data = charging_value_to_parameter(CS_VTH, array_size, reg_value);

	return status;
}



static u32 charging_set_current(void *data)
{
	u32 status = STATUS_OK;
	u32 set_chr_current;
	u32 array_size;
	u32 register_value;
	u32 current_value = *(u32 *) data;
	if (current_value <= CHARGE_CURRENT_350_00_MA) {
		fan5405_set_io_level(1);
	} else {
		fan5405_set_io_level(0);
		array_size = GETARRAYNUM(CS_VTH);
		set_chr_current = bmt_find_closest_level(CS_VTH, array_size, current_value);
		register_value = charging_parameter_to_value(CS_VTH, array_size, set_chr_current);
		fan5405_set_iocharge(register_value);
	}
	return status;
}


static u32 charging_set_input_current(void *data)
{
	u32 status = STATUS_OK;
	u32 set_chr_current;
	u32 array_size;
	u32 register_value;
	if (*(u32 *) data > CHARGE_CURRENT_500_00_MA) {
		register_value = 0x3;
	} else {
		array_size = GETARRAYNUM(INPUT_CS_VTH);
		set_chr_current = bmt_find_closest_level(INPUT_CS_VTH, array_size, *(u32 *) data);
		register_value =
		    charging_parameter_to_value(INPUT_CS_VTH, array_size, set_chr_current);
	}

	fan5405_set_input_charging_current(register_value);

	return status;
}


static u32 charging_get_charging_status(void *data)
{
	u32 status = STATUS_OK;
	u32 ret_val;

	ret_val = fan5405_get_chip_status();

	if (ret_val == 0x2)
		*(u32 *) data = KAL_TRUE;
	else
		*(u32 *) data = KAL_FALSE;

	return status;
}


static u32 charging_reset_watch_dog_timer(void *data)
{
	u32 status = STATUS_OK;

	fan5405_set_tmr_rst(1);

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
	 upmu_set_rg_vcdt_hv_vth(register_value);
	return status;
}


static u32 charging_get_hv_status(void *data)
{
	u32 status = STATUS_OK;

#if defined(CONFIG_POWER_EXT) || defined(CONFIG_MTK_FPGA)
	*(kal_bool *) (data) = 0;
#else
	   *(kal_bool*)(data) = upmu_get_rgs_vcdt_hv_det();
#endif
	return status;
}


static u32 charging_get_battery_status(void *data)
{
	unsigned int status = STATUS_OK;

#if 1 //defined(CONFIG_POWER_EXT) || defined(CONFIG_MTK_FPGA)
	*(kal_bool *) (data) = 0;	/* battery exist */
	battery_log(BAT_LOG_FULL, "[charging_get_battery_status] battery exist for bring up.\n");
#else
	unsigned int val = 0;

	val = pmic_get_register_value(PMIC_BATON_TDET_EN);
	battery_log(BAT_LOG_FULL, "[charging_get_battery_status] BATON_TDET_EN = %d\n", val);
	if (val) {
		pmic_set_register_value(PMIC_BATON_TDET_EN, 1);
		pmic_set_register_value(PMIC_RG_BATON_EN, 1);
		*(kal_bool *) (data) = pmic_get_register_value(PMIC_RGS_BATON_UNDET);
	} else {
		*(kal_bool *) (data) = KAL_FALSE;
	}
#endif

	return status;
}


static u32 charging_get_charger_det_status(void *data)
{
	unsigned int status = STATUS_OK;
	unsigned int val = 0;

#if defined(CONFIG_POWER_EXT) || defined(CONFIG_MTK_FPGA)
	val = 1;
	battery_log(BAT_LOG_CRTI, "[charging_get_charger_det_status] chr exist for fpga.\n");
#else
		val = upmu_get_rgs_chrdet();
#endif

	*(kal_bool *) (data) = val;

	return status;
}

static u32 charging_get_charger_type(void *data)
{
	u32 status = STATUS_OK;

#if defined(CONFIG_POWER_EXT) || defined(CONFIG_MTK_FPGA)
	*(CHARGER_TYPE *) (data) = STANDARD_HOST;
#else
	*(CHARGER_TYPE *) (data) = hw_charging_get_charger_type();
#endif

	return status;
}

static u32 charging_get_is_pcm_timer_trigger(void *data)
{
	u32 status = STATUS_OK;
/* M migration
	if (slp_get_wake_reason() == WR_PCM_TIMER)
		*(kal_bool *) (data) = KAL_TRUE;
	else
		*(kal_bool *) (data) = KAL_FALSE;
	battery_log(BAT_LOG_CRTI, "slp_get_wake_reason=%d\n", slp_get_wake_reason());
*/
	*(kal_bool *)(data) = KAL_FALSE;
	return status;
}

static u32 charging_set_platform_reset(void *data)
{
	unsigned int status = STATUS_OK;

#if defined(CONFIG_POWER_EXT) || defined(CONFIG_MTK_FPGA)
#else
	battery_log(BAT_LOG_CRTI, "charging_set_platform_reset\n");

	kernel_restart("battery service reboot system");
	/* arch_reset(0,NULL); */
#endif

	return status;
}

static u32 charging_get_platform_boot_mode(void *data)
{
	unsigned int status = STATUS_OK;

#if defined(CONFIG_POWER_EXT) || defined(CONFIG_MTK_FPGA)
#else
	*(unsigned int *) (data) = get_boot_mode();

	battery_log(BAT_LOG_CRTI, "get_boot_mode=%d\n", get_boot_mode());
#endif

	return status;
}

static u32 charging_set_power_off(void *data)
{
	unsigned int status = STATUS_OK;

#if defined(CONFIG_POWER_EXT) || defined(CONFIG_MTK_FPGA)
#else
	battery_log(BAT_LOG_CRTI, "charging_set_power_off\n");
	kernel_power_off();
#endif

	return status;
}

static u32 charging_get_power_source(void *data)
{
	u32 status = STATUS_UNSUPPORTED;

	return status;
}

static u32 charging_get_csdac_full_flag(void *data)
{
	return STATUS_UNSUPPORTED;
}

static u32 charging_set_ta_current_pattern(void *data)
{
	return STATUS_UNSUPPORTED;
}

#if defined(CONFIG_MTK_DUAL_INPUT_CHARGER_SUPPORT)
void set_vusb_auxadc_irq(bool enable, bool flag)
{
#if !defined(MTK_AUXADC_IRQ_SUPPORT)
	hrtimer_cancel(&diso_kthread_timer);

	DISO_Polling.reset_polling = KAL_TRUE;
	DISO_Polling.vusb_polling_measure.notify_irq_en = enable;
	DISO_Polling.vusb_polling_measure.notify_irq = flag;

	hrtimer_start(&diso_kthread_timer, ktime_set(0, MSEC_TO_NSEC(SW_POLLING_PERIOD)), HRTIMER_MODE_REL);
#else
	unsigned short threshold = 0;

	if (enable) {
		if (flag == 0)
			threshold = DISO_IRQ.vusb_measure_channel.falling_threshold;
		else
			threshold = DISO_IRQ.vusb_measure_channel.rising_threshold;

		threshold = (threshold * R_DISO_VBUS_PULL_DOWN)/(R_DISO_VBUS_PULL_DOWN + R_DISO_VBUS_PULL_UP);
		mt_auxadc_enableBackgroundDection(DISO_IRQ.vusb_measure_channel.number, threshold,
		DISO_IRQ.vusb_measure_channel.period, DISO_IRQ.vusb_measure_channel.debounce, flag);
	} else {
		mt_auxadc_disableBackgroundDection(DISO_IRQ.vusb_measure_channel.number);
	}
#endif
	pr_debug(" [%s] enable: %d, flag: %d!\n", __func__, enable, flag);
}

void set_vdc_auxadc_irq(bool enable, bool flag)
{
#if !defined(MTK_AUXADC_IRQ_SUPPORT)
	hrtimer_cancel(&diso_kthread_timer);

	DISO_Polling.reset_polling = KAL_TRUE;
	DISO_Polling.vdc_polling_measure.notify_irq_en = enable;
	DISO_Polling.vdc_polling_measure.notify_irq = flag;

	hrtimer_start(&diso_kthread_timer, ktime_set(0, MSEC_TO_NSEC(SW_POLLING_PERIOD)), HRTIMER_MODE_REL);
#else
	unsigned short threshold = 0;

	if (enable) {
		if (flag == 0)
			threshold = DISO_IRQ.vdc_measure_channel.falling_threshold;
		else
			threshold = DISO_IRQ.vdc_measure_channel.rising_threshold;

		threshold = (threshold * R_DISO_DC_PULL_DOWN)/(R_DISO_DC_PULL_DOWN + R_DISO_DC_PULL_UP);
		mt_auxadc_enableBackgroundDection(DISO_IRQ.vdc_measure_channel.number, threshold,
		DISO_IRQ.vdc_measure_channel.period, DISO_IRQ.vdc_measure_channel.debounce, flag);
	} else {
		mt_auxadc_disableBackgroundDection(DISO_IRQ.vdc_measure_channel.number);
	}
#endif
	pr_debug(" [%s] enable: %d, flag: %d!\n", __func__, enable, flag);
}

#if !defined(MTK_AUXADC_IRQ_SUPPORT)
static void diso_polling_handler(struct work_struct *work)
{
	int trigger_channel = -1;
	int trigger_flag = -1;

	if (DISO_Polling.vdc_polling_measure.notify_irq_en)
		trigger_channel = AP_AUXADC_DISO_VDC_CHANNEL;
	else if (DISO_Polling.vusb_polling_measure.notify_irq_en)
		trigger_channel = AP_AUXADC_DISO_VUSB_CHANNEL;

	pr_debug("[DISO]auxadc handler triggered\n");
	switch (trigger_channel) {
	case AP_AUXADC_DISO_VDC_CHANNEL:
		trigger_flag = DISO_Polling.vdc_polling_measure.notify_irq;
		pr_debug("[DISO]VDC IRQ triggered, channel ==%d, flag ==%d\n", trigger_channel, trigger_flag);
#ifdef MTK_DISCRETE_SWITCH /*for DSC DC plugin handle */
		set_vdc_auxadc_irq(DISO_IRQ_DISABLE, 0);
		set_vusb_auxadc_irq(DISO_IRQ_DISABLE, 0);
		set_vusb_auxadc_irq(DISO_IRQ_ENABLE, DISO_IRQ_FALLING);
		if (trigger_flag == DISO_IRQ_RISING) {
			DISO_data.diso_state.pre_vusb_state  = DISO_ONLINE;
			DISO_data.diso_state.pre_vdc_state  = DISO_OFFLINE;
			DISO_data.diso_state.pre_otg_state  = DISO_OFFLINE;
			DISO_data.diso_state.cur_vusb_state  = DISO_ONLINE;
			DISO_data.diso_state.cur_vdc_state  = DISO_ONLINE;
			DISO_data.diso_state.cur_otg_state  = DISO_OFFLINE;
			pr_debug(" cur diso_state is %s!\n", DISO_state_s[2]);
		}
#else /*for load switch OTG leakage handle*/
		set_vdc_auxadc_irq(DISO_IRQ_ENABLE, (~trigger_flag) & 0x1);
		if (trigger_flag == DISO_IRQ_RISING) {
			DISO_data.diso_state.pre_vusb_state  = DISO_OFFLINE;
			DISO_data.diso_state.pre_vdc_state  = DISO_OFFLINE;
			DISO_data.diso_state.pre_otg_state  = DISO_ONLINE;
			DISO_data.diso_state.cur_vusb_state  = DISO_OFFLINE;
			DISO_data.diso_state.cur_vdc_state  = DISO_ONLINE;
			DISO_data.diso_state.cur_otg_state  = DISO_ONLINE;
			pr_debug(" cur diso_state is %s!\n", DISO_state_s[5]);
		} else if (trigger_flag == DISO_IRQ_FALLING) {
			DISO_data.diso_state.pre_vusb_state  = DISO_OFFLINE;
			DISO_data.diso_state.pre_vdc_state  = DISO_ONLINE;
			DISO_data.diso_state.pre_otg_state  = DISO_ONLINE;
			DISO_data.diso_state.cur_vusb_state  = DISO_OFFLINE;
			DISO_data.diso_state.cur_vdc_state  = DISO_OFFLINE;
			DISO_data.diso_state.cur_otg_state  = DISO_ONLINE;
			pr_debug(" cur diso_state is %s!\n", DISO_state_s[1]);
		} else
			pr_debug("[%s] wrong trigger flag!\n", __func__);
#endif
		break;
	case AP_AUXADC_DISO_VUSB_CHANNEL:
		trigger_flag = DISO_Polling.vusb_polling_measure.notify_irq;
		pr_debug("[DISO]VUSB IRQ triggered, channel ==%d, flag ==%d\n", trigger_channel, trigger_flag);
		set_vdc_auxadc_irq(DISO_IRQ_DISABLE, 0);
		set_vusb_auxadc_irq(DISO_IRQ_DISABLE, 0);
		if (trigger_flag == DISO_IRQ_FALLING) {
			DISO_data.diso_state.pre_vusb_state  = DISO_ONLINE;
			DISO_data.diso_state.pre_vdc_state  = DISO_ONLINE;
			DISO_data.diso_state.pre_otg_state  = DISO_OFFLINE;
			DISO_data.diso_state.cur_vusb_state  = DISO_OFFLINE;
			DISO_data.diso_state.cur_vdc_state  = DISO_ONLINE;
			DISO_data.diso_state.cur_otg_state  = DISO_OFFLINE;
			pr_debug(" cur diso_state is %s!\n", DISO_state_s[4]);
		} else if (trigger_flag == DISO_IRQ_RISING) {
			DISO_data.diso_state.pre_vusb_state  = DISO_OFFLINE;
			DISO_data.diso_state.pre_vdc_state  = DISO_ONLINE;
			DISO_data.diso_state.pre_otg_state  = DISO_OFFLINE;
			DISO_data.diso_state.cur_vusb_state  = DISO_ONLINE;
			DISO_data.diso_state.cur_vdc_state  = DISO_ONLINE;
			DISO_data.diso_state.cur_otg_state  = DISO_OFFLINE;
			pr_debug(" cur diso_state is %s!\n", DISO_state_s[6]);
		} else
			pr_debug("[%s] wrong trigger flag!\n", __func__);
			set_vusb_auxadc_irq(DISO_IRQ_ENABLE, (~trigger_flag)&0x1);
		break;
	default:
		set_vdc_auxadc_irq(DISO_IRQ_DISABLE, 0);
		set_vusb_auxadc_irq(DISO_IRQ_DISABLE, 0);
		pr_debug("[DISO]VUSB auxadc IRQ triggered ERROR OR TEST\n");
		return; /* in error or unexecpt state just return */
	}

	g_diso_state = *(int *)&DISO_data.diso_state;
	pr_debug("[DISO]g_diso_state: 0x%x\n", g_diso_state);
	DISO_data.irq_callback_func(0, NULL);
}
#else
static irqreturn_t diso_auxadc_irq_handler(int irq, void *dev_id)
{
	int trigger_channel = -1;
	int trigger_flag = -1;

	trigger_channel = mt_auxadc_getCurrentChannel();
	pr_debug("[DISO]auxadc handler triggered\n");
	switch (trigger_channel) {
	case AP_AUXADC_DISO_VDC_CHANNEL:
		trigger_flag = mt_auxadc_getCurrentTrigger();
		pr_debug("[DISO]VDC IRQ triggered, channel ==%d, flag ==%d\n", trigger_channel, trigger_flag);
#ifdef MTK_DISCRETE_SWITCH /*for DSC DC plugin handle */
		set_vdc_auxadc_irq(DISO_IRQ_DISABLE, 0);
		set_vusb_auxadc_irq(DISO_IRQ_DISABLE, 0);
		set_vusb_auxadc_irq(DISO_IRQ_ENABLE, DISO_IRQ_FALLING);
		if (trigger_flag == DISO_IRQ_RISING) {
			DISO_data.diso_state.pre_vusb_state  = DISO_ONLINE;
			DISO_data.diso_state.pre_vdc_state  = DISO_OFFLINE;
			DISO_data.diso_state.pre_otg_state  = DISO_OFFLINE;
			DISO_data.diso_state.cur_vusb_state  = DISO_ONLINE;
			DISO_data.diso_state.cur_vdc_state  = DISO_ONLINE;
			DISO_data.diso_state.cur_otg_state  = DISO_OFFLINE;
			pr_debug(" cur diso_state is %s!\n", DISO_state_s[2]);
		}
#else /*for load switch OTG leakage handle*/
		set_vdc_auxadc_irq(DISO_IRQ_ENABLE, (~trigger_flag) & 0x1);
		if (trigger_flag == DISO_IRQ_RISING) {
			DISO_data.diso_state.pre_vusb_state  = DISO_OFFLINE;
			DISO_data.diso_state.pre_vdc_state  = DISO_OFFLINE;
			DISO_data.diso_state.pre_otg_state  = DISO_ONLINE;
			DISO_data.diso_state.cur_vusb_state  = DISO_OFFLINE;
			DISO_data.diso_state.cur_vdc_state  = DISO_ONLINE;
			DISO_data.diso_state.cur_otg_state  = DISO_ONLINE;
			pr_debug(" cur diso_state is %s!\n", DISO_state_s[5]);
			} else {
			DISO_data.diso_state.pre_vusb_state  = DISO_OFFLINE;
			DISO_data.diso_state.pre_vdc_state  = DISO_ONLINE;
			DISO_data.diso_state.pre_otg_state  = DISO_ONLINE;
			DISO_data.diso_state.cur_vusb_state  = DISO_OFFLINE;
			DISO_data.diso_state.cur_vdc_state  = DISO_OFFLINE;
			DISO_data.diso_state.cur_otg_state  = DISO_ONLINE;
			pr_debug(" cur diso_state is %s!\n", DISO_state_s[1]);
		}
#endif
		break;
	case AP_AUXADC_DISO_VUSB_CHANNEL:
		trigger_flag = mt_auxadc_getCurrentTrigger();
		pr_debug("[DISO]VUSB IRQ triggered, channel ==%d, flag ==%d\n", trigger_channel, trigger_flag);
		set_vdc_auxadc_irq(DISO_IRQ_DISABLE, 0);
		set_vusb_auxadc_irq(DISO_IRQ_DISABLE, 0);
		if (trigger_flag == DISO_IRQ_FALLING) {
			DISO_data.diso_state.pre_vusb_state  = DISO_ONLINE;
			DISO_data.diso_state.pre_vdc_state  = DISO_ONLINE;
			DISO_data.diso_state.pre_otg_state  = DISO_OFFLINE;
			DISO_data.diso_state.cur_vusb_state  = DISO_OFFLINE;
			DISO_data.diso_state.cur_vdc_state  = DISO_ONLINE;
			DISO_data.diso_state.cur_otg_state  = DISO_OFFLINE;
			pr_debug(" cur diso_state is %s!\n", DISO_state_s[4]);
		} else {
			DISO_data.diso_state.pre_vusb_state  = DISO_OFFLINE;
			DISO_data.diso_state.pre_vdc_state  = DISO_ONLINE;
			DISO_data.diso_state.pre_otg_state  = DISO_OFFLINE;
			DISO_data.diso_state.cur_vusb_state  = DISO_ONLINE;
			DISO_data.diso_state.cur_vdc_state  = DISO_ONLINE;
			DISO_data.diso_state.cur_otg_state  = DISO_OFFLINE;
			pr_debug(" cur diso_state is %s!\n", DISO_state_s[6]);
		}
		set_vusb_auxadc_irq(DISO_IRQ_ENABLE, (~trigger_flag)&0x1);
		break;
	default:
		set_vdc_auxadc_irq(DISO_IRQ_DISABLE, 0);
		set_vusb_auxadc_irq(DISO_IRQ_DISABLE, 0);
		pr_debug("[DISO]VUSB auxadc IRQ triggered ERROR OR TEST\n");
		return IRQ_HANDLED; /* in error or unexecpt state just return */
	}
	g_diso_state = *(int *)&DISO_data.diso_state;
	return IRQ_WAKE_THREAD;
}
#endif

static unsigned int diso_get_current_voltage(int Channel)
{
	int ret = 0, data[4], i, ret_value = 0, ret_temp = 0, times = 5;

	if (IMM_IsAdcInitReady() == 0) {
		pr_debug("[DISO] AUXADC is not ready");
		return 0;
	}

	i = times;
	while (i--) {
		ret_value = IMM_GetOneChannelValue(Channel, data, &ret_temp);
		if (ret_value == 0) {
			ret += ret_temp;
		} else {
			times = times > 1 ? times - 1 : 1;
			pr_debug("[diso_get_current_voltage] ret_value=%d, times=%d\n",
			ret_value, times);
		}
	}

	ret = ret*1500/4096;
	ret = ret/times;

	return  ret;
}

static void _get_diso_interrupt_state(void)
{
	int vol = 0;
	int diso_state = 0;

	mdelay(AUXADC_CHANNEL_DELAY_PERIOD);

	vol = diso_get_current_voltage(AP_AUXADC_DISO_VDC_CHANNEL);
	vol = (R_DISO_DC_PULL_UP + R_DISO_DC_PULL_DOWN)*100*vol/(R_DISO_DC_PULL_DOWN)/100;
	pr_debug("[DISO]	Current DC voltage mV = %d\n", vol);

	/* force delay for switching as no flag for check switching done */
	mdelay(SWITCH_RISING_TIMING + LOAD_SWITCH_TIMING_MARGIN);
	if (vol > VDC_MIN_VOLTAGE/1000 && vol < VDC_MAX_VOLTAGE/1000)
		diso_state |= 0x4; /*SET DC bit as 1*/
	else
		diso_state &= ~0x4; /*SET DC bit as 0*/


	vol = diso_get_current_voltage(AP_AUXADC_DISO_VUSB_CHANNEL);
	vol = (R_DISO_VBUS_PULL_UP + R_DISO_VBUS_PULL_DOWN)*100*vol/(R_DISO_VBUS_PULL_DOWN)/100;
	pr_debug("[DISO]	Current VBUS voltage  mV = %d\n", vol);

	if (vol > VBUS_MIN_VOLTAGE/1000 && vol < VBUS_MAX_VOLTAGE/1000) {
		if (!mt_usb_is_device()) {
			diso_state |= 0x1; /*SET OTG bit as 1*/
			diso_state &= ~0x2; /*SET VBUS bit as 0*/
		} else {
			diso_state &= ~0x1; /*SET OTG bit as 0*/
			diso_state |= 0x2; /*SET VBUS bit as 1;*/
		}

	} else {
		diso_state &= 0x4; /*SET OTG and VBUS bit as 0*/
	}
	pr_debug("[DISO] DISO_STATE==0x%x\n", diso_state);
	g_diso_state = diso_state;
}
#if !defined(MTK_AUXADC_IRQ_SUPPORT)
int _get_irq_direction(int pre_vol, int cur_vol)
{
	int ret = -1;

	/*threshold 1000mv*/
	if ((cur_vol - pre_vol) > 1000)
		ret = DISO_IRQ_RISING;
	else if ((pre_vol - cur_vol) > 1000)
		ret = DISO_IRQ_FALLING;

	return ret;
}
extern unsigned int iddig_pin;   // tongjun@mid declare in kernel-3.18/drivers/misc/mediatek/usb20/mt8163/usb20_host.c
static void _get_polling_state(void)
{
	int vdc_vol = 0, vusb_vol = 0;
	int vdc_vol_dir = -1;
	int vusb_vol_dir = -1;

	DISO_polling_channel *VDC_Polling = &DISO_Polling.vdc_polling_measure;
	DISO_polling_channel *VUSB_Polling = &DISO_Polling.vusb_polling_measure;

	vdc_vol = diso_get_current_voltage(AP_AUXADC_DISO_VDC_CHANNEL);
	vdc_vol = (R_DISO_DC_PULL_UP + R_DISO_DC_PULL_DOWN)*100*vdc_vol/(R_DISO_DC_PULL_DOWN)/100;

	vusb_vol = diso_get_current_voltage(AP_AUXADC_DISO_VUSB_CHANNEL);
	vusb_vol = (R_DISO_VBUS_PULL_UP + R_DISO_VBUS_PULL_DOWN)*100*vusb_vol/(R_DISO_VBUS_PULL_DOWN)/100;

	VDC_Polling->preVoltage = VDC_Polling->curVoltage;
	VUSB_Polling->preVoltage = VUSB_Polling->curVoltage;
	VDC_Polling->curVoltage = vdc_vol;
	VUSB_Polling->curVoltage = vusb_vol;

	if (DISO_Polling.reset_polling) {
		DISO_Polling.reset_polling = KAL_FALSE;
		VDC_Polling->preVoltage = vdc_vol;
		VUSB_Polling->preVoltage = vusb_vol;

		if (vdc_vol > 1000)
			vdc_vol_dir = DISO_IRQ_RISING;
		else
			vdc_vol_dir = DISO_IRQ_FALLING;

		if (vusb_vol > 1000)
			vusb_vol_dir = DISO_IRQ_RISING;
		else
			vusb_vol_dir = DISO_IRQ_FALLING;
	} else {
		/*get voltage direction*/
		vdc_vol_dir = _get_irq_direction(VDC_Polling->preVoltage, VDC_Polling->curVoltage);
		vusb_vol_dir = _get_irq_direction(VUSB_Polling->preVoltage, VUSB_Polling->curVoltage);
	}

	if (VDC_Polling->notify_irq_en &&
	(vdc_vol_dir == VDC_Polling->notify_irq)) {
		schedule_delayed_work(&diso_polling_work, 10*HZ/1000); /*10ms*/
		pr_debug("[%s] ready to trig VDC irq, irq: %d\n",
		__func__, VDC_Polling->notify_irq);
	} else if (VUSB_Polling->notify_irq_en && (vusb_vol_dir == VUSB_Polling->notify_irq)) {
		// tongjun@mid modify start @{
		int iddig_state = 1;  //tong
		iddig_state = __gpio_get_value(iddig_pin); //
		printk("DISO: iddig_state=%d\n", iddig_state); //
		if(iddig_state !=0){ // only trigger in usb mode, iddig should be 1
		schedule_delayed_work(&diso_polling_work, 10*HZ/1000);
		pr_debug("[%s] ready to trig VUSB irq, irq: %d\n",
		__func__, VUSB_Polling->notify_irq);
		} 
		// tongjun@mid @}
	} else if ((vdc_vol == 0) && (vusb_vol == 0)) {
		VDC_Polling->notify_irq_en = 0;
		VUSB_Polling->notify_irq_en = 0;
	}

}

enum hrtimer_restart diso_kthread_hrtimer_func(struct hrtimer *timer)
{
	diso_thread_timeout = KAL_TRUE;
	wake_up(&diso_polling_thread_wq);

	return HRTIMER_NORESTART;
}

int diso_thread_kthread(void *x)
{
	/* Run on a process content */
	while (1) {
		wait_event(diso_polling_thread_wq, (diso_thread_timeout == KAL_TRUE));

		diso_thread_timeout = KAL_FALSE;

		mutex_lock(&diso_polling_mutex);

		_get_polling_state();

		if (DISO_Polling.vdc_polling_measure.notify_irq_en || DISO_Polling.vusb_polling_measure.notify_irq_en)
			hrtimer_start(&diso_kthread_timer, ktime_set(0, MSEC_TO_NSEC(SW_POLLING_PERIOD)),
			HRTIMER_MODE_REL);
		else
			hrtimer_cancel(&diso_kthread_timer);

		mutex_unlock(&diso_polling_mutex);
	}

	return 0;
}
#endif
#endif

/*
static unsigned int charging_get_error_state(void)
{
	return charging_error;
}
*/
static unsigned int charging_set_error_state(void *data)
{
	unsigned int status = STATUS_OK;

	charging_error = *(unsigned int *)(data);

	return status;
}

static unsigned int charging_diso_init(void *data)
{
	unsigned int status = STATUS_OK;

#if defined(CONFIG_MTK_DUAL_INPUT_CHARGER_SUPPORT)
	DISO_ChargerStruct *pDISO_data = (DISO_ChargerStruct *)data;

	/* Initialization DISO Struct */
	pDISO_data->diso_state.cur_otg_state	 = DISO_OFFLINE;
	pDISO_data->diso_state.cur_vusb_state = DISO_OFFLINE;
	pDISO_data->diso_state.cur_vdc_state	 = DISO_OFFLINE;

	pDISO_data->diso_state.pre_otg_state	 = DISO_OFFLINE;
	pDISO_data->diso_state.pre_vusb_state = DISO_OFFLINE;
	pDISO_data->diso_state.pre_vdc_state	 = DISO_OFFLINE;

	pDISO_data->chr_get_diso_state = KAL_FALSE;
	pDISO_data->hv_voltage = VBUS_MAX_VOLTAGE;

#if !defined(MTK_AUXADC_IRQ_SUPPORT)
	hrtimer_init(&diso_kthread_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	diso_kthread_timer.function = diso_kthread_hrtimer_func;
	INIT_DELAYED_WORK(&diso_polling_work, diso_polling_handler);

	kthread_run(diso_thread_kthread, NULL, "diso_thread_kthread");
	pr_debug("[%s] done\n", __func__);
#else
	struct device_node *node;
	int ret;

	/*Initial AuxADC IRQ*/
	DISO_IRQ.vdc_measure_channel.number   = AP_AUXADC_DISO_VDC_CHANNEL;
	DISO_IRQ.vusb_measure_channel.number  = AP_AUXADC_DISO_VUSB_CHANNEL;
	DISO_IRQ.vdc_measure_channel.period   = AUXADC_CHANNEL_DELAY_PERIOD;
	DISO_IRQ.vusb_measure_channel.period  = AUXADC_CHANNEL_DELAY_PERIOD;
	DISO_IRQ.vdc_measure_channel.debounce	  = AUXADC_CHANNEL_DEBOUNCE;
	DISO_IRQ.vusb_measure_channel.debounce  = AUXADC_CHANNEL_DEBOUNCE;

	/* use default threshold voltage, if use high voltage,maybe refine*/
	DISO_IRQ.vusb_measure_channel.falling_threshold = VBUS_MIN_VOLTAGE/1000;
	DISO_IRQ.vdc_measure_channel.falling_threshold = VDC_MIN_VOLTAGE/1000;
	DISO_IRQ.vusb_measure_channel.rising_threshold = VBUS_MIN_VOLTAGE/1000;
	DISO_IRQ.vdc_measure_channel.rising_threshold = VDC_MIN_VOLTAGE/1000;

	node = of_find_compatible_node(NULL, NULL, "mediatek,AUXADC");
	if (!node) {
		pr_debug("[diso_adc]: of_find_compatible_node failed!!\n");
	} else {
		pDISO_data->irq_line_number = irq_of_parse_and_map(node, 0);
		pr_debug("[diso_adc]: IRQ Number: 0x%x\n", pDISO_data->irq_line_number);
	}

	mt_irq_set_sens(pDISO_data->irq_line_number, MT_EDGE_SENSITIVE);
	mt_irq_set_polarity(pDISO_data->irq_line_number, MT_POLARITY_LOW);

	ret = request_threaded_irq(pDISO_data->irq_line_number, diso_auxadc_irq_handler,
	pDISO_data->irq_callback_func, IRQF_ONESHOT, "DISO_ADC_IRQ", NULL);

	if (ret) {
		pr_debug("[diso_adc]: request_irq failed.\n");
	} else {
		set_vdc_auxadc_irq(DISO_IRQ_DISABLE, 0);
		set_vusb_auxadc_irq(DISO_IRQ_DISABLE, 0);
		pr_debug("[diso_adc]: diso_init success.\n");
	}
#endif
#endif

	return status;
}

static unsigned int charging_get_diso_state(void *data)
{
	unsigned int status = STATUS_OK;

#if defined(CONFIG_MTK_DUAL_INPUT_CHARGER_SUPPORT)
	int diso_state = 0x0;
	DISO_ChargerStruct *pDISO_data = (DISO_ChargerStruct *)data;

	_get_diso_interrupt_state();
	diso_state = g_diso_state;
	pr_debug("[do_chrdet_int_task] current diso state is %s!\n", DISO_state_s[diso_state]);
	if (((diso_state >> 1) & 0x3) != 0x0) {
		switch (diso_state) {
		case USB_ONLY:
			set_vdc_auxadc_irq(DISO_IRQ_DISABLE, 0);
			set_vusb_auxadc_irq(DISO_IRQ_DISABLE, 0);
#ifdef MTK_DISCRETE_SWITCH
			set_vdc_auxadc_irq(DISO_IRQ_ENABLE, 1);
#endif
			pDISO_data->diso_state.cur_vusb_state = DISO_ONLINE;
			pDISO_data->diso_state.cur_vdc_state = DISO_OFFLINE;
			pDISO_data->diso_state.cur_otg_state = DISO_OFFLINE;
			break;
		case DC_ONLY:
			set_vdc_auxadc_irq(DISO_IRQ_DISABLE, 0);
			set_vusb_auxadc_irq(DISO_IRQ_DISABLE, 0);
			set_vusb_auxadc_irq(DISO_IRQ_ENABLE, DISO_IRQ_RISING);
			pDISO_data->diso_state.cur_vusb_state = DISO_OFFLINE;
			pDISO_data->diso_state.cur_vdc_state = DISO_ONLINE;
			pDISO_data->diso_state.cur_otg_state = DISO_OFFLINE;
			break;
		case DC_WITH_USB:
			set_vdc_auxadc_irq(DISO_IRQ_DISABLE, 0);
			set_vusb_auxadc_irq(DISO_IRQ_DISABLE, 0);
			set_vusb_auxadc_irq(DISO_IRQ_ENABLE, DISO_IRQ_FALLING);
			pDISO_data->diso_state.cur_vusb_state = DISO_ONLINE;
			pDISO_data->diso_state.cur_vdc_state = DISO_ONLINE;
			pDISO_data->diso_state.cur_otg_state = DISO_OFFLINE;
			break;
		case DC_WITH_OTG:
			set_vdc_auxadc_irq(DISO_IRQ_DISABLE, 0);
			set_vusb_auxadc_irq(DISO_IRQ_DISABLE, 0);
			pDISO_data->diso_state.cur_vusb_state = DISO_OFFLINE;
			pDISO_data->diso_state.cur_vdc_state = DISO_ONLINE;
			pDISO_data->diso_state.cur_otg_state = DISO_ONLINE;
			break;
		default: /*OTG only also can trigger vcdt IRQ*/
			pDISO_data->diso_state.cur_vusb_state = DISO_OFFLINE;
			pDISO_data->diso_state.cur_vdc_state = DISO_OFFLINE;
			pDISO_data->diso_state.cur_otg_state = DISO_ONLINE;
			pr_debug(" switch load vcdt irq triggerd by OTG Boost!\n");
			break; /*OTG plugin no need battery sync action*/
		}
	}

	if (DISO_ONLINE == pDISO_data->diso_state.cur_vdc_state)
		pDISO_data->hv_voltage = VDC_MAX_VOLTAGE;
	else
		pDISO_data->hv_voltage = VBUS_MAX_VOLTAGE;
#endif

	return status;
}

static unsigned int charging_set_vbus_ovp_en(void *data)
{
	return STATUS_OK;
}

static unsigned int charging_set_vindpm(void *data)
{
	return STATUS_OK;
}

static unsigned int charging_set_pwrstat_led_en(void *data)
{
	return STATUS_OK;
}

static unsigned int charging_run_aicl(void *data)
{
	return STATUS_OK;
}

static u32(*charging_func[CHARGING_CMD_NUMBER]) (void *data);
/*
 = {
charging_hw_init, charging_dump_register, charging_enable, charging_set_cv_voltage,
	    charging_get_current, charging_set_current, charging_set_input_current,
	    charging_get_charging_status, charging_reset_watch_dog_timer,
	    charging_set_hv_threshold, charging_get_hv_status, charging_get_battery_status,
	    charging_get_charger_det_status, charging_get_charger_type,
	    charging_get_is_pcm_timer_trigger, charging_set_platform_reset,
	    charging_get_platform_boot_mode, charging_set_power_off,
	    charging_get_power_source, charging_get_csdac_full_flag,
	    charging_set_ta_current_pattern, charging_set_error_state};
*/
s32 chr_control_interface(CHARGING_CTRL_CMD cmd, void *data)
{
	s32 status;
	
	static signed int init = -1;

	if (init == -1) {
		init = 0;
		charging_func[CHARGING_CMD_INIT] = charging_hw_init;
		charging_func[CHARGING_CMD_DUMP_REGISTER] = charging_dump_register;
		charging_func[CHARGING_CMD_ENABLE] = charging_enable;
		charging_func[CHARGING_CMD_SET_CV_VOLTAGE] = charging_set_cv_voltage;
		charging_func[CHARGING_CMD_GET_CURRENT] = charging_get_current;
		charging_func[CHARGING_CMD_SET_CURRENT] = charging_set_current;
		charging_func[CHARGING_CMD_SET_INPUT_CURRENT] = charging_set_input_current;
		charging_func[CHARGING_CMD_GET_CHARGING_STATUS] = charging_get_charging_status;
		charging_func[CHARGING_CMD_RESET_WATCH_DOG_TIMER] = charging_reset_watch_dog_timer;
		charging_func[CHARGING_CMD_SET_HV_THRESHOLD] = charging_set_hv_threshold;
		charging_func[CHARGING_CMD_GET_HV_STATUS] = charging_get_hv_status;
		charging_func[CHARGING_CMD_GET_BATTERY_STATUS] = charging_get_battery_status;
		charging_func[CHARGING_CMD_GET_CHARGER_DET_STATUS] = charging_get_charger_det_status;
		charging_func[CHARGING_CMD_GET_CHARGER_TYPE] = charging_get_charger_type;
		charging_func[CHARGING_CMD_GET_IS_PCM_TIMER_TRIGGER] = charging_get_is_pcm_timer_trigger;
		charging_func[CHARGING_CMD_SET_PLATFORM_RESET] = charging_set_platform_reset;
		charging_func[CHARGING_CMD_GET_PLATFORM_BOOT_MODE] = charging_get_platform_boot_mode;
		charging_func[CHARGING_CMD_SET_POWER_OFF] = charging_set_power_off;
		charging_func[CHARGING_CMD_GET_POWER_SOURCE] = charging_get_power_source;
		charging_func[CHARGING_CMD_GET_CSDAC_FALL_FLAG] = charging_get_csdac_full_flag;
		charging_func[CHARGING_CMD_SET_TA_CURRENT_PATTERN] = charging_set_ta_current_pattern;
		charging_func[CHARGING_CMD_SET_ERROR_STATE] = charging_set_error_state;
		charging_func[CHARGING_CMD_DISO_INIT] = charging_diso_init;
		charging_func[CHARGING_CMD_GET_DISO_STATE] = charging_get_diso_state;
		charging_func[CHARGING_CMD_SET_VBUS_OVP_EN] = charging_set_vbus_ovp_en;
		charging_func[CHARGING_CMD_SET_VINDPM] = charging_set_vindpm;
		charging_func[CHARGING_CMD_SET_PWRSTAT_LED_EN] = charging_set_pwrstat_led_en;
		charging_func[CHARGING_CMD_RUN_AICL] = charging_run_aicl;
	}

	if (cmd < CHARGING_CMD_NUMBER)
		status = charging_func[cmd] (data);
	else
		return STATUS_UNSUPPORTED;

	return status;
}

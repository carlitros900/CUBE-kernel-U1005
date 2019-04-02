#include "mt65xx_lcm_list.h"
#include <lcm_drv.h>
#ifdef BUILD_LK
#include <platform/disp_drv_platform.h>
#else
#include <linux/delay.h>
/* #include <mach/mt_gpio.h> */
#endif
LCM_DSI_MODE_CON lcm_dsi_mode;

/* used to identify float ID PIN status */
#define LCD_HW_ID_STATUS_LOW      0
#define LCD_HW_ID_STATUS_HIGH     1
#define LCD_HW_ID_STATUS_FLOAT 0x02
#define LCD_HW_ID_STATUS_ERROR  0x03

#ifdef BUILD_LK
#define LCD_DEBUG(fmt)  dprintf(CRITICAL, fmt)
#else
#define LCD_DEBUG(fmt, args...)  pr_debug("[KERNEL/LCM]"fmt, ##args)
#endif

extern LCM_DRIVER c805_hz_jd9365_hsd_wxga_ips_8_lcm_drv;
extern LCM_DRIVER c805_jd9367_wxga_lcm_drv;
extern LCM_DRIVER c710_mc_nt35521s_cpt_wxga_ips_101_lcm_drv;
extern LCM_DRIVER c805_xy_jd9366ab_boe_wxga_ips_8_lcm_drv;
extern LCM_DRIVER c708_pbtb080h030_lcm_drv;
extern LCM_DRIVER c805_otm1287a_wxga_lcm_drv;
extern LCM_DRIVER c708_yf_jd9367_inx_wxga_ips_7_lcm_drv;
extern LCM_DRIVER c708_sx_jd9366_auo_wxga_ips_8_lcm_drv;
extern LCM_DRIVER c708_gx_ek79007_wsvga_cpt_tn_lcm_drv;
extern LCM_DRIVER c710_xc_nt51012_boe_wuxga_ips_101_lcm_drv;
extern LCM_DRIVER c708_zdcd_jd9366ab_boe_wxga_ips_8_lcm_drv;

extern LCM_DRIVER c805_hx_s6d7aa0x04_cpt_wxga_ips_8_lcm_drv;

extern LCM_DRIVER c805_bd_jd9366_wxga_ips_lcm_drv;

extern LCM_DRIVER c805_fx_S6D7AA0X01_boe_wxga_ips_8_lcm_drv;

extern LCM_DRIVER c805_fx_ota7290b_boe_wuxga_ips_101_lcm_drv;

extern LCM_DRIVER c71_cz_ek79007_boe_wsvganl_tn_7_lcm_drv;

extern LCM_DRIVER c708_mc_jd9366_wxga_cpt_8_lcm_drv;

extern LCM_DRIVER c805_zs_hx8394a01_inx_wxga_ips_8_lcm_drv;

extern LCM_DRIVER c710_pb_jd9365_boe_wxga_ips_101_lcm_drv;

extern LCM_DRIVER c102_qc_jd9364_inx_wxga_ips_101_lcm_drv;
extern LCM_DRIVER c805_zs_ek79029aa_boe_wxga_ips_8_lcm_drv;

extern LCM_DRIVER c805_hx_RM72010_auo_wxga_ips_8_lcm_drv;

extern LCM_DRIVER c86_xy_ek79007_cpt_wvsxganl_ips_7_rgb_lcm_drv;

extern LCM_DRIVER c805_jd9366_wxga_lcm_drv;

extern LCM_DRIVER c805_hnh_hx8394_boe_wuxga_ips_101_lcm_drv;

extern LCM_DRIVER c708_cz_ek79007_boe_wsxga_tn_7_lcm_drv;
extern LCM_DRIVER c805_jlt_jd9364_inx_wxga_ips_101_lcm_drv;
extern LCM_DRIVER c805_pxjt_nt35521s_inx_wxga_ips_101_lcm_drv;
extern LCM_DRIVER c118_it6151_edp_dsi_lcm_drv;
extern LCM_DRIVER c716_ek79007_wsvga_dsi_lcm_drv;
extern LCM_DRIVER c716_zs18032901_dsi_wxga_lcm_drv;
extern LCM_DRIVER c716_fx_ota7290b_boe_wuxga_ips_101_lcm_drv;
extern LCM_DRIVER c716_fx_jd9365_boe_wxga_ips_101_lcm_drv;
extern LCM_DRIVER c716_pb_jd9365_hsd_wxga_ips_101_lcm_drv;
extern LCM_DRIVER c716_fy_hx8394d_boe_wxga_ips_101_lcm_drv;
extern LCM_DRIVER c716_fx_jd9365_cpt_wxga_ips_101_lcm_drv;
extern LCM_DRIVER amsa05bv_dsi_vdo_mt8173_lcm_drv;

LCM_DRIVER *lcm_driver_list[] = {
/**######################## MID_LCM start ###########################**/
	&amsa05bv_dsi_vdo_mt8173_lcm_drv,
/**######################## MID_LCM end #############################**/
};
#if defined(MTK_LCM_DEVICE_TREE_SUPPORT)
unsigned char lcm_name_list[][128] = {
	"rc_rgb_wsvga",

};
#endif


#define LCM_COMPILE_ASSERT(condition) LCM_COMPILE_ASSERT_X(condition, __LINE__)
#define LCM_COMPILE_ASSERT_X(condition, line) LCM_COMPILE_ASSERT_XX(condition, line)
#define LCM_COMPILE_ASSERT_XX(condition, line) char assertion_failed_at_line_##line[(condition) ? 1 : -1]

unsigned int lcm_count = sizeof(lcm_driver_list) / sizeof(LCM_DRIVER *);
LCM_COMPILE_ASSERT(0 != sizeof(lcm_driver_list) / sizeof(LCM_DRIVER *));
#if defined(NT35520_HD720_DSI_CMD_TM) | defined(NT35520_HD720_DSI_CMD_BOE) | \
	defined(NT35521_HD720_DSI_VDO_BOE) | defined(NT35521_HD720_DSI_VIDEO_TM)
static unsigned char lcd_id_pins_value = 0xFF;

/**
 * Function:       which_lcd_module_triple
 * Description:    read LCD ID PIN status,could identify three status:highlowfloat
 * Input:           none
 * Output:         none
 * Return:         LCD ID1|ID0 value
 * Others:
 */
unsigned char which_lcd_module_triple(void)
{
	unsigned char  high_read0 = 0;
	unsigned char  low_read0 = 0;
	unsigned char  high_read1 = 0;
	unsigned char  low_read1 = 0;
	unsigned char  lcd_id0 = 0;
	unsigned char  lcd_id1 = 0;
	unsigned char  lcd_id = 0;
	/*Solve Coverity scan warning : check return value*/
	unsigned int ret = 0;

	/*only recognise once*/
	if (0xFF != lcd_id_pins_value)
		return lcd_id_pins_value;

	/*Solve Coverity scan warning : check return value*/
	ret = mt_set_gpio_mode(GPIO_DISP_ID0_PIN, GPIO_MODE_00);
	if (0 != ret)
		LCD_DEBUG("ID0 mt_set_gpio_mode fail\n");

	ret = mt_set_gpio_dir(GPIO_DISP_ID0_PIN, GPIO_DIR_IN);
	if (0 != ret)
		LCD_DEBUG("ID0 mt_set_gpio_dir fail\n");

	ret = mt_set_gpio_pull_enable(GPIO_DISP_ID0_PIN, GPIO_PULL_ENABLE);
	if (0 != ret)
		LCD_DEBUG("ID0 mt_set_gpio_pull_enable fail\n");

	ret = mt_set_gpio_mode(GPIO_DISP_ID1_PIN, GPIO_MODE_00);
	if (0 != ret)
		LCD_DEBUG("ID1 mt_set_gpio_mode fail\n");

	ret = mt_set_gpio_dir(GPIO_DISP_ID1_PIN, GPIO_DIR_IN);
	if (0 != ret)
		LCD_DEBUG("ID1 mt_set_gpio_dir fail\n");

	ret = mt_set_gpio_pull_enable(GPIO_DISP_ID1_PIN, GPIO_PULL_ENABLE);
	if (0 != ret)
		LCD_DEBUG("ID1 mt_set_gpio_pull_enable fail\n");

	/*pull down ID0 ID1 PIN*/
	ret = mt_set_gpio_pull_select(GPIO_DISP_ID0_PIN, GPIO_PULL_DOWN);
	if (0 != ret)
		LCD_DEBUG("ID0 mt_set_gpio_pull_select->Down fail\n");

	ret = mt_set_gpio_pull_select(GPIO_DISP_ID1_PIN, GPIO_PULL_DOWN);
	if (0 != ret)
		LCD_DEBUG("ID1 mt_set_gpio_pull_select->Down fail\n");

	/* delay 100ms , for discharging capacitance*/
	mdelay(100);
	/* get ID0 ID1 status*/
	low_read0 = mt_get_gpio_in(GPIO_DISP_ID0_PIN);
	low_read1 = mt_get_gpio_in(GPIO_DISP_ID1_PIN);
	/* pull up ID0 ID1 PIN */
	ret = mt_set_gpio_pull_select(GPIO_DISP_ID0_PIN, GPIO_PULL_UP);
	if (0 != ret)
		LCD_DEBUG("ID0 mt_set_gpio_pull_select->UP fail\n");

	ret = mt_set_gpio_pull_select(GPIO_DISP_ID1_PIN, GPIO_PULL_UP);
	if (0 != ret)
		LCD_DEBUG("ID1 mt_set_gpio_pull_select->UP fail\n");

	/* delay 100ms , for charging capacitance */
	mdelay(100);
	/* get ID0 ID1 status */
	high_read0 = mt_get_gpio_in(GPIO_DISP_ID0_PIN);
	high_read1 = mt_get_gpio_in(GPIO_DISP_ID1_PIN);

	if (low_read0 != high_read0) {
		/*float status , pull down ID0 ,to prevent electric leakage*/
		ret = mt_set_gpio_pull_select(GPIO_DISP_ID0_PIN, GPIO_PULL_DOWN);
		if (0 != ret)
			LCD_DEBUG("ID0 mt_set_gpio_pull_select->Down fail\n");

		lcd_id0 = LCD_HW_ID_STATUS_FLOAT;
	} else if ((LCD_HW_ID_STATUS_LOW == low_read0) && (LCD_HW_ID_STATUS_LOW == high_read0)) {
		/*low status , pull down ID0 ,to prevent electric leakage*/
		ret = mt_set_gpio_pull_select(GPIO_DISP_ID0_PIN, GPIO_PULL_DOWN);
		if (0 != ret)
			LCD_DEBUG("ID0 mt_set_gpio_pull_select->Down fail\n");

		lcd_id0 = LCD_HW_ID_STATUS_LOW;
	} else if ((LCD_HW_ID_STATUS_HIGH == low_read0) && (LCD_HW_ID_STATUS_HIGH == high_read0)) {
		/*high status , pull up ID0 ,to prevent electric leakage*/
		ret = mt_set_gpio_pull_select(GPIO_DISP_ID0_PIN, GPIO_PULL_UP);
		if (0 != ret)
			LCD_DEBUG("ID0 mt_set_gpio_pull_select->UP fail\n");

		lcd_id0 = LCD_HW_ID_STATUS_HIGH;
	} else {
		LCD_DEBUG(" Read LCD_id0 error\n");
		ret = mt_set_gpio_pull_select(GPIO_DISP_ID0_PIN, GPIO_PULL_DISABLE);
		if (0 != ret)
			LCD_DEBUG("ID0 mt_set_gpio_pull_select->Disbale fail\n");

		lcd_id0 = LCD_HW_ID_STATUS_ERROR;
	}


	if (low_read1 != high_read1) {
		/*float status , pull down ID1 ,to prevent electric leakage*/
		ret = mt_set_gpio_pull_select(GPIO_DISP_ID1_PIN, GPIO_PULL_DOWN);
		if (0 != ret)
			LCD_DEBUG("ID1 mt_set_gpio_pull_select->Down fail\n");

		lcd_id1 = LCD_HW_ID_STATUS_FLOAT;
	} else if ((LCD_HW_ID_STATUS_LOW == low_read1) && (LCD_HW_ID_STATUS_LOW == high_read1)) {
		/*low status , pull down ID1 ,to prevent electric leakage*/
		ret = mt_set_gpio_pull_select(GPIO_DISP_ID1_PIN, GPIO_PULL_DOWN);
		if (0 != ret)
			LCD_DEBUG("ID1 mt_set_gpio_pull_select->Down fail\n");

		lcd_id1 = LCD_HW_ID_STATUS_LOW;
	} else if ((LCD_HW_ID_STATUS_HIGH == low_read1) && (LCD_HW_ID_STATUS_HIGH == high_read1)) {
		/*high status , pull up ID1 ,to prevent electric leakage*/
		ret = mt_set_gpio_pull_select(GPIO_DISP_ID1_PIN, GPIO_PULL_UP);
		if (0 != ret)
			LCD_DEBUG("ID1 mt_set_gpio_pull_select->UP fail\n");

		lcd_id1 = LCD_HW_ID_STATUS_HIGH;
	} else {

		LCD_DEBUG(" Read LCD_id1 error\n");
		ret = mt_set_gpio_pull_select(GPIO_DISP_ID1_PIN, GPIO_PULL_DISABLE);
		if (0 != ret)
			LCD_DEBUG("ID1 mt_set_gpio_pull_select->Disable fail\n");

		lcd_id1 = LCD_HW_ID_STATUS_ERROR;
	}
#ifdef BUILD_LK
	dprintf(CRITICAL, "which_lcd_module_triple,lcd_id0:%d\n", lcd_id0);
	dprintf(CRITICAL, "which_lcd_module_triple,lcd_id1:%d\n", lcd_id1);
#else
	LCD_DEBUG("which_lcd_module_triple,lcd_id0:%d\n", lcd_id0);
	LCD_DEBUG("which_lcd_module_triple,lcd_id1:%d\n", lcd_id1);
#endif
	lcd_id =  lcd_id0 | (lcd_id1 << 2);

#ifdef BUILD_LK
	dprintf(CRITICAL, "which_lcd_module_triple,lcd_id:%d\n", lcd_id);
#else
	LCD_DEBUG("which_lcd_module_triple,lcd_id:%d\n", lcd_id);
#endif

	lcd_id_pins_value = lcd_id;
	return lcd_id;
}
#endif

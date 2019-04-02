/*
* Copyright (C) 2016 MediaTek Inc.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See http://www.gnu.org/licenses/gpl-2.0.html for more details.
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/suspend.h>
#include <linux/console.h>
#include <mt-plat/aee.h>
#include <mt-plat/sync_write.h>
#include "mt_sleep.h"
#include "mt_spm.h"
#include "mt_spm_sleep.h"
#include <mach/mt_spm_mtcmos.h>
/*
#include <mach/mt_clkmgr.h>
#include <mach/mt_gpio.h>
*/
/**************************************
 * only for internal debug
 **************************************/
#ifdef CONFIG_MTK_LDVT
#define SLP_SLEEP_DPIDLE_EN         1
#define SLP_REPLACE_DEF_WAKESRC     1
#define SLP_SUSPEND_LOG_EN          1
#else
#define SLP_SLEEP_DPIDLE_EN         1
#define SLP_REPLACE_DEF_WAKESRC     0
#define SLP_SUSPEND_LOG_EN          0
#endif

/**************************************
 * SW code for suspend
 **************************************/
#define slp_read(addr)			__raw_readl((void __force __iomem *)(addr))
#define slp_write(addr, val)		mt_reg_sync_writel(val, addr)

#define spm_is_wakesrc_invalid(wakesrc)     (!!((u32)(wakesrc) & 0xc082b881))

#define slp_emerg(fmt, args...)     pr_emerg("[SLP] " fmt, ##args)
#define slp_alert(fmt, args...)     pr_alert("[SLP] " fmt, ##args)
#define slp_crit(fmt, args...)      pr_crit("[SLP] " fmt, ##args)
#define slp_crit2(fmt, args...)     pr_crit("[SLP] " fmt, ##args)
#define slp_error(fmt, args...)     pr_err("[SLP] " fmt, ##args)
#define slp_warning(fmt, args...)   pr_warn("[SLP] " fmt, ##args)
#define slp_notice(fmt, args...)    pr_notice("[SLP] " fmt, ##args)
#define slp_info(fmt, args...)      pr_info("[SLP] " fmt, ##args)
#define slp_debug(fmt, args...)     pr_info("[SLP] " fmt, ##args)

static DEFINE_SPINLOCK(slp_lock);

static wake_reason_t slp_wake_reason = WR_NONE;

static bool slp_ck26m_on;

/*
 * SLEEP_DPIDLE_EN:1 && slp_ck26m_on=1
 *    1 = CPU dormant
 *    0 = CPU standby
 * SLEEP_DPIDLE_EN:0 || slp_ck26m_on=0
 *    1 = CPU shutdown
 *    0 = CPU standby
 */
static bool slp_cpu_pdn = true;

/*
 * SLEEP_DPIDLE_EN:0 || slp_ck26m_on=0
 *    1 = INFRA/DDRPHY power down
 *    0 = keep INFRA/DDRPHY power
 */
#ifndef MTK_ALPS_BOX_SUPPORT
static bool slp_infra_pdn = true;
#else
static bool slp_infra_pdn;
#endif
/*
 * SLEEP_DPIDLE_EN:1 && slp_ck26m_on=1
 *    0 = AXI is off
 *    1 = AXI is 26M
 */
static u16 slp_pwrlevel;

static int slp_pwake_time = -1;	/* sec */

static bool slp_chk_golden;
static bool slp_dump_gpio;
static bool slp_dump_regs;

#if 0
static void slp_dump_pm_regs(void)
{
	/* PLL/TOPCKGEN register */
	slp_debug("AP_PLL_CON0     0x%x = 0x%x\n", AP_PLL_CON0, slp_read(AP_PLL_CON0));
	slp_debug("AP_PLL_CON1     0x%x = 0x%x\n", AP_PLL_CON1, slp_read(AP_PLL_CON1));
	slp_debug("AP_PLL_CON2     0x%x = 0x%x\n", AP_PLL_CON2, slp_read(AP_PLL_CON2));
	slp_debug("UNIVPLL_CON0    0x%x = 0x%x\n", UNIVPLL_CON0, slp_read(UNIVPLL_CON0));
	slp_debug("UNIVPLL_PWR_CON 0x%x = 0x%x\n", UNIVPLL_PWR_CON0, slp_read(UNIVPLL_PWR_CON0));
	slp_debug("MMPLL_CON0      0x%x = 0x%x\n", MMPLL_CON0, slp_read(MMPLL_CON0));
	slp_debug("MMPLL_PWR_CON   0x%x = 0x%x\n", MMPLL_PWR_CON0, slp_read(MMPLL_PWR_CON0));
	slp_debug("CLK_SCP_CFG_0   0x%x = 0x%x\n", CLK_SCP_CFG_0, slp_read(CLK_SCP_CFG_0));
	slp_debug("CLK_SCP_CFG_1   0x%x = 0x%x\n", CLK_SCP_CFG_1, slp_read(CLK_SCP_CFG_1));

	/* INFRA/PERICFG register */
	slp_debug("INFRA_PDN_STA   0x%x = 0x%x\n", INFRA_PDN_STA, slp_read(INFRA_PDN_STA));
	slp_debug("PERI_PDN0_STA   0x%x = 0x%x\n", PERI_PDN0_STA, slp_read(PERI_PDN0_STA));

	/* SPM register */
	slp_debug("POWER_ON_VAL0   0x%x = 0x%x\n", SPM_POWER_ON_VAL0, slp_read(SPM_POWER_ON_VAL0));
	slp_debug("POWER_ON_VAL1   0x%x = 0x%x\n", SPM_POWER_ON_VAL1, slp_read(SPM_POWER_ON_VAL1));
	slp_debug("SPM_PCM_CON1    0x%x = 0x%x\n", SPM_PCM_CON1, slp_read(SPM_PCM_CON1));
	slp_debug("PCM_PWR_IO_EN   0x%x = 0x%x\n", SPM_PCM_PWR_IO_EN, slp_read(SPM_PCM_PWR_IO_EN));
	slp_debug("PCM_REG0_DATA   0x%x = 0x%x\n", SPM_PCM_REG0_DATA, slp_read(SPM_PCM_REG0_DATA));
	slp_debug("PCM_REG7_DATA   0x%x = 0x%x\n", SPM_PCM_REG7_DATA, slp_read(SPM_PCM_REG7_DATA));
	slp_debug("PCM_REG13_DATA  0x%x = 0x%x\n", SPM_PCM_REG13_DATA,
		  slp_read(SPM_PCM_REG13_DATA));
	slp_debug("CLK_CON         0x%x = 0x%x\n", SPM_CLK_CON, slp_read(SPM_CLK_CON));
	slp_debug("AP_DVFS_CON     0x%x = 0x%x\n", SPM_AP_DVFS_CON_SET,
		  slp_read(SPM_AP_DVFS_CON_SET));
	slp_debug("PWR_STATUS      0x%x = 0x%x\n", SPM_PWR_STATUS, slp_read(SPM_PWR_STATUS));
	slp_debug("SPM_PCM_SRC_REQ 0x%x = 0x%x\n", SPM_PCM_SRC_REQ, slp_read(SPM_PCM_SRC_REQ));
}
#endif

static int slp_suspend_ops_valid(suspend_state_t state)
{
	return state == PM_SUSPEND_MEM;
}

static int slp_suspend_ops_begin(suspend_state_t state)
{
	/* legacy log */
	slp_notice("@@@@@@@@@@@@ Chip_pm_begin(%u)(%u) @@@@@@@@@@@@\n", slp_cpu_pdn, slp_infra_pdn);

	slp_wake_reason = WR_NONE;

	return 0;
}

static int slp_suspend_ops_prepare(void)
{
	/* legacy log */
	slp_crit2("@@@@@@@@@@@@ Chip_pm_prepare @@@@@@@@@@@@\n");

#if 0
	if (slp_chk_golden)
		mt_power_gs_dump_suspend();
#endif
	return 0;
}

static int slp_suspend_ops_enter(suspend_state_t state)
{
	/* legacy log */
	slp_crit2("@@@@@@@@@@@@ Chip_pm_enter @@@@@@@@@@@@\n");

#if 0
	if (slp_dump_gpio)
		gpio_dump_regs();

	if (slp_dump_regs)
		slp_dump_pm_regs();
#endif
	if (!spm_cpusys0_can_power_down()) {
		slp_error("CANNOT SLEEP DUE TO CPU1/2/3 PON\n");
		return -EPERM;
	}
	if (slp_infra_pdn && !slp_cpu_pdn) {
		slp_error("CANNOT SLEEP DUE TO INFRA PDN BUT CPU PON\n");
		return -EPERM;
	}
#if SLP_SLEEP_DPIDLE_EN
	if (slp_ck26m_on)
		slp_wake_reason = spm_go_to_sleep_dpidle(slp_cpu_pdn, slp_pwrlevel, slp_pwake_time);
	else
#endif
		slp_wake_reason = spm_go_to_sleep(slp_cpu_pdn, slp_infra_pdn, slp_pwake_time);

	return 0;
}

static void slp_suspend_ops_finish(void)
{
	/* legacy log */
	slp_notice("@@@@@@@@@@@@@@@@@@@@\n");
	slp_crit2("Chip_pm_finish\n");
	slp_notice("@@@@@@@@@@@@@@@@@@@@\n");
}

static void slp_suspend_ops_end(void)
{
	/* legacy log */
	slp_notice("@@@@@@@@@@@@@@@@@@@@\n");
	slp_notice("Chip_pm_end\n");
	slp_notice("@@@@@@@@@@@@@@@@@@@@\n");
}

static const struct platform_suspend_ops slp_suspend_ops = {
	.valid = slp_suspend_ops_valid,
	.begin = slp_suspend_ops_begin,
	.prepare = slp_suspend_ops_prepare,
	.enter = slp_suspend_ops_enter,
	.finish = slp_suspend_ops_finish,
	.end = slp_suspend_ops_end,
};

/*
 * wakesrc : WAKE_SRC_XXX
 * enable  : enable or disable @wakesrc
 * ck26m_on: if true, mean @wakesrc needs 26M to work
 */
int slp_set_wakesrc(u32 wakesrc, bool enable, bool ck26m_on)
{
	int r;
	unsigned long flags;

	slp_notice("wakesrc = 0x%x, enable = %u, ck26m_on = %u\n", wakesrc, enable, ck26m_on);

#if SLP_REPLACE_DEF_WAKESRC
	if (wakesrc & WAKE_SRC_CFG_KEY)
#else
	if (!(wakesrc & WAKE_SRC_CFG_KEY))
#endif
		return -EPERM;

	spin_lock_irqsave(&slp_lock, flags);
#if SLP_REPLACE_DEF_WAKESRC
	r = spm_set_sleep_wakesrc(wakesrc, enable, true);
#else
	r = spm_set_sleep_wakesrc(wakesrc & ~WAKE_SRC_CFG_KEY, enable, false);
#endif

	if (!r)
		slp_ck26m_on = ck26m_on;
	spin_unlock_irqrestore(&slp_lock, flags);

	return r;
}

wake_reason_t slp_get_wake_reason(void)
{
	return slp_wake_reason;
}

bool slp_will_infra_pdn(void)
{
	return slp_infra_pdn;
}

static int __init slp_module_init(void)
{
	spm_output_sleep_option();

	slp_notice("SLEEP_DPIDLE_EN:%d, REPLACE_DEF_WAKESRC:%d, SUSPEND_LOG_EN:%d\n",
		   SLP_SLEEP_DPIDLE_EN, SLP_REPLACE_DEF_WAKESRC, SLP_SUSPEND_LOG_EN);

	suspend_set_ops(&slp_suspend_ops);

#if SLP_SUSPEND_LOG_EN
	console_suspend_enabled = 0;
#endif
	return 0;
}
arch_initcall(slp_module_init);

module_param(slp_ck26m_on, bool, 0644);

module_param(slp_cpu_pdn, bool, 0644);
module_param(slp_infra_pdn, bool, 0644);
module_param(slp_pwrlevel, ushort, 0644);

module_param(slp_pwake_time, int, 0644);

module_param(slp_chk_golden, bool, 0644);
module_param(slp_dump_gpio, bool, 0644);
module_param(slp_dump_regs, bool, 0644);

MODULE_AUTHOR("YT Lee <yt.lee@mediatek.com>");
MODULE_DESCRIPTION("Sleep Driver v0.4");

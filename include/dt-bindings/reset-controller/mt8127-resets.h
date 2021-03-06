/*
 * Copyright (c) 2015 MediaTek, Shunli Wang <shunli.wang@mediatek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _DT_BINDINGS_RESET_CONTROLLER_MT8127
#define _DT_BINDINGS_RESET_CONTROLLER_MT8127

/* INFRACFG resets */
#define MT8127_INFRA_EMI_REG_RST		0
#define MT8127_INFRA_DRAMC0_A0_RST		1
#define MT8127_INFRA_FHCTL_RST			2
#define MT8127_INFRA_APCIRQ_EINT_RST		3
#define MT8127_INFRA_APXGPT_RST			4
#define MT8127_INFRA_SCPSYS_RST			5
#define MT8127_INFRA_KP_RST			6
#define MT8127_INFRA_PMIC_WRAP_RST		7
#define MT8127_INFRA_MIPI_RST			8
#define MT8127_INFRA_IRRX_RST			9
#define MT8127_INFRA_CEC_RST			10
#define MT8127_INFRA_EMI_RST			32
#define MT8127_INFRA_DRAMC0_RST			34
#define MT8127_INFRA_TRNG_RST			37
#define MT8127_INFRA_SYSIRQ_RST			38


/*  PERICFG resets */
#define MT8127_PERI_UART0_SW_RST		0
#define MT8127_PERI_UART1_SW_RST		1
#define MT8127_PERI_UART2_SW_RST		2
#define MT8127_PERI_UART3_SW_RST		3
#define MT8127_PERI_GCPU_SW_RST			5
#define MT8127_PERI_BTIF_SW_RST			6
#define MT8127_PERI_PWM_SW_RST			8
#define MT8127_PERI_AUXADC_SW_RST		10
#define MT8127_PERI_DMA_SW_RST			11
#define MT8127_PERI_NFI_SW_RST			14
#define MT8127_PERI_NLI_SW_RST			15
#define MT8127_PERI_THERM_SW_RST		16
#define MT8127_PERI_MSDC2_SW_RST		17
#define MT8127_PERI_MSDC0_SW_RST		19
#define MT8127_PERI_MSDC1_SW_RST		20
#define MT8127_PERI_I2C0_SW_RST			22
#define MT8127_PERI_I2C1_SW_RST			23
#define MT8127_PERI_I2C2_SW_RST			24
#define MT8127_PERI_I2C3_SW_RST			25
#define MT8127_PERI_USB_SW_RST			28
#define MT8127_PERI_ETH_SW_RST			29
#define MT8127_PERI_SPI0_SW_RST			33

/* TOPRGU resets */
#define MT8127_TOPRGU_INFRA_RST         0
#define MT8127_TOPRGU_MM_RST            1
#define MT8127_TOPRGU_MFG_RST           2
#define MT8127_TOPRGU_VENC_RST          3
#define MT8127_TOPRGU_VDEC_RST          4
#define MT8127_TOPRGU_IMG_RST           5
#define MT8127_TOPRGU_DDRPHY_RST        6
#define MT8127_TOPRGU_MD_RST            7
#define MT8127_TOPRGU_INFRA_AO_RST      8
#define MT8127_TOPRGU_CONN_RST          9
#define MT8127_TOPRGU_APMIXED_RST       10
#define MT8127_TOPRGU_PWRAP_SPICTL_RST  11
#define MT8127_TOPRGU_CONN_MCU_RST      12

#endif  /* _DT_BINDINGS_RESET_CONTROLLER_MT8127 */

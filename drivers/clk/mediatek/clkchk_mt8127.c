/*
 * Copyright (C) 2015 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include "clkchk.h"

static const char * const off_pll_names[] = {
	"msdcpll",
	"audpll",
	"tvdpll",
	"lvdspll",
	NULL
};

static const char * const all_clk_names[] = {
	/* ROOT */
	"clkph_mck_o",
	"mempll2",
	"fpc_ck",
	"hdmitx_clkdig",
	"lvdstx_clkdig",
	"lvdspll_eth",
	"dpi_ck",
	"vencpll_ck",
	/* APMIXEDSYS */
	"armpll",
	"mainpll",
	"univpll",
	"mmpll",
	"msdcpll",
	"audpll",
	"tvdpll",
	"lvdspll",
	/* TOP_DIVS */
	"syspll_d2",
	"syspll_d3",
	"syspll_d5",
	"6syspll_d7",
	"syspll1_d2",
	"syspll1_d4",
	"syspll1_d8",
	"syspll1_d16",
	"syspll2_d2",
	"syspll2_d4",
	"syspll2_d8",
	"syspll3_d2",
	"syspll3_d4",
	"syspll4_d2",
	"syspll4_d4",
	"univpll_d2",
	"univpll_d3",
	"univpll_d5",
	"univpll_d26",
	"univpll1_d2",
	"univpll1_d4",
	"univpll1_d8",
	"univpll2_d2",
	"univpll2_d4",
	"univpll2_d8",
	"univpll3_d2",
	"univpll3_d4",
	"univpll3_d8",
	"msdcpll_ck",
	"msdcpll_d2",
	"mmpll_ck",
	"mmpll_d2",
	"dmpll_ck",
	"dmpll_d2",
	"dmpll_d4",
	"dmpll_x2",
	"tvdpll_ck",
	"tvdpll_d2",
	"tvdpll_d4",
	"mipipll_ck",
	"mipipll_d2",
	"mipipll_d4",
	"hdmipll_ck",
	"hdmipll_d2",
	"hdmipll_d3",
	"lvdspll_ck",
	"lvdspll_d2",
	"lvdspll_d4",
	"lvdspll_d8",
	"audpll_ck",
	"audpll_d4",
	"audpll_d8",
	"audpll_d16",
	"audpll_d24",
	"32k_internal",
	"32k_external",
	/* TOP */
	"axi_sel",
	"mem_sel",
	"ddrphycfg_sel",
	"mm_sel",
	"pwm_sel",
	"vdec_sel",
	"mfg_sel",
	"camtg_sel",
	"uart_sel",
	"spi0_sel",
	"usb20_sel",
	"msdc30_0_sel",
	"msdc30_1_sel",
	"msdc30_2_sel",
	"audio_sel",
	"aud_intbus_sel",
	"pmicspi_sel",
	"scp_sel",
	"dpi0_sel",
	"dpi1_sel",
	"tve_sel",
	"hdmi_sel",
	"apll_sel",
	"dpilvds_sel",
	"rtc_sel",
	"nfi2x_sel",
	"eth_50m_sel",
	/* INFRA */
	"pmicwrap_ck",
	"pmicspi_ck",
	"irrx_ck",
	"cec_ck",
	"kp_ck",
	"cpum_ck",
	"trng_ck",
	"connsys_bus",
	"m4u_ck",
	"l2c_sram_ck",
	"efuse_ck",
	"audio_ck",
	"smi_ck",
	"dbgclk",
	"clk13m",
	"ckmux_sel",
	"ckdiv1",
	/* PERI */
	"usbslv_ck",
	"usb1_mcu_ck",
	"usb0_mcu_ck",
	"eth_ck",
	"spi0_ck",
	"auxadc_ck",
	"i2c3_ck",
	"i2c2_ck",
	"i2c1_ck",
	"i2c0_ck",
	"bitif_ck",
	"uart3_ck",
	"uart2_ck",
	"uart1_ck",
	"uart0_ck",
	"nli_ck",
	"msdc30_2_ck",
	"msdc30_1_ck",
	"msdc30_0_ck",
	"ap_dma_ck",
	"usb1_ck",
	"usb0_ck",
	"pwm_ck",
	"pwm7_ck",
	"pwm6_ck",
	"pwm5_ck",
	"pwm4_ck",
	"pwm3_ck",
	"pwm2_ck",
	"pwm1_ck",
	"therm_ck",
	"nfi_ck",
	"nfi_pad_ck",
	"nfi_ecc_ck",
	"gcpu_ck",
	/* MM */
	"mm_smi_comm",
	"mm_smi_larb0",
	"mm_cmdq",
	"mm_mutex",
	"mm_disp_color",
	"mm_disp_bls",
	"mm_disp_wdma",
	"mm_disp_rdma",
	"mm_disp_ovl",
	"mm_mdp_tdshp",
	"mm_mdp_wrot",
	"mm_mdp_wdma",
	"mm_mdp_rsz1",
	"mm_mdp_rsz0",
	"mm_mdp_rdma",
	"mm_mdp_bls_26m",
	"mm_cam_mdp",
	"mm_fake_eng",
	"mm_mutex_32k",
	"mm_disp_rdma1",
	"mm_disp_ufoe",
	"mm_dsi_eng",
	"mm_dsi_dig",
	"mm_dpi_digl",
	"mm_dpi_eng",
	"mm_dpi1_digl",
	"mm_dpi1_eng",
	"mm_tve_output",
	"mm_tve_input",
	"mm_hdmi_pixel",
	"mm_hdmi_pll",
	"mm_hdmi_audio",
	"mm_hdmi_spdif",
	"mm_lvds_pixel",
	"mm_lvds_cts",
	/* IMG */
	"larb2_smi",
	"cam_smi",
	"cam_cam",
	"img_sen_tg",
	"img_sen_cam",
	"img_venc_jpgenc",
	/* VDEC */
	"vdec_cken",
	"vdec_larb_cken",
	/* MFG */
	"mfg_bg3d",
	/* end */
	NULL
};

static const char * const compatible[] = {"mediatek,mt8127", NULL};

static struct clkchk_cfg cfg = {
	.aee_excp_on_fail = 1,
	.warn_on_fail = 0,
	.compatible = compatible,
	.off_pll_names = off_pll_names,
	.all_clk_names = all_clk_names,
};

static int __init clkchk_platform_init(void)
{
	return clkchk_init(&cfg);
}
subsys_initcall(clkchk_platform_init);
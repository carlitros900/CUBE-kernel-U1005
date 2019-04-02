#ifndef __MT65XX_LCM_LIST_H__
#define __MT65XX_LCM_LIST_H__

#include <lcm_drv.h>

#if defined(MTK_LCM_DEVICE_TREE_SUPPORT)
extern LCM_DRIVER lcm_common_drv;
#endif
extern LCM_DRIVER p101dbz_3z1_lcm_drv;
extern LCM_DRIVER c805_bw8022_wuxga_boe_lcm_drv;
extern LCM_DRIVER c805_jd9366ab_hx00821_lcm_drv;

extern LCM_DRIVER c710_tpf0702001n_lcm_drv;
extern LCM_DRIVER c710_lt070me05000_wxga_lcm_drv;
extern LCM_DRIVER c102_nt51021_wuxga_lcm_drv;
extern LCM_DRIVER c712_yx070dips30b_10428e_lcm_drv;
extern LCM_DRIVER es6311_20811010280038_nt35521_wxga_lcm_drv;
extern LCM_DRIVER es6311_anx6585_zigzag_wxga_lcm_drv;
extern LCM_DRIVER c710_ek79007_wsvga_dsi_lcm_drv;
extern LCM_DRIVER c708_gx_ek79007_wsvga_ips_cpt_lcm_drv;
extern LCM_DRIVER rc_rgb_wsvga_vgp2_lcm_drv;
extern LCM_DRIVER c86f_rc_rgb_wsvga_lcm_drv;
extern LCM_DRIVER c106_tc358775_ltl106hl01_lcm_drv;
extern LCM_DRIVER c805_hygd_nt35521s_cpt_wxga_ips_101_lcm_drv;
extern LCM_DRIVER c809_sl008pn24d1304_a00_lcm_drv;

#ifdef BUILD_LK
extern void mdelay(unsigned long msec);
#endif

#endif

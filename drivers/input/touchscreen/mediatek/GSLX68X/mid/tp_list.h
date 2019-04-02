#ifndef _MID_TP_LIST_H__
#define _MID_TP_LIST_H__

#include <linux/kernel.h>
#include "tpd_gsl_common.h"

//--------------------------------------------
//.h file
#include "TEST_COMPATIBILITY.h"
#include "K71_XC.h"
#include "C854G_TP_MF595.h"
#include "C854G_TP_MF633.h"
#include "RC15F2_OLM.h"
#include "C102_DPT.h"
#include "C102_DPT_WUXGA.h"
#include "C86_AG.h"
#include "C805G_DX.h"
#include "C805G_HK_WUXGA.h"
#include "C805G_HK_WUXGA_HOME.h"
#include "C805G_HK_WXGA.h"
#include "C805G_WJ.h"
#include "C805G_DZ_WUXGA.h"
#include "C712_gsl1680.h"
#include "C710_gsl3676.h"
#include "C71_GSL1680.h"
#include "C710_XC_WUXGA.h"
#include "C710_HK_WUXGA.h"
#include "RC15F2_YJ467FPC.h"
#include "C106_GSL3680B.h"
#include "C710_QCY_WUXGA.h"
#include "C710_DFL_WUXGA.h"
#include "C710_FC_WUXGA.h"
#include "C710_XC40_WUXGA.h"
#include "C710_DFL_WUXGANL_101.h"
#include "C710_MF_WXGA_101.h"
#include "C809_GSL3676.h"
#include "C809_GSL3692.h"
#include "C708_gsl1680_800x1280.h"
#include "C708_gsl1680_QSF_PG7006_FPC_A0.h"
#include "C708_gsl1680.h"
#include "C708_LC_gsl1680.h"
#include "C708_CZ_WSVGA_7_ONCELL.h"
#include "C710_CY_WXGANL_101.h"
#include "C710_CY_WUXGANL_101.h"
#include "C708_HNFX_WSVGA_7.h"
#include "C708_MR_WXGA_8.h"
#include "C710_JWN_WUXGA.h"
#include "C805G_XC_WXGA_8.h"
#include "C809_HQ_GSL3692.h"
#include "C805G_FC_WXGA.h"
#include "C805G_HC_WXGA.h"
#include "C710_HK_HUARUIAN_WUXGA.h"
#include "C805G_DH_WXGA.h"
#include "C118_GSL3692_FHD.h"
#include "C716_GSL1680_WSVGA.h"
#include "C716_GSL3676_WXGA.h"
#include "C716_SQ_WXGA_101.h"
#include "C716_XC_WXGA.h"
#include "C716_DZ_WXGA.h"
#include "C716_DZ_WUXGA.h"
//--------------------------------------------

//--------------------------------------------
//tp key
#ifdef TPD_HAVE_BUTTON 
static int tpd_keys_local[TPD_KEY_COUNT] = TPD_KEYS;
static int tpd_keys_local_home[1] = {KEY_HOMEPAGE};

#endif
//--------------------------------------------

//--------------------------------------------
//tp_modle
typedef enum {
   K71_tp_gsl_modle,
   C854_tp_gsl_modle,
   RC15f2_tp_gsl_modle,
   C102_tp_gsl_modle,
   C86G_tp_gsl_modle,
   C805G_tp_gsl_modle,
   C716_tp_gsl_modle,
   C712_tp_gsl_modle,
   C710_tp_gsl_modle,
   C71_tp_gsl_modle,
   C106_tp_gsl_modle,
   C809_tp_gsl_modle, 	   
   C708_tp_gsl_modle,
   C118_tp_gsl_modle,
   //default
   default_tp_gsl_modle
}NUM_GSL_MODLE;
//--------------------------------------------




TP_PROJECT_DATA K71_tp_data[]={
    {"K71_XC", {0xff,0xff,0xff,0xff}, gsl_config_data_id_k71_xc, GSLX680_FW_k71_xc, NULL, ARRAY_SIZE(GSLX680_FW_k71_xc)},
};

TP_PROJECT_DATA C854_tp_data[]={
    {"C854G_TP_MF595", {0xff,0xff,0xff,0xff}, gsl_config_data_id_C854G_TP_MF595, GSLX680_FW_C854G_TP_MF595, NULL, ARRAY_SIZE(GSLX680_FW_C854G_TP_MF595)},
    {"C854G_TP_MF633", {0xff,0xff,0xff,0xff}, gsl_config_data_id_C854G_TP_MF633, GSLX680_FW_C854G_TP_MF633, NULL, ARRAY_SIZE(GSLX680_FW_C854G_TP_MF633)},
};

TP_PROJECT_DATA RC15F2_tp_data[]={
    {"RC15F2_OLM", {0xff,0xff,0xff,0xff}, gsl_config_data_id_RC15F2_OLM, GSLX680_FW_RC15F2_OLM, NULL, ARRAY_SIZE(GSLX680_FW_RC15F2_OLM)},
    {"RC15F2_YJ467FPC", {0xff,0xff,0xff,0xff}, gsl_config_data_id_RC15F2_YJ467FPC, GSLX680_FW_RC15F2_YJ467FPC, NULL, ARRAY_SIZE(GSLX680_FW_RC15F2_YJ467FPC)},
};

TP_PROJECT_DATA C102_tp_data[]={
    {"C102_DPT", {0xff,0xff,0xff,0xff}, gsl_config_data_id_C102_DPT, GSLX680_FW_C102_DPT, NULL, ARRAY_SIZE(GSLX680_FW_C102_DPT)},
    {"C102_DPT_WUXGA", {0xff,0xff,0xff,0xff}, gsl_config_data_id_C102_DPT_WUXGA, GSLX680_FW_C102_DPT_WUXGA, NULL, ARRAY_SIZE(GSLX680_FW_C102_DPT_WUXGA)},
};

TP_PROJECT_DATA C86G_tp_data[]={
    {"C86G_AG", {0xff,0xff,0xff,0xff}, gsl_config_data_id_C86_AG, GSLX680_FW_C86_AG, NULL, ARRAY_SIZE(GSLX680_FW_C86_AG)},
};

TP_PROJECT_DATA C805G_tp_data[]={
    {"C805G_DX", {0xff,0xff,0xff,0xff}, gsl_config_data_id_C805G_DX, GSLX680_FW_C805G_DX, NULL, ARRAY_SIZE(GSLX680_FW_C805G_DX)},
    {"C805G_HK_WUXGA", {0xff,0xff,0xff,0xff}, gsl_config_data_id_C805G_HK_WUXGA, GSLX680_FW_C805G_HK_WUXGA, NULL, ARRAY_SIZE(GSLX680_FW_C805G_HK_WUXGA)},
    {"C805G_HK_WXGA", {0xff,0xff,0xff,0xff}, gsl_config_data_id_C805G_HK_WXGA, GSLX680_FW_C805G_HK_WXGA, NULL, ARRAY_SIZE(GSLX680_FW_C805G_HK_WXGA)},
    {"C805G_WJ", {0xff,0xff,0xff,0xff}, gsl_config_data_id_C805G_WJ, GSLX680_FW_C805G_WJ, NULL, ARRAY_SIZE(GSLX680_FW_C805G_WJ)},
    {"C805G_HK_WUXGA_HOME", {0xff,0xff,0xff,0xff}, gsl_config_data_id_C805G_HK_WUXGA_HOME, GSLX680_FW_C805G_HK_WUXGA_HOME, NULL, ARRAY_SIZE(GSLX680_FW_C805G_HK_WUXGA_HOME)},
	{"C805G_XC_WXGA_8", {0xff,0xff,0xff,0xff}, gsl_config_data_id_C805G_XC_WXGA_8, GSLX680_FW_C805G_XC_WXGA_8, NULL, ARRAY_SIZE(GSLX680_FW_C805G_XC_WXGA_8)},
	{"C805G_FC_WXGA", {0xff,0xff,0xff,0xff}, gsl_config_data_id_C805G_FC_WXGA, GSLX680_FW_C805G_FC_WXGA, NULL, ARRAY_SIZE(GSLX680_FW_C805G_FC_WXGA)},
    {"C805G_DZ_WUXGA", {0xff,0xff,0xff,0xff}, gsl_config_data_id_C805G_DZ_WUXGA, GSLX680_FW_C805G_DZ_WUXGA, NULL, ARRAY_SIZE(GSLX680_FW_C805G_DZ_WUXGA)},
    {"C805G_DH_WXGA", {0xff,0xff,0xff,0xff}, gsl_config_data_id_C805G_DH_WXGA, GSLX680_FW_C805G_DH_WXGA, NULL, ARRAY_SIZE(GSLX680_FW_C805G_DH_WXGA)},
    {"C805G_HC_WXGA", {0xff,0xff,0xff,0xff}, gsl_config_data_id_C805G_HC_WXGA, GSLX680_FW_C805G_HC_WXGA, NULL, ARRAY_SIZE(GSLX680_FW_C805G_HC_WXGA)},	
};

TP_PROJECT_DATA C716_tp_data[]={
    {"C716_SQ_WXGA_101", {0xff,0xff,0xff,0xff}, gsl_config_data_id_C716_SQ_WXGA_101, GSLX680_FW_C716_SQ_WXGA_101, NULL, ARRAY_SIZE(GSLX680_FW_C716_SQ_WXGA_101)},
    {"C716_GSL1680_WSVGA", {0xff,0xff,0xff,0xff}, gsl_config_data_id_C716_GSL1680_WSVGA, GSLX680_FW_C716_GSL1680_WSVGA, NULL, ARRAY_SIZE(GSLX680_FW_C716_GSL1680_WSVGA)},
    {"C716_GSL3676_WXGA", {0xff,0xff,0xff,0xff}, gsl_config_data_id_C716_GSL3676_WXGA, GSLX680_FW_C716_GSL3676_WXGA, NULL, ARRAY_SIZE(GSLX680_FW_C716_GSL3676_WXGA)},
    {"C716_XC_WXGA", {0xff,0xff,0xff,0xff}, gsl_config_data_id_C716_XC_WXGA, GSLX680_FW_C716_XC_WXGA, NULL, ARRAY_SIZE(GSLX680_FW_C716_XC_WXGA)},	
    {"C716_DZ_WXGA", {0xff,0xff,0xff,0xff}, gsl_config_data_id_C716_DZ_WXGA, GSLX680_FW_C716_DZ_WXGA, NULL, ARRAY_SIZE(GSLX680_FW_C716_DZ_WXGA)},		
    {"C716_DZ_WUXGA", {0xff,0xff,0xff,0xff}, gsl_config_data_id_C716_DZ_WUXGA, GSLX680_FW_C716_DZ_WUXGA, NULL, ARRAY_SIZE(GSLX680_FW_C716_DZ_WUXGA)},	
};

TP_PROJECT_DATA C712_tp_data[]={
    {"C712_gsl1680", {0xff,0xff,0xff,0xff}, gsl_config_data_id_C712_gsl1680, GSLX680_FW_C712_gsl1680, NULL, ARRAY_SIZE(GSLX680_FW_C712_gsl1680)},
};

TP_PROJECT_DATA C710_tp_data[]={
    {"C710_gsl3676", {0xff,0xff,0xff,0xff}, gsl_config_data_id_C710_gsl3676, GSLX680_FW_C710_gsl3676, NULL, ARRAY_SIZE(GSLX680_FW_C710_gsl3676)},
    {"C710_XC_WUXGA", {0xff,0xff,0xff,0xff}, gsl_config_data_id_C710_XC_WUXGA, GSLX680_FW_C710_XC_WUXGA, NULL, ARRAY_SIZE(GSLX680_FW_C710_XC_WUXGA)},
    {"C710_HK_WUXGA", {0xff,0xff,0xff,0xff}, gsl_config_data_id_C710_HK_WUXGA, GSLX680_FW_C710_HK_WUXGA, NULL, ARRAY_SIZE(GSLX680_FW_C710_HK_WUXGA)},
    {"C710_QCY_WUXGA", {0xff,0xff,0xff,0xff}, gsl_config_data_id_C710_QCY_WUXGA, GSLX680_FW_C710_QCY_WUXGA, NULL, ARRAY_SIZE(GSLX680_FW_C710_QCY_WUXGA)},
    {"C710_DFL_WUXGA", {0xff,0xff,0xff,0xff}, gsl_config_data_id_C710_DFL_WUXGA, GSLX680_FW_C710_DFL_WUXGA, NULL, ARRAY_SIZE(GSLX680_FW_C710_DFL_WUXGA)},
    {"C710_FC_WUXGA", {0xff,0xff,0xff,0xff}, gsl_config_data_id_C710_FC_WUXGA, GSLX680_FW_C710_FC_WUXGA, NULL, ARRAY_SIZE(GSLX680_FW_C710_FC_WUXGA)},	
    {"C710_XC40_WUXGA", {0xff,0xff,0xff,0xff}, gsl_config_data_id_C710_XC40_WUXGA, GSLX680_FW_C710_XC40_WUXGA, NULL, ARRAY_SIZE(GSLX680_FW_C710_XC40_WUXGA)},
	{"C710_DFL_WUXGANL_101", {0xff,0xff,0xff,0xff}, gsl_config_data_id_C710_DFL_WUXGANL_101, GSLX680_FW_C710_DFL_WUXGANL_101, NULL, ARRAY_SIZE(GSLX680_FW_C710_DFL_WUXGANL_101)},
	{"C710_MF_WXGA_101", {0xff,0xff,0xff,0xff}, gsl_config_data_id_C710_MF_WXGA_101, GSLX680_FW_C710_MF_WXGA_101, NULL, ARRAY_SIZE(GSLX680_FW_C710_MF_WXGA_101)},
	{"C710_CY_WXGANL_101", {0xff,0xff,0xff,0xff}, gsl_config_data_id_C710_CY_WXGANL_101, GSLX680_FW_C710_CY_WXGANL_101, NULL, ARRAY_SIZE(GSLX680_FW_C710_CY_WXGANL_101)},
	{"C710_CY_WUXGANL_101", {0xff,0xff,0xff,0xff}, gsl_config_data_id_C710_CY_WUXGANL_101, GSLX680_FW_C710_CY_WUXGANL_101, NULL, ARRAY_SIZE(GSLX680_FW_C710_CY_WUXGANL_101)},
	{"C710_JWN_WUXGA", {0xff,0xff,0xff,0xff}, gsl_config_data_id_C710_JWN_WUXGA, GSLX680_FW_C710_JWN_WUXGA, NULL, ARRAY_SIZE(GSLX680_FW_C710_JWN_WUXGA)},
	{"C710_HK_HUARUIAN_WUXGA", {0xff,0xff,0xff,0xff}, gsl_config_data_id_C710_HK_HUARUIAN_WUXGA, GSLX680_FW_C710_HK_HUARUIAN_WUXGA, NULL, ARRAY_SIZE(GSLX680_FW_C710_HK_HUARUIAN_WUXGA)},
};

TP_PROJECT_DATA C71_tp_data[]={
    {"C71_GSL1680", {0xff,0xff,0xff,0xff}, gsl_config_data_id_C71_GSL1680, GSLX680_FW_C71_GSL1680, NULL, ARRAY_SIZE(GSLX680_FW_C71_GSL1680)},
};
TP_PROJECT_DATA C106_tp_data[]={
    {"C106_GSL3680B", {0xff,0xff,0xff,0xff}, gsl_config_data_id_C106_GSL3680B, GSLX680_FW_C106_GSL3680B, NULL, ARRAY_SIZE(GSLX680_FW_C106_GSL3680B)},
};

TP_PROJECT_DATA C809_tp_data[]={
    {"C809_GSL3676", {0xff,0xff,0xff,0xff}, gsl_config_data_id_C809_GSL3676, GSLX680_FW_C809_GSL3676, NULL, ARRAY_SIZE(GSLX680_FW_C809_GSL3676)},
    {"C809_GSL3692", {0xff,0xff,0xff,0xff}, gsl_config_data_id_C809_GSL3692, GSLX680_FW_C809_GSL3692, NULL, ARRAY_SIZE(GSLX680_FW_C809_GSL3692)},
    {"C809_HQ_GSL3692", {0xff,0xff,0xff,0xff}, gsl_config_data_id_C809_HQ_GSL3692, GSLX680_FW_C809_HQ_GSL3692, NULL, ARRAY_SIZE(GSLX680_FW_C809_HQ_GSL3692)},
};

TP_PROJECT_DATA C708_tp_data[]={
    {"C708_gsl1680_800x1280", {0xff,0xff,0xff,0xff}, gsl_config_data_id_C708_gsl1680_800x1280, GSLX680_FW_C708_gsl1680_800x1280, NULL, ARRAY_SIZE(GSLX680_FW_C708_gsl1680_800x1280)},
    {"C708_CZ_WSVGA_7_ONCELL", {0xff,0xff,0xff,0xff}, gsl_config_data_id_C708_CZ_WSVGA_7_ONCELL, GSLX680_FW_C708_CZ_WSVGA_7_ONCELL, NULL, ARRAY_SIZE(GSLX680_FW_C708_CZ_WSVGA_7_ONCELL)},
    {"C708_gsl1680", {0xff,0xff,0xff,0xff}, gsl_config_data_id_C708_gsl1680, GSLX680_FW_C708_gsl1680, NULL, ARRAY_SIZE(GSLX680_FW_C708_gsl1680)},
    {"C708_LC_gsl1680", {0xff,0xff,0xff,0xff}, gsl_config_data_id_C708_LC_gsl1680, GSLX680_FW_C708_LC_gsl1680, NULL, ARRAY_SIZE(GSLX680_FW_C708_LC_gsl1680)},	
    {"C708_gsl1680_QSF_PG7006_FPC_A0", {0xff,0xff,0xff,0xff}, gsl_config_data_id_C708_gsl1680_QSF_PG7006_FPC_A0, GSLX680_FW_C708_gsl1680_QSF_PG7006_FPC_A0, NULL, ARRAY_SIZE(GSLX680_FW_C708_gsl1680_QSF_PG7006_FPC_A0)},
	{"C708_HNFX_WSVGA_7", {0xff,0xff,0xff,0xff}, gsl_config_data_id_C708_HNFX_WSVGA_7, GSLX680_FW_C708_HNFX_WSVGA_7, NULL, ARRAY_SIZE(GSLX680_FW_C708_HNFX_WSVGA_7)},
	{"C708_MR_WXGA_8", {0xff,0xff,0xff,0xff}, gsl_config_data_id_C708_MR_WXGA_8, GSLX680_FW_C708_MR_WXGA_8, NULL, ARRAY_SIZE(GSLX680_FW_C708_MR_WXGA_8)},
	};

TP_PROJECT_DATA C118_tp_data[]={
    {"C118_GSL3692_FHD", {0xff,0xff,0xff,0xff}, gsl_config_data_id_C118_GSL3692_FHD, GSLX680_FW_C118_GSL3692_FHD, NULL, ARRAY_SIZE(GSLX680_FW_C118_GSL3692_FHD)},
};

ALL_TP_INFO gsl_tp_info[] = {
   {"K71",    K71_tp_gsl_modle,   K71_tp_data,  0, ARRAY_SIZE(K71_tp_data),  0},
   {"C854",    C854_tp_gsl_modle,   C854_tp_data,  0, ARRAY_SIZE(C854_tp_data),  0},
   {"RC15F2",    RC15f2_tp_gsl_modle,   RC15F2_tp_data,  0, ARRAY_SIZE(RC15F2_tp_data),  0},
   {"C102",    C102_tp_gsl_modle,   C102_tp_data,  0, ARRAY_SIZE(C102_tp_data),  0},
   {"C86G",    C86G_tp_gsl_modle,   C86G_tp_data,  0, ARRAY_SIZE(C86G_tp_data),  0},
   {"C805G",    C805G_tp_gsl_modle,   C805G_tp_data,  0, ARRAY_SIZE(C805G_tp_data),  0},
   {"C716",    C716_tp_gsl_modle,   C716_tp_data,  0, ARRAY_SIZE(C716_tp_data),  0},
   {"C712",    C712_tp_gsl_modle,   C712_tp_data,  0, ARRAY_SIZE(C712_tp_data),  0},
   {"C710",    C710_tp_gsl_modle,   C710_tp_data,  0, ARRAY_SIZE(C710_tp_data),  0},
   {"C71",    C71_tp_gsl_modle,   C71_tp_data,  0, ARRAY_SIZE(C71_tp_data),  0},
   {"C106",    C106_tp_gsl_modle,   C106_tp_data,  0, ARRAY_SIZE(C106_tp_data),  0},   
   {"C809",    C809_tp_gsl_modle,   C809_tp_data,  0, ARRAY_SIZE(C809_tp_data),  0}, 
   {"C708",    C708_tp_gsl_modle,   C708_tp_data,  0, ARRAY_SIZE(C708_tp_data),  0},
   {"C118",    C118_tp_gsl_modle,   C118_tp_data,  0, ARRAY_SIZE(C118_tp_data),  0}, 	   
};

#endif

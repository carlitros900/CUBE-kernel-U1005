#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>

#include "kd_camera_hw.h"
#include "kd_camera_typedef.h"

#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_camera_feature.h"
#include <mt-plat/upmu_common.h>
#include <mt-plat/items.h>   // buff size  should be ITEM_MAX_LEN   otherwise kernel will panic

/******************************************************************************
 * Debug configuration
******************************************************************************/
#define PFX "[kd_camera_hw]"
#define PK_DBG_NONE(fmt, arg...)    do {} while (0)
#define PK_DBG_FUNC(fmt, args...)    printk(PFX  fmt, ##args)

#define DEBUG_CAMERA_HW_K
#ifdef DEBUG_CAMERA_HW_K
#define PK_DBG PK_DBG_FUNC
#define PK_ERR(fmt, arg...) pr_err(fmt, ##arg)
#define PK_XLOG_INFO(fmt, args...)  printk(PFX  fmt, ##args)

#else
#define PK_DBG(a, ...)
#define PK_ERR(a, ...)
#define PK_XLOG_INFO(fmt, args...)
#endif
#ifdef CONFIG_ES6311_PROJECT
extern void mt_led_ch1_poweron(int level);//Hexh 20160423 add
#endif
/* GPIO Pin control*/
struct platform_device *cam_plt_dev = NULL;
struct pinctrl *camctrl = NULL;
struct pinctrl_state *cam0_pnd_h = NULL;
struct pinctrl_state *cam0_pnd_l = NULL;
struct pinctrl_state *cam0_rst_h = NULL;
struct pinctrl_state *cam0_rst_l = NULL;
struct pinctrl_state *cam1_pnd_h = NULL;
struct pinctrl_state *cam1_pnd_l = NULL;
struct pinctrl_state *cam1_rst_h = NULL;
struct pinctrl_state *cam1_rst_l = NULL;
struct pinctrl_state *cam_ldo0_h = NULL;
struct pinctrl_state *cam_ldo0_l = NULL;

/*=============================================================*/
// voltage can supply see Pmic.c (drivers\misc\mediatek\power\mt6735)
#define VOL_2800 2800000
#define VOL_1800 1800000
#define VOL_1500 1500000
#define VOL_1200 1200000
#define VOL_1000 1000000

#define IMG_NAME_LEN 32
typedef struct img_info {
	char name[IMG_NAME_LEN];
	int avdd;	// avdd
	int dvdd;	// dvdd
	int iovdd;	// iovdd
	int afvdd;	// afvdd
	u32 rst_on;			// sensor work
	u32 rst_off;		// sensor reset
	u32 pdn_on;			// sensor work
	u32 pdn_off;		// sensor sleep
}IMG_INFO_STRUCT;

IMG_INFO_STRUCT camera_list[] = {
//----30w-----
	{"hm5040mipiraw", 	VOL_2800, 		VOL_1200, 		VOL_1800, 		VOL_2800, GPIO_OUT_ONE, GPIO_OUT_ZERO, GPIO_OUT_ONE,  GPIO_OUT_ZERO},
	//{"gc030amipiraw", 	VOL_2800, 		VOL_1800, 		VOL_1800, 		0, 		  GPIO_OUT_ONE, GPIO_OUT_ZERO, GPIO_OUT_ZERO, GPIO_OUT_ONE},
	{"gc0310mipiyuv", 	VOL_2800, 		VOL_1800, 		VOL_1800, 		0, 		  GPIO_OUT_ONE, GPIO_OUT_ZERO, GPIO_OUT_ZERO, GPIO_OUT_ONE},
	{"gc030amipiraw", 	VOL_2800, 		VOL_1800, 		VOL_1800, 		0, 		  GPIO_OUT_ONE, GPIO_OUT_ZERO, GPIO_OUT_ZERO, GPIO_OUT_ONE},
	{"gc033amipiraw", 	VOL_2800, 		VOL_1800, 		VOL_1800, 		0, 		  GPIO_OUT_ONE, GPIO_OUT_ZERO, GPIO_OUT_ZERO, GPIO_OUT_ONE},
	//{"gc0409mipiraw", 	VOL_2800, 		VOL_1800, 		VOL_1800, 		0, 		  GPIO_OUT_ONE, GPIO_OUT_ZERO, GPIO_OUT_ZERO, GPIO_OUT_ONE},
	{"gc0328yuv", 	    VOL_2800, 		VOL_1800, 		VOL_1800, 		0, 		  GPIO_OUT_ONE, GPIO_OUT_ZERO, GPIO_OUT_ZERO, GPIO_OUT_ONE},
	{"gc0312yuv", 		VOL_2800, 		VOL_1500, 		VOL_1800, 		0, 		  GPIO_OUT_ONE, GPIO_OUT_ZERO, GPIO_OUT_ZERO, GPIO_OUT_ONE},
	{"sp0a09mipiraw", 	VOL_2800, 		VOL_1800, 		VOL_1800, 		0,		  GPIO_OUT_ONE, GPIO_OUT_ZERO, GPIO_OUT_ZERO, GPIO_OUT_ONE},
//----200w----
	{"bf2206mipiraw", 	VOL_2800, 		VOL_1800, 		VOL_1800, 		0,		  GPIO_OUT_ONE, GPIO_OUT_ZERO, GPIO_OUT_ZERO, GPIO_OUT_ONE},

	{"gc2145mipiyuv", 	VOL_2800, 		VOL_1800, 		VOL_1800, 		0, 		  GPIO_OUT_ONE, GPIO_OUT_ZERO, GPIO_OUT_ZERO, GPIO_OUT_ONE},
	{"gc2385mipiraw", 	VOL_2800, 		VOL_1200, 		VOL_1800, 		0,		  GPIO_OUT_ONE, GPIO_OUT_ZERO, GPIO_OUT_ONE,  GPIO_OUT_ZERO},
	{"gc2235mipiraw", 	VOL_2800, 		VOL_1800, 		VOL_1800, 		0,		  GPIO_OUT_ONE, GPIO_OUT_ZERO, GPIO_OUT_ZERO, GPIO_OUT_ONE},
	{"gc2235submipiraw", 	VOL_2800, 		VOL_1800, 		VOL_1800, 		0,		  GPIO_OUT_ONE, GPIO_OUT_ZERO, GPIO_OUT_ZERO, GPIO_OUT_ONE},
	{"gc2365mipiraw", 	VOL_2800, 		VOL_1800, 		VOL_1800, 		0,		  GPIO_OUT_ONE, GPIO_OUT_ZERO, GPIO_OUT_ZERO, GPIO_OUT_ONE},
	{"gc2365submipiraw", 	VOL_2800, 		VOL_1800, 		VOL_1800, 		0,		  GPIO_OUT_ONE, GPIO_OUT_ZERO, GPIO_OUT_ZERO, GPIO_OUT_ONE},
	{"gc2035yuv", 		VOL_2800, 		VOL_1500, 		VOL_1800, 		0, 		  GPIO_OUT_ONE, GPIO_OUT_ZERO, GPIO_OUT_ZERO, GPIO_OUT_ONE},	
	{"gc0329yuv", 		VOL_2800, 		VOL_1500, 		VOL_1800, 		0, 		  GPIO_OUT_ONE, GPIO_OUT_ZERO, GPIO_OUT_ZERO, GPIO_OUT_ONE},
    {"ov2680mipiraw", 	VOL_2800, 		VOL_1500, 		VOL_1800, 		0,		  GPIO_OUT_ONE, GPIO_OUT_ZERO, GPIO_OUT_ONE, GPIO_OUT_ZERO},
    {"gc2145yuv", 	    VOL_2800, 		VOL_1800, 		VOL_1800, 		0,		  GPIO_OUT_ONE, GPIO_OUT_ZERO, GPIO_OUT_ZERO, GPIO_OUT_ONE},
	{"gc2355raw", 		VOL_2800, 		VOL_1800, 		VOL_1800, 		0,		  GPIO_OUT_ONE, GPIO_OUT_ZERO, GPIO_OUT_ZERO, GPIO_OUT_ONE},
	{"gc2355mipiraw", 	VOL_2800, 		VOL_1800, 		VOL_1800, 		0,		  GPIO_OUT_ONE, GPIO_OUT_ZERO, GPIO_OUT_ZERO, GPIO_OUT_ONE},
	//{"gc2755mipiraw", 	VOL_2800, 		VOL_1800, 		VOL_1800, 		0,		  GPIO_OUT_ONE, GPIO_OUT_ZERO, GPIO_OUT_ZERO, GPIO_OUT_ONE},
	//{"gc2145mipiyuv", 	VOL_2800, 		VOL_1800, 		VOL_1800, 		0, 		  GPIO_OUT_ONE, GPIO_OUT_ZERO, GPIO_OUT_ZERO, GPIO_OUT_ONE},
	//{"gc2155mipiyuv", 	VOL_2800, 		VOL_1800, 		VOL_1800, 		0, 		  GPIO_OUT_ONE, GPIO_OUT_ZERO, GPIO_OUT_ZERO, GPIO_OUT_ONE},
	//{"gc2355mipiraw", 	VOL_2800, 		VOL_1800, 		VOL_1800, 		0,		  GPIO_OUT_ONE, GPIO_OUT_ZERO, GPIO_OUT_ZERO, GPIO_OUT_ONE},
	//{"gc2365mipiraw", 	VOL_2800, 		VOL_1800, 		VOL_1800, 		0,		  GPIO_OUT_ONE, GPIO_OUT_ZERO, GPIO_OUT_ZERO, GPIO_OUT_ONE},
	//{"sp2508mipiraw", 	VOL_2800, 		VOL_1800, 		VOL_1800, 		0,		  GPIO_OUT_ONE, GPIO_OUT_ZERO, GPIO_OUT_ZERO, GPIO_OUT_ONE},	
	{"sp2509mipiraw", 	VOL_2800, 		VOL_1800, 		VOL_1800, 		0,		  GPIO_OUT_ONE, GPIO_OUT_ZERO, GPIO_OUT_ZERO, GPIO_OUT_ONE},	
	{"sp250amipiraw", 	VOL_2800, 		VOL_1800, 		VOL_1800, 		VOL_2800, GPIO_OUT_ONE, GPIO_OUT_ZERO, GPIO_OUT_ZERO, GPIO_OUT_ONE},
//----500w----
    {"ov8858mipiraw", 	VOL_2800, 		VOL_1500, 		VOL_1800, 		VOL_2800, GPIO_OUT_ONE, GPIO_OUT_ZERO, GPIO_OUT_ONE, GPIO_OUT_ZERO},
	{"gc5024mipiraw", 	VOL_2800, 		VOL_1500, 		VOL_1800, 		VOL_2800, GPIO_OUT_ONE, GPIO_OUT_ZERO, GPIO_OUT_ZERO, GPIO_OUT_ONE},
	{"gc5004_raw", 	    VOL_2800, 		VOL_1500, 		VOL_1800, 		VOL_2800, GPIO_OUT_ONE, GPIO_OUT_ZERO, GPIO_OUT_ZERO, GPIO_OUT_ONE},
	{"gc5005mipiraw", 	VOL_2800, 		VOL_1200, 		VOL_1800, 		VOL_2800, GPIO_OUT_ONE, GPIO_OUT_ZERO, GPIO_OUT_ZERO, GPIO_OUT_ONE},
	{"gc5025mipiraw", 	VOL_2800, 		VOL_1800, 		VOL_1800, 		VOL_2800, GPIO_OUT_ONE, GPIO_OUT_ZERO, GPIO_OUT_ONE,GPIO_OUT_ZERO },
    {"ov5648mipi", 	    VOL_2800, 		VOL_1500, 		VOL_1800, 		VOL_2800, GPIO_OUT_ONE, GPIO_OUT_ZERO, GPIO_OUT_ONE, GPIO_OUT_ZERO},
	//{"gc5024mipiraw", 	VOL_2800, 		VOL_1500, 		VOL_1800, 		VOL_2800, GPIO_OUT_ONE, GPIO_OUT_ZERO, GPIO_OUT_ZERO, GPIO_OUT_ONE},
	//{"sp5409mipiraw", 	VOL_2800, 		VOL_1800, 		VOL_1800, 		VOL_2800, GPIO_OUT_ONE, GPIO_OUT_ZERO, GPIO_OUT_ZERO, GPIO_OUT_ONE},
	{"sp5506mipiraw",   VOL_2800,       VOL_1200,       VOL_1800,       VOL_2800, GPIO_OUT_ONE, GPIO_OUT_ZERO, GPIO_OUT_ONE,  GPIO_OUT_ZERO},	
//----800w----	
	{"gc8024mipiraw", 	VOL_2800, 		VOL_1200, 		VOL_1800, 		VOL_2800, GPIO_OUT_ONE, GPIO_OUT_ZERO, GPIO_OUT_ZERO, GPIO_OUT_ONE},
	//{"gc8003mipiraw", 	VOL_2800, 		VOL_1500, 		VOL_1800, 		VOL_2800, GPIO_OUT_ONE, GPIO_OUT_ZERO, GPIO_OUT_ZERO, GPIO_OUT_ONE},
    {"ov5670mipiraw", 	VOL_2800, 		VOL_1800, 		VOL_1800, 		0,		  GPIO_OUT_ONE, GPIO_OUT_ZERO, GPIO_OUT_ONE, GPIO_OUT_ZERO},
	
};

/*=================== items related =========================*/
enum {
	CAMIF_UNKNOW,
	CAMIF_MIPI_MIPI,
	CAMIF_DVP_MIPI,
	CAMIF_DVP_DVP,
	CAMIF_MIPI_DVP
};
static int g_camif = CAMIF_UNKNOW;  // interface type, to filter unmatched sensor
static int parse_items(void)
{
	char maincam[ITEM_MAX_LEN], subcam[ITEM_MAX_LEN];   // should be 

	if(item_exist("camera.main.interface") && item_exist("camera.sub.interface")) {	
		item_string(maincam,"camera.main.interface",0);
		item_string(subcam,"camera.sub.interface",0);
		if( (0 == strcmp(maincam, "mipi")) && (0 == strcmp(subcam, "mipi")) ) {g_camif = CAMIF_MIPI_MIPI; }
		else if( (0 == strcmp(maincam, "dvp")) && (0 == strcmp(subcam, "dvp")) ) {g_camif = CAMIF_DVP_DVP; }
		else if( (0 == strcmp(maincam, "mipi")) && (0 == strcmp(subcam, "dvp")) ){ g_camif = CAMIF_MIPI_DVP; }
		else if( (0 == strcmp(maincam, "dvp")) && (0 == strcmp(subcam, "mipi")) ){ g_camif = CAMIF_DVP_MIPI; }
		printk("\n[kd_camera_hw: %s] maincam is %s, subcam is %s. \n", __func__, maincam, subcam);
	} 
	return 0;	
}
static int interface_check(u32 pinSetIdx, const char *currSensorName)
{
	int ret = 0;
	switch(g_camif){
		case CAMIF_DVP_MIPI:
			if((pinSetIdx==0) && (NULL!=strstr(currSensorName, "mipi")))
				ret = -1;
			if((pinSetIdx==1) && (NULL==strstr(currSensorName, "mipi")))
				ret = -1;
			break;
		case CAMIF_MIPI_MIPI:
			if(NULL==strstr(currSensorName, "mipi"))
				ret = -1;
			break;
		case CAMIF_DVP_DVP:
			if(NULL!=strstr(currSensorName, "mipi"))
				ret = -1;		
			break;
		case CAMIF_MIPI_DVP:
			if((pinSetIdx==0) && (NULL==strstr(currSensorName, "mipi")))
				ret = -1;
			if((pinSetIdx==1) && (NULL!=strstr(currSensorName, "mipi")))
				ret = -1;
			break;	
		default:
			break;					
	}
	return ret;
}
int mtkcam_gpio_init(struct platform_device *pdev)
{
	int ret = 0;

	camctrl = devm_pinctrl_get(&pdev->dev);
	if (IS_ERR(camctrl)) {
		dev_err(&pdev->dev, "Cannot find camera pinctrl!");
		ret = PTR_ERR(camctrl);
	}
	/*Cam0 Power/Rst Ping initialization */
	cam0_pnd_h = pinctrl_lookup_state(camctrl, "cam0_pnd1");
	if (IS_ERR(cam0_pnd_h)) {
		ret = PTR_ERR(cam0_pnd_h);
		pr_debug("%s : pinctrl err, cam0_pnd_h\n", __func__);
	}

	cam0_pnd_l = pinctrl_lookup_state(camctrl, "cam0_pnd0");
	if (IS_ERR(cam0_pnd_l)) {
		ret = PTR_ERR(cam0_pnd_l);
		pr_debug("%s : pinctrl err, cam0_pnd_l\n", __func__);
	}


	cam0_rst_h = pinctrl_lookup_state(camctrl, "cam0_rst1");
	if (IS_ERR(cam0_rst_h)) {
		ret = PTR_ERR(cam0_rst_h);
		pr_debug("%s : pinctrl err, cam0_rst_h\n", __func__);
	}

	cam0_rst_l = pinctrl_lookup_state(camctrl, "cam0_rst0");
	if (IS_ERR(cam0_rst_l)) {
		ret = PTR_ERR(cam0_rst_l);
		pr_debug("%s : pinctrl err, cam0_rst_l\n", __func__);
	}

	/*Cam1 Power/Rst Ping initialization */
	cam1_pnd_h = pinctrl_lookup_state(camctrl, "cam1_pnd1");
	if (IS_ERR(cam1_pnd_h)) {
		ret = PTR_ERR(cam1_pnd_h);
		pr_debug("%s : pinctrl err, cam1_pnd_h\n", __func__);
	}

	cam1_pnd_l = pinctrl_lookup_state(camctrl, "cam1_pnd0");
	if (IS_ERR(cam1_pnd_l)) {
		ret = PTR_ERR(cam1_pnd_l);
		pr_debug("%s : pinctrl err, cam1_pnd_l\n", __func__);
	}


	cam1_rst_h = pinctrl_lookup_state(camctrl, "cam1_rst1");
	if (IS_ERR(cam1_rst_h)) {
		ret = PTR_ERR(cam1_rst_h);
		pr_debug("%s : pinctrl err, cam1_rst_h\n", __func__);
	}


	cam1_rst_l = pinctrl_lookup_state(camctrl, "cam1_rst0");
	if (IS_ERR(cam1_rst_l)) {
		ret = PTR_ERR(cam1_rst_l);
		pr_debug("%s : pinctrl err, cam1_rst_l\n", __func__);
	}
	/*externel LDO enable */
	cam_ldo0_h = pinctrl_lookup_state(camctrl, "cam_ldo0_1");
	if (IS_ERR(cam_ldo0_h)) {
		ret = PTR_ERR(cam_ldo0_h);
		pr_debug("%s : pinctrl err, cam_ldo0_h\n", __func__);
	}


	cam_ldo0_l = pinctrl_lookup_state(camctrl, "cam_ldo0_0");
	if (IS_ERR(cam_ldo0_l)) {
		ret = PTR_ERR(cam_ldo0_l);
		pr_debug("%s : pinctrl err, cam_ldo0_l\n", __func__);
	}
	return ret;
}

int mtkcam_gpio_set(int PinIdx, int PwrType, int Val)
{
	int ret = 0;

	switch (PwrType) {
	case CAMRST:
		if (PinIdx == 0) {
			if (Val == 0)
				pinctrl_select_state(camctrl, cam0_rst_l);
			else
				pinctrl_select_state(camctrl, cam0_rst_h);
		} else {
			if (Val == 0)
				pinctrl_select_state(camctrl, cam1_rst_l);
			else
				pinctrl_select_state(camctrl, cam1_rst_h);
		}
		break;
	case CAMPDN:
		if (PinIdx == 0) {
			if (Val == 0)
				pinctrl_select_state(camctrl, cam0_pnd_l);
			else
				pinctrl_select_state(camctrl, cam0_pnd_h);
		} else {
			if (Val == 0)
				pinctrl_select_state(camctrl, cam1_pnd_l);
			else
				pinctrl_select_state(camctrl, cam1_pnd_h);
		}

		break;
	case CAMLDO:
		if (Val == 0)
			pinctrl_select_state(camctrl, cam_ldo0_l);
		else
			pinctrl_select_state(camctrl, cam_ldo0_h);
		break;
	default:
		PK_DBG("PwrType(%d) is invalid !!\n", PwrType);
		break;
	};

	return ret;
}




int cntVCAMD = 0;
int cntVCAMA = 0;
int cntVCAMIO = 0;
int cntVCAMAF = 0;
int cntVCAMD_SUB = 0;

static DEFINE_SPINLOCK(kdsensor_pw_cnt_lock);


bool _hwPowerOnCnt(int PinIdx, KD_REGULATOR_TYPE_T powerId, int powerVolt, char *mode_name)
{
	if (_hwPowerOn(PinIdx, powerId, powerVolt)) {
		spin_lock(&kdsensor_pw_cnt_lock);
		if (powerId == VCAMD)
			cntVCAMD += 1;
		else if (powerId == VCAMA)
			cntVCAMA += 1;
		else if (powerId == VCAMIO)
			cntVCAMIO += 1;
		else if (powerId == VCAMIO)
			cntVCAMAF += 1;
		else if (powerId == SUB_VCAMD)
			cntVCAMD_SUB += 1;
		spin_unlock(&kdsensor_pw_cnt_lock);
		return true;
	}
	return false;
}

bool _hwPowerDownCnt(int PinIdx, KD_REGULATOR_TYPE_T powerId, char *mode_name)
{

	if (_hwPowerDown(PinIdx, powerId)) {
		spin_lock(&kdsensor_pw_cnt_lock);
		if (powerId == VCAMD)
			cntVCAMD -= 1;
		else if (powerId == VCAMA)
			cntVCAMA -= 1;
		else if (powerId == VCAMIO)
			cntVCAMIO -= 1;
		else if (powerId == VCAMAF)
			cntVCAMAF -= 1;
		else if (powerId == SUB_VCAMD)
			cntVCAMD_SUB -= 1;
		spin_unlock(&kdsensor_pw_cnt_lock);
		return true;
	}
	return false;
}

void checkPowerBeforClose(int PinIdx, char *mode_name)
{

	int i = 0;

	for (i = 0; i < cntVCAMD; i++)
		_hwPowerDown(PinIdx, VCAMD);
	for (i = 0; i < cntVCAMA; i++)
		_hwPowerDown(PinIdx, VCAMA);
	for (i = 0; i < cntVCAMIO; i++)
		_hwPowerDown(PinIdx, VCAMIO);
	for (i = 0; i < cntVCAMAF; i++)
		_hwPowerDown(PinIdx, VCAMAF);
	for (i = 0; i < cntVCAMD_SUB; i++)
		_hwPowerDown(PinIdx, SUB_VCAMD);

	cntVCAMD = 0;
	cntVCAMA = 0;
	cntVCAMIO = 0;
	cntVCAMAF = 0;
	cntVCAMD_SUB = 0;

}



int kdCISModulePowerOn(CAMERA_DUAL_CAMERA_SENSOR_ENUM SensorIdx, char *currSensorName, bool On,
		       char *mode_name)
{

	static int first_in = 1;  // tongjun@MID add for do items parse
	u32 pinSetIdx = 0;/* default main sensor */
	int drvIdx;
	int sizeoflist = sizeof(camera_list) / sizeof(IMG_INFO_STRUCT);

	if(first_in){
		first_in = 0;
		parse_items();
	}

	if (DUAL_CAMERA_MAIN_SENSOR == SensorIdx){
		if(0 == strcmp(currSensorName, "gc2235submipiraw")){
			return -EIO;
		}
		if(0 == strcmp(currSensorName, "gc2365submipiraw")){
			return -EIO;
		}
		pinSetIdx = 0;
	}
	 else if (DUAL_CAMERA_SUB_SENSOR == SensorIdx){
		if(0 == strcmp(currSensorName, "gc2235mipiraw")){
			return -EIO;
		}
		if(0 == strcmp(currSensorName, "gc2365mipiraw")){
			return -EIO;
		}
		pinSetIdx = 1;			
	 }
	 else if (DUAL_CAMERA_MAIN_2_SENSOR == SensorIdx)
		pinSetIdx = 2;

	for (drvIdx = 0; drvIdx < sizeoflist; drvIdx++){
		if(strcmp(currSensorName, camera_list[drvIdx].name) == 0)
			break;
	}
	if (drvIdx == sizeoflist){
		PK_DBG("list of designated camera isn't exist currSensorName =%s\n", currSensorName);
		return -EIO;
	}
	if(interface_check(pinSetIdx, currSensorName) !=  0){
		printk("%s fail to match interafce\n", currSensorName);
		return -EIO; 
	}

	// if(!should_power(currSensorName,pinSetIdx)){
	// 	printk("%s not allowed to power \n", currSensorName);
	// 	return -EIO; 	
	// }

		// ½»»»Ç°ºóÉãÏñÍ·
	if(item_exist("camera.swap")){
		if(1==item_integer("camera.swap",0)){
			pinSetIdx = pinSetIdx==1 ? 0 : 1;
		}
	}
	
	printk("lingyq_hw [Power: %s]pinSetIdx:%d, currSensorName: %s\n", On?"ON":"OFF", pinSetIdx, currSensorName);
	if (On) {
		ISP_MCLK1_EN(1);
		/* First Power Pin low and Reset Pin Low */
		mtkcam_gpio_set(pinSetIdx, CAMPDN, camera_list[drvIdx].pdn_off);
		mdelay(5);
		mtkcam_gpio_set(pinSetIdx, CAMRST, camera_list[drvIdx].rst_off);
		mdelay(5);

		/* VCAM_D */
		if (TRUE != _hwPowerOnCnt(pinSetIdx, VCAMD,camera_list[drvIdx].dvdd, mode_name)) {
			PK_DBG("[CAMERA SENSOR] Fail to enable analog power (VCAM_D), power id = %d\n", VCAMD);
			goto _kdCISModulePowerOn_exit_;
		}
       	mdelay(5);

		/* VCAM_IO */
		if (TRUE != _hwPowerOnCnt(pinSetIdx, VCAMIO, camera_list[drvIdx].iovdd, mode_name)) {
			PK_DBG("[CAMERA SENSOR] Fail to enable IO power (VCAM_IO), power id = %d\n", VCAMIO);
			goto _kdCISModulePowerOn_exit_;
		}
        mdelay(5);
				
        /* VCAM_A */
		if (TRUE != _hwPowerOnCnt(pinSetIdx, VCAMA, camera_list[drvIdx].avdd, mode_name)) {
			PK_DBG("[CAMERA SENSOR] Fail to enable analog power (VCAM_A), power id = %d\n", VCAMA);
			goto _kdCISModulePowerOn_exit_;
		}
        mdelay(1);

		/* AF_VCC  no AF, do not power it.*/
		if (camera_list[drvIdx].afvdd && (TRUE != _hwPowerOnCnt(pinSetIdx, VCAMAF, camera_list[drvIdx].afvdd, mode_name))) {
			PK_DBG("[CAMERA SENSOR] Fail to enable AF power (VCAM_AF), power id = %d\n", VCAMAF);
			goto _kdCISModulePowerOn_exit_;
		}

		mtkcam_gpio_set(1-pinSetIdx, CAMPDN, camera_list[drvIdx].pdn_off);
        mdelay(10);
		mtkcam_gpio_set(1-pinSetIdx, CAMRST, camera_list[drvIdx].rst_off);
		mdelay(5);
		mtkcam_gpio_set(pinSetIdx, CAMPDN, camera_list[drvIdx].pdn_on);
		mdelay(5);
		mtkcam_gpio_set(pinSetIdx, CAMRST, camera_list[drvIdx].rst_on);
		mdelay(5);
#ifdef CONFIG_ES6311_PROJECT
        if (pinSetIdx == 1 && currSensorName && (0 == strcmp(currSensorName, SENSOR_DRVNAME_OV2680_MIPI_RAW))) {
			 mt_led_ch1_poweron(1);//Hexh 20160423 add
		}
#endif
	}
/*************************************************** power OFF *********************************************/
	else{ 
		ISP_MCLK1_EN(0);
		mtkcam_gpio_set(pinSetIdx, CAMPDN, camera_list[drvIdx].pdn_off);
		mdelay(5);
		mtkcam_gpio_set(pinSetIdx, CAMRST, camera_list[drvIdx].rst_off);
		mdelay(5);
    
		/* VCAM_A */
		if (TRUE != _hwPowerDownCnt(pinSetIdx, VCAMA, mode_name)) {
			PK_DBG("[CAMERA SENSOR] Fail to OFF analog power (VCAM_A),power id= (%d)\n", VCAMA);
			/* return -EIO; */
			goto _kdCISModulePowerOn_exit_;
		}
    
		/* VCAM_IO */
		if (TRUE != _hwPowerDownCnt(pinSetIdx, VCAMIO, mode_name)) {
			PK_DBG("[CAMERA SENSOR] Fail to OFF digital power (VCAM_IO),power id = %d\n", VCAMIO);
			/* return -EIO; */
			goto _kdCISModulePowerOn_exit_;
		}
    
		/* VCAM_D */
		if (TRUE != _hwPowerDownCnt(pinSetIdx, VCAMD, mode_name)) {
			PK_DBG("[CAMERA SENSOR] Fail to OFF digital power (VCAM_IO),power id = %d\n", VCAMIO);
			/* return -EIO; */
			goto _kdCISModulePowerOn_exit_;
		}
    
		/* AF_VCC */
		if (camera_list[drvIdx].afvdd &&  (TRUE != _hwPowerDownCnt(pinSetIdx, VCAMAF, mode_name))) {
			PK_DBG("[CAMERA SENSOR] Fail to OFF AF power (VCAM_AF),power id = %d\n", VCAMAF);
			/* return -EIO; */
			goto _kdCISModulePowerOn_exit_;
		}
#ifdef CONFIG_ES6311_PROJECT
        if (pinSetIdx == 1 && currSensorName && (0 == strcmp(currSensorName, SENSOR_DRVNAME_OV2680_MIPI_RAW))) {
			 mt_led_ch1_poweron(0);//Hexh 20160423 add
		}
#endif
	}

	return 0;

_kdCISModulePowerOn_exit_:
	return -EIO;

}
EXPORT_SYMBOL(kdCISModulePowerOn);


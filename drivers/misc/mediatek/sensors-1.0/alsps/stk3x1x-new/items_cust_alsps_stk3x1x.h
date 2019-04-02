/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
#ifndef __CUST_ALSPS_H__
#define __CUST_ALSPS_H__

#include <linux/types.h>
#include <mach/mt_pm_ldo.h>
#include <mach/items.h>

#define C_CUST_ALS_LEVEL    16
#define C_CUST_I2C_ADDR_NUM 1

struct alsps_hw {
    int i2c_num;                                    /*!< the i2c bus used by ALS/PS */
    int power_id;                                   /*!< the power id of the chip */
    int power_vol;                                  /*!< the power voltage of the chip */
	//int polling_mode;                               /*!< 1: polling mode ; 0:interrupt mode*/
	int polling_mode_ps;                               /*!< 1: polling mode ; 0:interrupt mode*/
	int polling_mode_als;                               /*!< 1: polling mode ; 0:interrupt mode*/
    unsigned char   i2c_addr[C_CUST_I2C_ADDR_NUM];  /*!< i2c address list, some chip will have multiple address */
    unsigned int    als_level[C_CUST_ALS_LEVEL-1];  /*!< (C_CUST_ALS_LEVEL-1) levels divides all range into C_CUST_ALS_LEVEL levels*/
    unsigned int    als_value[C_CUST_ALS_LEVEL];    /*!< the value reported in each level */
    //unsigned int    ps_threshold;                   /*!< the threshold of proximity sensor */	
	unsigned int	state_val;
	unsigned int 	psctrl_val;
	unsigned int 	alsctrl_val;
	unsigned int 	ledctrl_val;
	unsigned int 	wait_val;	
    unsigned int    ps_high_thd_val;
    unsigned int    ps_low_thd_val;
	unsigned int    als_window_loss;                /*!< the window loss  */
};

static struct alsps_hw items_cust_alsps_hw = {
	.i2c_num    = 2,	
	//.polling_mode =1,
	.polling_mode_ps =0,
	.polling_mode_als =1,
    .power_id   = MT65XX_POWER_NONE,    /*LDO is not used*/
    .power_vol  = VOL_DEFAULT,          /*LDO is not used*/
    .i2c_addr   = {0x90, 0x00, 0x00, 0x00},	/*STK3x1x*/
    .als_level  = {5,  9, 36, 59, 82, 132, 205, 273, 500, 845, 1136, 1545, 2364, 4655, 6982},	/* als_code */
	  //.als_level  = {6, 12, 56, 78, 108, 174, 270, 360, 660, 1116, 1500, 2040, 3120, 6144, 9216},	/* als_code */
    .als_value  = {0, 10, 40, 65, 90, 145, 225, 300, 550, 930, 1250, 1700, 2600, 5120, 7680, 10240},    /* lux */
   	.state_val = 0x0,		/* disable all */
	.psctrl_val = 0x32,		/* ps_persistance=4, ps_gain=64X, PS_IT=0.391ms */
	.alsctrl_val = 0x3A,	/* als_persistance=1, als_gain=64X, ALS_IT=50ms */
	.ledctrl_val = 0xFF,	/* 100mA IRDR, 64/64 LED duty */
	.wait_val = 0x7,		/* 50 ms */
    .ps_high_thd_val = 1700,
    .ps_low_thd_val = 1500,
};
struct alsps_hw *items_stx3x1x_get_cust_alsps_hw(void) {
	int orient,i2c_num;
	i2c_num	= item_integer("alsps.i2c",0);
	printk("items_stx3x1x_get_cust_alsps_hw i2c_num =%d\n",i2c_num);
	if(i2c_num >= 0);
		items_cust_alsps_hw.i2c_num = i2c_num;
	
    return &items_cust_alsps_hw;
}
#endif 
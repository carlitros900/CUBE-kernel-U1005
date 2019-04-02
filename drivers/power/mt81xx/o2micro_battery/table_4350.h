/*****************************************************************************
* Copyright(c) O2Micro, 2014. All rights reserved.
*
* O2Micro battery gauge driver
* File: table.h
*
* $Source: /data/code/CVS
* $Revision: 4.00.01 $
*
* This program is free software and can be edistributed and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* This Source Code Reference Design for O2MICRO Battery Gauge access (\u201cReference Design\u201d) 
* is sole for the use of PRODUCT INTEGRATION REFERENCE ONLY, and contains confidential 
* and privileged information of O2Micro International Limited. O2Micro shall have no 
* liability to any PARTY FOR THE RELIABILITY, SERVICEABILITY FOR THE RESULT OF PRODUCT 
* INTEGRATION, or results from: (i) any modification or attempted modification of the 
* Reference Design by any party, or (ii) the combination, operation or use of the 
* Reference Design with non-O2Micro Reference Design.
*
* Battery Manufacture: NOVEO
* Battery Model: U1005
* Absolute Max Capacity(mAhr): 4326
* Limited Charge Voltage(mV): 4350
* Cutoff Discharge Voltage(mV): 3200
* Equipment: "JY1-1","JY1-1","JY1-1","JY1-3","JY1-3","JY1-8","JY1-8","JY1-8","JY1-1","JY1-1","JY1-1","JY1-1","JY1-1","JY1-1","JY1-1","JY1-1","JY1-3","JY1-3","JY1-3","JY1-3","JY1-3","JY1-5","JY1-3","JY1-3","JY2-1","JY2-1","JY2-1","JY2-1","JY2-1","JY2-1","JY2-1","JY2-7","JY2-7","JY2-7","JY2-7","JY2-7","JY2-7","JY2-7","JY1-6","JY1-6","JY1-6","JY1-6","JY2-7","JY2-7","JY2-7"
* Tester: "SZ","SZ","SZ","SZ","SZ","SZ","SZ","SZ","SZ","SZ","SZ","SZ","SZ","SZ","SZ","SZ","SZ","SZ","SZ","SZ","SZ","SZ","SZ","SZ","SZ","SZ","SZ","SZ","SZ","SZ","SZ","SZ","SZ","SZ","SZ","SZ","SZ","SZ","SZ","SZ","SZ","SZ","SZ","SZ"
* Battery ID: "U1005_1","U1005_1","U1005_1","U1005_1","U1005_1","U1005_1","U1005_1","U1005_1","U1005_2","U1005_2","U1005_2","U1005_2","U1005_2","U1005_2","U1005_2","U1005_2","U1005_3","U1005_3","U1005_3","U1005_3","U1005_3","U1005_3","U1005_3","U1005_3","U1005_3","U1005_3","U1005_3","U1005_3","U1005_3","U1005_3","U1005_3","U1005_3","U1005_3","U1005_3","U1005_3","U1005_3","U1005_3","U1005_3","U1005_3","U1005_3","U1005_3","U1005_3","U1005_3","U1005_3","U1005_3"
* Version = V00.9F.00
* Date = 2018.6.6
* Comment = 
*           
*****************************************************************************/

#ifndef _TABLE_H_
#define _TABLE_H_


#define OCV_DATA_NUM 	 65
#define CHARGE_DATA_NUM 		 65


#define XAxis 		 77
#define YAxis 		 7
#define ZAxis 		 6

#define BATTERY_ID_NUM 		 45


/****************************************************************************
* Struct section
*  add struct #define here if any
***************************************************************************/
typedef struct tag_one_latitude_data {
 	 int32_t 			 x;//
 	 int32_t 			 y;//
} one_latitude_data_t;


/****************************************************************************
* extern variable declaration section
***************************************************************************/
extern const char *battery_id[] ;

extern const char *table_version;

#endif


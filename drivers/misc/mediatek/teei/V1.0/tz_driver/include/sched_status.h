/*
 * Copyright (c) 2015-2017 MICROTRUST Incorporated
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef SCHED_STATUS_H
#define SCHED_STATUS_H

#define GLSCH_NONE				(0x00)
#define GLSCH_LOW				(0x01)
#define GLSCH_HIGH				(0x02)
#define GLSCH_NEG				(0x03)
#define GLSCH_FOR_SOTER			(0x04)

extern struct mutex pm_mutex;
extern unsigned long ut_pm_count;
extern unsigned int need_mig_flag;
extern unsigned int nt_core;

#endif /* end of SCHED_STATUS_H */

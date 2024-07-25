/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-07-25     xym-ee       the first version
 */

#ifndef __SENSOR_MCP9808_H__
#define __SENSOR_MCP9808_H__

#include <rtthread.h>
//#include <rtdevice.h>

#include "sensor.h"



struct mcp9808_device {
    struct rt_i2c_bus_device * i2c_bus;
    rt_uint8_t addr;

};

typedef struct mcp9808_device * mcp9808_device_t;


int rt_hw_mcp9808_init(const char *name, struct rt_sensor_config *cfg);

#endif

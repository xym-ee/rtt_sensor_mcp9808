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
#include <rtdevice.h>

#include <drivers/sensor_v2.h>



#define MCP9808_REG_SENSOR_CONFIG       0x01
#define MCP9808_REG_LIMIT_UPPER         0x02
#define MCP9808_REG_LIMIT_LOWER         0x03
#define MCP9808_REG_LIMIT_CRIT          0x04
#define MCP9808_REG_TEMP_REG            0x05
#define MCP9808_REG_MANUFACTUR_ID       0x06
#define MCP9808_REG_DEV_ID_VERSION      0x07
#define MCP9808_REG_RESOLUTION          0X08





struct mcp9808_device {
    struct rt_i2c_bus_device * i2c_bus;
    rt_uint8_t addr;

};

typedef struct mcp9808_device * mcp9808_device_t;


int rt_hw_mcp9808_init(const char *name, struct rt_sensor_config *cfg);

#endif

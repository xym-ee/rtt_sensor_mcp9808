/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-07-25     xym-ee       the first version
 */
 
#include "sensor_mcp9808.h"


#define DBG_TAG "mcp9808"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>


static void seneor_mcp9808_sample_entry(void *parameter)
{
    rt_device_t mcp9808_sensor;
    rt_sensor_data_t mcp9808_data;
    rt_size_t ret;

    mcp9808_sensor = rt_device_find(parameter);
    
    if (mcp9808_sensor == RT_NULL) {
        LOG_E("Can't find device:%s\n", parameter);
        return;
    }

    if (rt_device_open(mcp9808_sensor, RT_DEVICE_FLAG_RDWR) != RT_EOK) {
        LOG_E("open device failed!\n");
        return;
    }
    
    mcp9808_data = rt_calloc(1, sizeof(struct rt_sensor_data));

    while (1) {
        ret = rt_device_read(mcp9808_sensor, 0, mcp9808_data, 1);
        if (ret != 1) {
            LOG_E("read data failed!size is %d\n", ret);
            rt_device_close(mcp9808_sensor);
            return;
        }
        else {
            LOG_I("Temperature: %d.%d timestamp:%5d", (int)mcp9808_data->data.temp, (int)(mcp9808_data->data.temp * 1000) % 1000, mcp9808_data->timestamp);
        }
        rt_thread_mdelay(2000);
    }
}


int mcp9808_samplle(void)
{
    rt_thread_t mcp9808_thread;

    mcp9808_thread = rt_thread_create("mcp",
                                   seneor_mcp9808_sample_entry,
                                   "tm-mcp",
                                   1024,
                                   RT_THREAD_PRIORITY_MAX / 2,
                                   20);
    if (mcp9808_thread != RT_NULL) {
        rt_thread_startup(mcp9808_thread);
    }

    return RT_EOK;
}
INIT_APP_EXPORT(mcp9808_samplle);


#define MCP9808_IIC_DEV_NAME    "i2c1"
#define MCP9808_IIC_DEV_ADDR    0x18

int rt_hw_mcp9808_port(void)
{
    struct rt_sensor_config cfg;
    
    cfg.intf.dev_name = MCP9808_IIC_DEV_NAME;
    cfg.intf.arg      = (void *)MCP9808_IIC_DEV_ADDR;
    
    rt_hw_mcp9808_init("mcp", &cfg);
    
    LOG_I("MCP9808(sensor_v2) has been initialized!");
    LOG_I("Temperature mount cmd:        [sensor probe tm-9808]");
    LOG_I("Temperature read 5 times cmd: [sensor read 5]");    
    
    return RT_EOK;
}
INIT_COMPONENT_EXPORT(rt_hw_mcp9808_port);





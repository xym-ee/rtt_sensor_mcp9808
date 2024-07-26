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


#define DBG_TAG     "MCP9808"
#define DBG_LVL     DBG_LOG
#include <rtdbg.h>



static void seneor_mcp9808_sample_entry(void *parameter)
{
    rt_device_t dev = RT_NULL;
    
    rt_sensor_t mcp9808_senrot;
    rt_sensor_data_t mcp9808_data;
    
    struct rt_sensor_data sensor_data;
    rt_size_t res;

    dev = rt_device_find(parameter);
    if (dev == RT_NULL) {
        rt_kprintf("Can't find device:%s\n", parameter);
        return;
    }

    if (rt_device_open(dev, RT_DEVICE_FLAG_RDWR) != RT_EOK) {
        rt_kprintf("open device failed!\n");
        return;
    }

    while (1) {
        res = rt_device_read(dev, 0, &sensor_data, 1);
        if (res != 1) {
            rt_kprintf("read data failed!size is %d\n", res);
            rt_device_close(dev);
            return;
        }
        else {
            rt_kprintf("temp:%6d , timestamp:%5d\n", (rt_int32_t)(sensor_data.data.temp*1000),  sensor_data.timestamp);
        }
        rt_thread_mdelay(2000);
    }
}


int mcp9808_samplle(void)
{
    rt_thread_t mcp9808_thread;

    mcp9808_thread = rt_thread_create("mcp",
                                   seneor_mcp9808_sample_entry,
                                   "tm-9808",
                                   1024,
                                   RT_THREAD_PRIORITY_MAX / 2,
                                   20);
    if (mcp9808_thread != RT_NULL) {
        rt_thread_startup(mcp9808_thread);
    }

    return RT_EOK;
}
INIT_APP_EXPORT(mcp9808_samplle);


#define MCP9808_IIC_BUS_NAME    "i2c1"
#define MCP9808_IIC_ADDR        0x18


int rt_hw_mcp9808_port(void)
{
    struct rt_sensor_config cfg;
    
    cfg.intf.dev_name = MCP9808_IIC_BUS_NAME;
    cfg.intf.arg      = (void *)MCP9808_IIC_ADDR;
    
    rt_hw_mcp9808_init("9808", &cfg);
    
    LOG_I("MCP9808(sensor_v2) has been initialized!");
    LOG_I("Temperature mount cmd:        [sensor probe tm-9808]");
    LOG_I("Temperature read 5 times cmd: [sensor read 5]");    
    
    return RT_EOK;
}
INIT_COMPONENT_EXPORT(rt_hw_mcp9808_port);





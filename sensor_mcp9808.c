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

#define DBG_TAG     "mcp9808"
#define DBG_LVL     DBG_INFO
#include <rtdbg.h>


mcp9808_device_t mcp9808_dev = RT_NULL;






static rt_size_t mcp9808_fetch_data(struct rt_sensor_device *sensor, void *buf, rt_size_t size)
{
    struct rt_sensor_data* data = buf;
    
    if (size < 1)
    {
        LOG_E("%s:read size err! size=%d", __func__, size);
        return 0;
    }

    if (buf == RT_NULL)
    {
        LOG_E("%s:read buf is NULL!", __func__);
        return 0;
    }

    for (int i = 0; i < size; i++)
    {
        float temperature = 0.0; 
        
        rt_uint8_t recv_buf[2] = {0x00, 0x00};
        
        rt_uint8_t send_buf[1] = {MCP9808_REG_TEMP_REG};      

        rt_i2c_master_send(mcp9808_dev->i2c_bus, mcp9808_dev->addr, RT_I2C_WR, send_buf, 1);

        rt_i2c_master_recv(mcp9808_dev->i2c_bus, mcp9808_dev->addr, RT_I2C_RD, recv_buf, 2);
        
        
        /* 高字节高位3bit为标志位 */
        recv_buf[0] = recv_buf[0] & 0x1F;       //Clear flag bits
        
        /* 高字节 bit4 为符号位 */
        if ((recv_buf[0] & 0x10) == 0x10)         //TA < 0°C
        { 
            recv_buf[0] = recv_buf[0] & 0x0F;   //Clear SIGN
            temperature = 256.0 - (recv_buf[0] * 16.0 + recv_buf[1] / 16.0);
        }
        else    //TA ³ 0°C
        {
            temperature = (recv_buf[0] * 16.0 + recv_buf[1] / 16.0);
        }
        
        data->type = RT_SENSOR_CLASS_TEMP;
        data->data.temp = temperature*10;
        data->timestamp = rt_sensor_get_ts();
        LOG_D("%s:%d", __func__, data->data.temp);
        data++;
    }

    return size;
}


static rt_err_t mcp9808_control(struct rt_sensor_device* sensor, int cmd, void* args)
{
    rt_err_t result = RT_EOK;

    switch (cmd)
    {
    case RT_SENSOR_CTRL_GET_ID:
        *(rt_uint8_t*)args = sens_temp_tbl[SENS_TEMP_01].sens_id;
        break;
    case RT_SENSOR_CTRL_SET_ODR:
        result = temp_sensor_set_odr(sensor, (rt_uint32_t)args & 0xffff);
        break;
    case RT_SENSOR_CTRL_SET_RANGE:
        result = temp_sensor_set_range(sensor, (rt_uint32_t)args);
        break;
    case RT_SENSOR_CTRL_SET_POWER:
        result = temp_sensor_set_power(sensor, (rt_uint32_t)args & 0xff);
        break;
    case RT_SENSOR_CTRL_SELF_TEST:
        /* TODO */
        result = -RT_EINVAL;
        break;
    default:
        return -RT_EINVAL;
    }
    return result;
}




static struct rt_sensor_ops sensor_ops =
{
    mcp9808_fetch_data,
    mcp9808_control,
};



static mcp9808_device_t _mcp9808_init(struct rt_sensor_config *cfg)
{
    mcp9808_device_t dev;

    dev = rt_calloc(1, sizeof(struct mcp9808_device));
    if (dev == RT_NULL) {
        LOG_E("Can't allocate memory for sr04 device on '%s'.\n", cfg->intf.dev_name);
        return RT_NULL;
    }
    
    dev->i2c_bus = (struct rt_i2c_bus_device *)rt_device_find(cfg->intf.dev_name);

    if (dev->i2c_bus == RT_NULL)
    {
        LOG_E("can't find %s device!", cfg->intf.dev_name);
    }
    
    dev->addr = (rt_uint32_t)cfg->intf.user_data;
    
    return dev;
}


int rt_hw_mcp9808_init(const char *name, struct rt_sensor_config *cfg)
{
    rt_int8_t result;
    rt_sensor_t sensor_mcp9808 = RT_NULL;

    
    mcp9808_dev = _mcp9808_init(cfg);

    /* sr04 sensor register */
    sensor_mcp9808 = rt_calloc(1, sizeof(struct rt_sensor_device));
    if (sensor_mcp9808 == RT_NULL) {
        return -1;
    }

    sensor_mcp9808->info.type       = RT_SENSOR_CLASS_TEMP;
    sensor_mcp9808->info.vendor     = RT_SENSOR_VENDOR_UNKNOWN;
    sensor_mcp9808->info.model      = "mcp";
    sensor_mcp9808->info.unit       = RT_SENSOR_UNIT_DCELSIUS;
    sensor_mcp9808->info.intf_type  = RT_SENSOR_INTF_I2C;
    sensor_mcp9808->info.range_max  = 12500;
    sensor_mcp9808->info.range_min  = -4000;
    sensor_mcp9808->info.period_min = 100;

    rt_memcpy(&sensor_mcp9808->config, cfg, sizeof(struct rt_sensor_config));
    sensor_mcp9808->ops = &sensor_ops;

    result = rt_hw_sensor_register(sensor_mcp9808, name, RT_DEVICE_FLAG_RDONLY, RT_NULL);
    if (result != RT_EOK) {
        LOG_E("device register err code: %d", result);
        goto __exit;
    }

    return RT_EOK;

__exit:
    if (sensor_mcp9808)
        rt_free(sensor_mcp9808);
    return -RT_ERROR;
}











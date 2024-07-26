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




/**
 * This function initializes aht10 registered device driver
 *
 * @param dev the name of aht10 device
 *
 * @return the aht10 device.
 */
static mcp9808_device_t _mcp9808_init(struct rt_sensor_config *cfg)
{
    mcp9808_device_t dev;

    dev = rt_calloc(1, sizeof(struct mcp9808_device));
    if (dev == RT_NULL) {
        LOG_E("Can't allocate memory for sr04 device on '%s'.\n", cfg->intf.dev_name);
        return RT_NULL;
    }
    
    dev->i2c = (struct rt_i2c_bus_device *)rt_device_find(cfg->intf.dev_name);

    if (dev->i2c == RT_NULL)
    {
        LOG_E("can't find %s device!", cfg->intf.dev_name);
    }
    
    dev->addr = (rt_uint32_t)cfg->intf.arg;
    
    return dev;
}




static rt_ssize_t _mcp9808_polling_get_data(rt_sensor_t sensor, rt_sensor_data_t data)
{
    //struct aht10_device *aht10_dev = (struct aht10_device *)sensor->parent.user_data;

    if (sensor->info.type == RT_SENSOR_TYPE_TEMP)
    {
        //data->data.temp = aht10_read_temperature(aht10_dev);
        data->timestamp = rt_sensor_get_ts();
        return 1;
    }
    else if (sensor->info.type == RT_SENSOR_TYPE_HUMI)
    {
        //data->data.humi = aht10_read_humidity(aht10_dev);
        data->timestamp = rt_sensor_get_ts();
        return 1;
    }
    else
    {
        return -RT_EINVAL;
    }
}


static rt_ssize_t mcp9808_fetch_data(rt_sensor_t sensor, rt_sensor_data_t buf, rt_size_t len)
{
    rt_sensor_data_t data = buf;
    
    mcp9808_device_t mcp9808 = sensor->parent.user_data;
    
    if (len < 1)
    {
        LOG_E("%s:read size err! size=%d", __func__, len);
        return 0;
    }

    if (buf == RT_NULL)
    {
        LOG_E("%s:read buf is NULL!", __func__);
        return 0;
    }

    for (int i = 0; i < len; i++)
    {
        float temperature = 0.0; 
        
        rt_uint8_t recv_buf[2] = {0x00, 0x00};
        
        rt_uint8_t send_buf[1] = {MCP9808_REG_TEMP_REG};

        rt_i2c_master_send(mcp9808->i2c, mcp9808->addr, RT_I2C_WR, send_buf, 1);

        rt_i2c_master_recv(mcp9808->i2c, mcp9808->addr, RT_I2C_RD, recv_buf, 2);
    
        
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
        
        data->type = RT_SENSOR_TYPE_TEMP;
        data->data.temp = temperature;
        data->timestamp = rt_sensor_get_ts();
        LOG_D("%s:%d", __func__, data->data.temp);
        data++;
    }

    return len;
}


static rt_err_t mcp9808_control(struct rt_sensor_device* sensor, int cmd, void* args)
{
    rt_err_t result = RT_EOK;


    return result;
}




static const struct rt_sensor_ops sensor_ops =
{
    mcp9808_fetch_data,
    mcp9808_control,
};






/*
    负责找 iic 设备嘛？



*/
int rt_hw_mcp9808_init(const char *name, struct rt_sensor_config *cfg)
{
    rt_int8_t result;
    rt_sensor_t sensor_mcp9808 = RT_NULL;

    /* sr04 sensor register */
    sensor_mcp9808 = rt_calloc(1, sizeof(struct rt_sensor_device));
    if (sensor_mcp9808 == RT_NULL) {
        return -1;
    }
    sensor_mcp9808->parent.user_data = (void*)_mcp9808_init(cfg);

    sensor_mcp9808->info.type   = RT_SENSOR_TYPE_TEMP;
    sensor_mcp9808->info.vendor = RT_SENSOR_VENDOR_UNKNOWN;
    sensor_mcp9808->info.name   = "mcp9808";
    sensor_mcp9808->info.unit   = RT_SENSOR_UNIT_CELSIUS;

    RT_SENSOR_MODE_SET_ACCURACY(sensor_mcp9808->info.mode, RT_SENSOR_MODE_ACCURACY_MEDIUM);
    RT_SENSOR_MODE_SET_POWER(sensor_mcp9808->info.mode, RT_SENSOR_MODE_POWER_MEDIUM);
    RT_SENSOR_MODE_SET_FETCH(sensor_mcp9808->info.mode, RT_SENSOR_MODE_FETCH_POLLING);
    
    sensor_mcp9808->info.intf_type   = RT_SENSOR_INTF_I2C;
    sensor_mcp9808->info.acquire_min = 100;

    sensor_mcp9808->info.accuracy.resolution = 0.0625;
    sensor_mcp9808->info.accuracy.error      = 0.5;
    
    sensor_mcp9808->info.scale.range_max = 125.0;
    sensor_mcp9808->info.scale.range_min = -40.0;
    

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











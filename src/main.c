#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/sys/printk.h>

#include "database.hpp"

#define BME280_NODE DT_NODELABEL(bme280)

int main(void)
{
    const struct device *bme280 = DEVICE_DT_GET(BME280_NODE);

    if (!device_is_ready(bme280)) {
        printk("PulseCore: BME280 device not ready\n");
        return 0;
    }

    printk("PulseCore: BME280 initialized\n");

    while (1) {
        struct sensor_value temperature;
        struct sensor_value humidity;
        struct sensor_value pressure;

        int ret = sensor_sample_fetch(bme280);

        if (ret == 0) {
            sensor_channel_get(bme280, SENSOR_CHAN_AMBIENT_TEMP,
                               &temperature);
            sensor_channel_get(bme280, SENSOR_CHAN_HUMIDITY,
                               &humidity);
            sensor_channel_get(bme280, SENSOR_CHAN_PRESS,
                               &pressure);

        printk("TEMP:%d.%06d,HUM:%d.%06d,PRESS:%d.%06d\n",
            temperature.val1, temperature.val2,
            humidity.val1, humidity.val2,
            pressure.val1, pressure.val2);
            
        } else {
            printk("PulseCore: Sensor read failed (%d)\n", ret);
        }

        k_sleep(K_SECONDS(2));
    }

    return 0;
}

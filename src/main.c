#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/sys/printk.h>

#define BME280_NODE DT_NODELABEL(bme280)

struct sensor_data {
    struct sensor_value temperature;
    struct sensor_value humidity;
    struct sensor_value pressure;
};

K_MSGQ_DEFINE(sensor_msgq, sizeof(struct sensor_data), 10, 4);

void sensor_thread(void)
{
    const struct device *bme280 = DEVICE_DT_GET(BME280_NODE);

    while (1) {
        struct sensor_data data;

        int ret = sensor_sample_fetch(bme280);

        if (ret == 0) {
            sensor_channel_get(
                bme280,
                SENSOR_CHAN_AMBIENT_TEMP,
                &data.temperature
            );

            sensor_channel_get(
                bme280,
                SENSOR_CHAN_HUMIDITY,
                &data.humidity
            );

            sensor_channel_get(
                bme280,
                SENSOR_CHAN_PRESS,
                &data.pressure
            );

            k_msgq_put(&sensor_msgq, &data, K_FOREVER);

        } else {
            printk("PulseCore: Sensor read failed (%d)\n", ret);
        }

        k_sleep(K_SECONDS(2));
    }
}

void telemetry_thread(void)
{
    struct sensor_data data;

    while (1) {
        k_msgq_get(&sensor_msgq, &data, K_FOREVER);

        printk("TEMP:%d.%06d,HUM:%d.%06d,PRESS:%d.%06d\n",
               data.temperature.val1, data.temperature.val2,
               data.humidity.val1, data.humidity.val2,
               data.pressure.val1, data.pressure.val2);
    }
}

#define SENSOR_THREAD_STACK_SIZE 2048
#define TELEMETRY_THREAD_STACK_SIZE 2048

#define SENSOR_THREAD_PRIORITY 5
#define TELEMETRY_THREAD_PRIORITY 5

K_THREAD_DEFINE(
    sensor_thread_id,
    SENSOR_THREAD_STACK_SIZE,
    sensor_thread,
    NULL, NULL, NULL,
    SENSOR_THREAD_PRIORITY,
    0,
    1
);

K_THREAD_DEFINE(
    telemetry_thread_id,
    TELEMETRY_THREAD_STACK_SIZE,
    telemetry_thread,
    NULL, NULL, NULL,
    TELEMETRY_THREAD_PRIORITY,
    0,
    1
);

int main(void)
{
    const struct device *bme280 = DEVICE_DT_GET(BME280_NODE);

    if (!device_is_ready(bme280)) {
        printk("PulseCore: BME280 device not ready\n");
        return 0;
    }

    printk("PulseCore: BME280 initialized\n");

    k_thread_start(sensor_thread_id);
    k_thread_start(telemetry_thread_id);

    printk("PulseCore: RTOS threads started\n");

    return 0;
}
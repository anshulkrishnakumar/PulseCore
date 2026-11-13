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
static uint32_t samples_produced = 0;
static uint32_t samples_transmitted = 0;

K_MUTEX_DEFINE(stats_mutex);

void sensor_thread(void)
{
    const struct device *bme280 = DEVICE_DT_GET(BME280_NODE);
    uint32_t consecutive_failures = 0;

    while (1) {
        struct sensor_data data;

        int ret = sensor_sample_fetch(bme280);

        if (ret != 0) {
            consecutive_failures++;

            printk("PulseCore: Sensor read failed (%d), consecutive failures: %u\n",
                ret, consecutive_failures);

            if (consecutive_failures == 3) {
                printk("PulseCore ERROR: Sensor communication problem\n");
            }

            k_sleep(K_SECONDS(2));
            continue;
        }

        int channel_ret;

        channel_ret = sensor_channel_get(
            bme280,
            SENSOR_CHAN_AMBIENT_TEMP,
            &data.temperature
        );

        if (channel_ret != 0) {
            consecutive_failures++;
            printk("PulseCore: Temperature read failed (%d)\n", channel_ret);
            k_sleep(K_SECONDS(2));
            continue;
        }

        channel_ret = sensor_channel_get(
            bme280,
            SENSOR_CHAN_HUMIDITY,
            &data.humidity
        );

        if (channel_ret != 0) {
            consecutive_failures++;
            printk("PulseCore: Humidity read failed (%d)\n", channel_ret);
            k_sleep(K_SECONDS(2));
            continue;
        }

        channel_ret = sensor_channel_get(
            bme280,
            SENSOR_CHAN_PRESS,
            &data.pressure
        );

        if (channel_ret != 0) {
            consecutive_failures++;
            printk("PulseCore: Pressure read failed (%d)\n", channel_ret);
            k_sleep(K_SECONDS(2));
            continue;
        }

        // All sensor readings succeeded.
        consecutive_failures = 0;

        int queue_ret = k_msgq_put(&sensor_msgq, &data, K_FOREVER);

        if (queue_ret != 0) {
            printk("PulseCore: Message queue insertion failed (%d)\n",
                   queue_ret);
        }

        if (queue_ret == 0) {
            k_mutex_lock(&stats_mutex, K_FOREVER);
            samples_produced++;
            k_mutex_unlock(&stats_mutex);
        }

        k_sleep(K_SECONDS(2));
    }
}


void telemetry_thread(void)
{
    struct sensor_data data;

    while (1) {
        int queue_ret = k_msgq_get(&sensor_msgq, &data, K_FOREVER);

        if (queue_ret != 0) {
            printk("PulseCore: Message queue retrieval failed (%d)\n",
                queue_ret);
            continue;
        }

        k_mutex_lock(&stats_mutex, K_FOREVER);
        samples_transmitted++;
        k_mutex_unlock(&stats_mutex);

        printk("TEMP:%d.%06d,HUM:%d.%06d,PRESS:%d.%06d\n",
               data.temperature.val1, data.temperature.val2,
               data.humidity.val1, data.humidity.val2,
               data.pressure.val1, data.pressure.val2);
    }
}

void monitoring_thread(void)
{
    while (1) {
        k_sleep(K_SECONDS(10));

        uint32_t produced;
        uint32_t transmitted;

        k_mutex_lock(&stats_mutex, K_FOREVER);

        produced = samples_produced;
        transmitted = samples_transmitted;

        k_mutex_unlock(&stats_mutex);

        printk(
            "PulseCore Stats: Produced=%u, Transmitted=%u\n",
            produced,
            transmitted
        );
    }
}

#define SENSOR_THREAD_STACK_SIZE 2048
#define TELEMETRY_THREAD_STACK_SIZE 2048

#define SENSOR_THREAD_PRIORITY 5
#define TELEMETRY_THREAD_PRIORITY 5

#define MONITORING_THREAD_STACK_SIZE 2048
#define MONITORING_THREAD_PRIORITY 5

K_THREAD_DEFINE(
    monitoring_thread_id,
    MONITORING_THREAD_STACK_SIZE,
    monitoring_thread,
    NULL, NULL, NULL,
    MONITORING_THREAD_PRIORITY,
    0,
    1
);

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
    k_thread_start(monitoring_thread_id);

    printk("PulseCore: RTOS threads started\n");

    return 0;
}
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

int main(void)
{
    printk("PulseCore: Firmware started\n");

    while (1) {
        printk("PulseCore: System is running\n");
        k_sleep(K_SECONDS(2));
    }

    return 0;
}

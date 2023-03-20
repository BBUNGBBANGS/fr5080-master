#ifndef FR8000_BURNN_H
#define FR8000_BURNN_H

#define HD_ONKEY_ON()       (pmu_set_gpio_value(GPIO_PORT_D, (1<<GPIO_BIT_1), 0))
#define HD_ONKEY_OFF()      (pmu_set_gpio_value(GPIO_PORT_D, (1<<GPIO_BIT_1), 1))
#define RESET_EN()          (pmu_set_gpio_value(GPIO_PORT_D, (1<<GPIO_BIT_2), 0))
#define RESET_RELEASE()     (pmu_set_gpio_value(GPIO_PORT_D, (1<<GPIO_BIT_2), 1))

void fr8000_reset(void);
void fr8000_program(void);

#endif

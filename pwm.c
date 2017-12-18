/* Very basic example to test the pwm library
 * Hook up an LED to pin14 and you should see the intensity change
 *
 * Part of esp-open-rtos
 * Copyright (C) 2015 Javier Cardona (https://github.com/jcard0na),
 * BSD Licensed as described in the file LICENSE
 */
#include "espressif/esp_common.h"
#include "esp/uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "pwm.h"

#define MAX 7710

uint8_t pins[2];

void forward()
{
    pwm_init(2, pins, true);
    gpio_write(0,1);
    gpio_write(2,1);
    vTaskDelay(200);
}

void backward()
{
    pwm_init(2, pins, true);
    gpio_write(0,1);
    gpio_write(2,1);
    vTaskDelay(200);
}

void left()
{
    pwm_init(2, pins, true);
    gpio_write(0,1);
    gpio_write(2,0);
    vTaskDelay(200);
}

void right()
{
    pwm_init(2, pins, true);
    gpio_write(0,0);
    gpio_write(2,1);
    vTaskDelay(200);
}

void stop()
{
    gpio_write(5,0);
    gpio_write(4,0);
    vTaskDelay(200);
}


void task1(void *pvParameters)
{
    printf("Hello from task1!\r\n");
   gpio_enable(0,GPIO_OUTPUT);    
   gpio_enable(2,GPIO_OUTPUT);    

    while(1) {
        vTaskDelay(100);
	pwm_set_freq(1000);
        pwm_set_duty(MAX);


	forward();
	stop();
/////////////////////////////	
	right();
	stop();
///////////////////////////
	backward();	
	stop();
////////////////////////////
	left();
	stop();


    }
}

void user_init(void)
{
    
    uart_set_baud(0, 115200);

    printf("SDK version:%s\n", sdk_system_get_sdk_version());

    printf("pwm_init(1, [14])\n");
    pins[0] = 5; //A- 오른쪽
    //pins[0] = 0; //A+ 
    pins[1] = 4; //B- 왼쪽
    //pins[1] = 2; //B+ 

    pwm_init(2, pins, true);

    xTaskCreate(task1, "tsk1", 256, NULL, 2, NULL);
}

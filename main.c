/*
 * Example of using ultrasonic rnaghe meter like HC-SR04
 *
 * Part of esp-open-rtos
 * Copyright (C) 2016 Ruslan V. Uss <unclerus@gmail.com>
 * BSD Licensed as described in the file LICENSE
 */

/*
 * Softuart example
 *
 * Copyright (C) 2017 Ruslan V. Uss <unclerus@gmail.com>
 * Copyright (C) 2016 Bernhard Guillon <Bernhard.Guillon@web.de>
 * Copyright (c) 2015 plieningerweb
 *
 * MIT Licensed as described in the file LICENSE
 */

/* Implementation of PWM support for the Espressif SDK.
 *
 * Part of esp-open-rtos
 * Copyright (C) 2015 Guillem Pascual Ginovart (https://github.com/gpascualg)
 * Copyright (C) 2015 Javier Cardona (https://github.com/jcard0na)
 * BSD Licensed as described in the file LICENSE
 */

#include <stdio.h>
#include <stdlib.h>
#include "espressif/esp_common.h"
#include "esp/uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "esp8266.h"
#include <esp/gpio.h>
#include "ultrasonic/ultrasonic.h"
#include "softuart/softuart.h"
#include "pwm.h"

#define RX_PIN 3 //D9
#define TX_PIN 1 //D10

#define TRIGGER_PIN 14	//D5
#define ECHO_PIN    12	//D6

#define MAX_DISTANCE_CM 500 // 5m max
#define MAX_PWM_DUTY 13   // %

#define A_PLUS 0	// A+ D3
#define A_MINUS 5	// A- D1
#define B_PLUS 2	// B+ D2
#define B_MINUS 4	// B- D4

int32_t distance = 500;
char key = 'p';
char string_out[10];

void bluetooth(void *pvParameters)
{
    QueueHandle_t *queue = (QueueHandle_t *)pvParameters;
    // Baudrate 9600
    softuart_open(0, 9600, RX_PIN, TX_PIN);
    uint32_t count = 0;
    while (true)
    {
        if (!softuart_available(0))
            continue;
        key = softuart_read(0);
        vTaskDelay(20);
        xQueueSend(*queue, &count, 0);
        count++;
    }
}

void ultrasonic(void *pvParameters)
{
    QueueHandle_t *queue = (QueueHandle_t *)pvParameters;
    //Trigger = GPIO14 = D5 // ECHO_PIN = 12 = D6
    ultrasonic_sensor_t sensor = {
        .trigger_pin = TRIGGER_PIN,
        .echo_pin = ECHO_PIN
    };
    //GPIO_Enable
    ultrasoinc_init(&sensor);

    uint32_t count = 0;
    while(1) {
	//  l = (v * t) / 2
	uint32_t distance_temp = ultrasoinc_measure_cm(&sensor, MAX_DISTANCE_CM);
	//receive error(no signal or farther than MAX_DISTANCE)        
	if (distance_temp < 0)
	    break;
        else
	  distance = distance_temp;
        vTaskDelay(20);
        xQueueSend(*queue, &count, 0);
        count++;
    }
}

void motorcontrol(void *pvParameters)
{
    QueueHandle_t *queue = (QueueHandle_t *)pvParameters;
    uint32_t count = 0;
    uint8_t pins[2];
    pins[0] = A_MINUS;	// A-
    pins[1] = B_MINUS;	// B-
    //PWM_FREQ
    pwm_set_freq(1000);
    //MAX POWER = (MAX_PWM_DUTY)%
    pwm_set_duty(UINT16_MAX * MAX_PWM_DUTY/(double)100);
    pwm_start();
    //GPIO_ENABLE
    gpio_enable(A_PLUS, GPIO_OUTPUT);
    gpio_enable(B_PLUS, GPIO_OUTPUT);
    
    while(1) {
	  //stops if too close
	if(distance < 30)
	  if (!((key=='p')||(key=='s'))) key = 'p';
	switch(key){
	  //forward
	  case 'w':
	    pwm_init(2,pins,true);
	    gpio_write(A_PLUS,1);
	    gpio_write(B_PLUS,1);
	    break;
	  //backward
	  case 's':
	    pwm_init(2,pins,true);
	    gpio_write(A_PLUS,0);
	    gpio_write(B_PLUS,0);
	    break;
	  //left
	  case 'a':
	    pwm_init(2,pins,true);
	    gpio_write(A_PLUS,1);
	    gpio_write(B_PLUS,0);
	    break;
	  //right
	  case 'd':
	    pwm_init(2,pins,true);
	    gpio_write(A_PLUS,0);
	    gpio_write(B_PLUS,1);
	    break;
	  //stop
	  case 'p':
	    gpio_write(A_MINUS,0);
	    gpio_write(B_MINUS,0);
	    break;
	  //else: keep
	  default:
	    break;
	}
	
        vTaskDelay(20);
        xQueueSend(*queue, &count, 0);
        count++;
    }
}

static QueueHandle_t mainqueue;

void user_init(void)
{
    uart_set_baud(0, 115200);
    printf("SDK version:%s\n", sdk_system_get_sdk_version());
    mainqueue = xQueueCreate(4, sizeof(uint32_t));
    xTaskCreate(bluetooth, "bluetooth", 256, &mainqueue, 2, NULL);
    xTaskCreate(ultrasonic, "ultrasonic", 256, &mainqueue, 4, NULL);
    xTaskCreate(motorcontrol, "motorcontrol", 256, &mainqueue, 3, NULL);
}

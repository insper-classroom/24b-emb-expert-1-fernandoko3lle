#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"  

#include "servo.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include <queue.h>
#include "hardware/pwm.h"

volatile bool direction = true;
volatile int servoPin = 16; 
volatile int servoPin2 = 17;

QueueHandle_t xQueuePot;
QueueHandle_t xQueuePot2;
SemaphoreHandle_t xAdcSemaphore;  

int map_value(int n) {
    if (n >= 15 && n <= 4095) {
        return ((n - 15) * 2000) / 4080 + 400;
    } else {
        printf("O valor deve estar entre 15 e 4095.\n");
        return -1;
    }
}

void potenciometro_task_1() {
    while(1) {
        int pot_value;
        if (xSemaphoreTake(xAdcSemaphore, portMAX_DELAY)) {
            adc_select_input(0); 
            pot_value = adc_read();
            xSemaphoreGive(xAdcSemaphore);  
        }

        printf("Valor do potenciômetro 1: %d\n", pot_value);
        xQueueSend(xQueuePot, &pot_value, 0);
        vTaskDelay(pdMS_TO_TICKS(100));  
    }
}

void potenciometro_task_2() {
    while(1) {
        int pot_value;
        if (xSemaphoreTake(xAdcSemaphore, portMAX_DELAY)) {
            adc_select_input(1);  
            pot_value = adc_read();
            xSemaphoreGive(xAdcSemaphore);  
        }

        printf("Valor do potenciômetro 2: %d\n", pot_value);
        xQueueSend(xQueuePot2, &pot_value, 0);
        vTaskDelay(pdMS_TO_TICKS(100));  
    }
}

void servo_task_1() {
    int currentMillis1 = 400;
    setServo(servoPin, currentMillis1);
    int pot_value;

    while (1) {
        if (xQueueReceive(xQueuePot, &pot_value, pdMS_TO_TICKS(100))) {
            currentMillis1 = map_value(pot_value);
            printf("currentMillis1: %d\n", currentMillis1);
            setMillis(servoPin, currentMillis1);
            vTaskDelay(pdMS_TO_TICKS(5));  
        }
    }
}

void servo_task_2() {
    int currentMillis2 = 400;
    setServo(servoPin2, currentMillis2);
    int pot_value;

    while (1) {
        if (xQueueReceive(xQueuePot2, &pot_value, pdMS_TO_TICKS(100))) {
            currentMillis2 = map_value(pot_value);
            printf("currentMillis2: %d\n", currentMillis2);
            setMillis(servoPin2, currentMillis2);
            vTaskDelay(pdMS_TO_TICKS(5));  
        }
    }
}

int main() {
    stdio_init_all();  
   
    adc_init();
    adc_gpio_init(26);  
    adc_gpio_init(27);  

   
    xAdcSemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(xAdcSemaphore);

    xQueuePot = xQueueCreate(32, sizeof(int));
    xQueuePot2 = xQueueCreate(32, sizeof(int));

    xTaskCreate(potenciometro_task_1, "potenciometro_task_1", 1024, NULL, 1, NULL);
    xTaskCreate(servo_task_1, "servo_task_1", 1024, NULL, 1, NULL);

    xTaskCreate(potenciometro_task_2, "potenciometro_task_2", 1024, NULL, 1, NULL);
    xTaskCreate(servo_task_2, "servo_task_2", 1024, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true) {
    }
}

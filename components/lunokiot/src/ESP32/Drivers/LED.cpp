#include <hal/gpio_types.h> // ESP32 gpio

#include "LunokIoT.hpp"
#include "ESP32/Driver.hpp"
#include "ESP32/Drivers/LED.hpp"

#include <stdio.h>
#include "driver/ledc.h"
#include "esp_err.h"
#include <cmath>

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO          (10) // Define the output GPIO
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_8_BIT
#define LEDC_FREQUENCY          (5000) // Hz
#define MAX_DUTY_CYCLE          (int)(pow(2, LEDC_DUTY_RES) - 1)  

using namespace LunokIoT;

ledc_timer_config_t ledc_timer = { };
ledc_channel_config_t ledc_channel = { };

LEDDriver::LEDDriver(): DriverBaseClass((const char*)"(-) LED", (unsigned long)23) {
    printf("%p %s Setup \n", this, name);

    // Set the LEDC peripheral configuration

    // Prepare and then apply the LEDC PWM timer configuration
    
    ledc_timer.speed_mode       = LEDC_MODE;
    ledc_timer.timer_num        = LEDC_TIMER;
    ledc_timer.duty_resolution  = LEDC_DUTY_RES;
    ledc_timer.freq_hz          = LEDC_FREQUENCY;  // Set output frequency at 5 kHz
    ledc_timer.clk_cfg          = LEDC_AUTO_CLK;

    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    
    ledc_channel.speed_mode     = LEDC_MODE;
    ledc_channel.channel        = LEDC_CHANNEL;
    ledc_channel.timer_sel      = LEDC_TIMER;
    ledc_channel.intr_type      = LEDC_INTR_DISABLE;
    ledc_channel.gpio_num       = LEDC_OUTPUT_IO;
    ledc_channel.duty           = 0; // Set duty to 0%
    ledc_channel.hpoint         = 0;
    ledc_channel_config(&ledc_channel);
    ledc_fade_func_install(0);
    
    
}
//int dutyCycle = 0;
bool LEDDriver::Loop() {
    return true; //@TODO debug

    int timeDutyCycle = xTaskGetTickCount() % MAX_DUTY_CYCLE;
    //int seconds = xTaskGetTickCount() / (portTICK_RATE_MS * 1000);
    //int timeDutyCycleTest = ((xTaskGetTickCount() * LEDC_FREQUENCY) / (portTICK_RATE_MS * 1000))% MAX_DUTY_CYCLE;
    printf("calcDuty: %d\n", timeDutyCycle );
    ledc_set_duty_and_update(LEDC_MODE, LEDC_CHANNEL, timeDutyCycle, 0);
    /*
    dutyCycle++;
    if ( dutyCycle > MAX_DUTY_CYCLE ) {
        dutyCycle = 0;
    }*/
    return true;
}

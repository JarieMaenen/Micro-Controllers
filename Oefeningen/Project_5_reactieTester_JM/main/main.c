#include <stdio.h>
#include "driver/gpio.h"
#include "myGPIO.h"
#include "esp_attr.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gptimer.h"

#define KNOP 4
#define LED 2
/*
#define esp_timer_handle_t void*
#define esp_timer_create_args_t void*

void esp_timer_create(const esp_timer_create_args_t* create_args, esp_timer_handle_t* out_handle)
{
    // Placeholder implementation
}
*/

gptimer_handle_t gptimer = NULL;
void timer_setup()
{

    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT, // Select the default clock source
        .direction = GPTIMER_COUNT_UP,      // Counting direction is up
        .resolution_hz = 1 * 1000 * 1000,   // Resolution is 1 MHz, i.e., 1 tick equals 1 microsecond
    };
    // Create a timer instance
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));
    // Enable the timer
    ESP_ERROR_CHECK(gptimer_enable(gptimer));
    // Start the timer
    // ESP_ERROR_CHECK(gptimer_start(gptimer));
}

int run = 1;
double time;

static void IRAM_ATTR de_isr_functie(void *arg)
{
    run = 0;
    // Check the timer's resolution
    uint32_t resolution_hz;
    gptimer_get_resolution(gptimer, &resolution_hz);
    // Read the current count value
    uint64_t count;
    gptimer_get_raw_count(gptimer, &count);
    // (Optional) Convert the count value to time units (seconds)
    time = (double)count / resolution_hz;
}

void app_main(void)
{
    myGPIO_LED_Setup(LED);
    myGPIO_KNOP_Setup(KNOP);
    timer_setup();

    gpio_set_intr_type(KNOP, GPIO_INTR_NEGEDGE);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(KNOP, de_isr_functie, 0);

    while(1)
    {
        vTaskDelay(1000);
        myGPIO_LED_On(LED);
        gptimer_set_raw_count(gptimer, 0); // Reset the timer count to 0
        gptimer_start(gptimer);

        while(run)
        {
            vTaskDelay(100);
        }
        printf("Tijd: %f seconden\n", time);
        run = 1;
    }
}

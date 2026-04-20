
#include <stdio.h>
#include "driver/gpio.h"
#include "myGPIO.h"
#include "esp_attr.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gptimer.h"
#include "esp_random.h"
#include "esp_err.h"

#define LED 4
#define KNOP 5
#define DEBOUNCE_MS 50

gptimer_handle_t gptimer;
uint64_t reaction_time;
volatile int flag = 0;
volatile uint32_t last_isr_tick = 0;

static void IRAM_ATTR de_isr_functie(void *arg)
{
    uint32_t now = xTaskGetTickCountFromISR();
    if ((now - last_isr_tick) < pdMS_TO_TICKS(DEBOUNCE_MS)) {
        return;
    }
    last_isr_tick = now;

    uint64_t count;
    gptimer_stop(gptimer);
    gptimer_get_raw_count(gptimer, &count);
    reaction_time = count;
    flag = 1;
}

void app_main(void)
{
    myGPIO_BUTTON_Setup(KNOP);

    gpio_set_intr_type(KNOP, GPIO_INTR_NEGEDGE);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(KNOP, de_isr_functie, 0);

    myGPIO_LED_Setup(LED);

    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000, // 1 MHz
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));
    ESP_ERROR_CHECK(gptimer_enable(gptimer));

    while (1)
    {
        uint32_t delay_ms = 1000 + (esp_random() % 4000);
        vTaskDelay(delay_ms / portTICK_PERIOD_MS);

        gpio_set_level(LED, 1); // LED aan
        ESP_ERROR_CHECK(gptimer_start(gptimer));

        while (!flag)
        {
            // wacht op interrupt
        }

        ESP_ERROR_CHECK(gptimer_stop(gptimer));
        uint64_t seconds = reaction_time / 1000000;
        uint32_t millis = (reaction_time % 1000000) / 1000;
        printf("Reaction time: %llu.%03u s\n", seconds, (unsigned)millis);

        gpio_set_level(LED, 0); // LED uit
        flag = 0;
        ESP_ERROR_CHECK(gptimer_set_raw_count(gptimer, 0));
    }
}




// #include <stdio.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "myUART.h"
// #include "driver/uart.h"
// #include "esp_system.h"
// #include "esp_log.h"
// #include "esp_random.h"

// // int randomGetal(void)
// // {
// //     int randomGetal = esp_random() % 11; // Get a random number between 0 and 10
// //     return randomGetal;
// // }

// void randomGetal(void)
// {
//     int randomGetal = (esp_random() % 9) + 1; // Get a random number between 1 and 9
//     //myUART_printf("Random Getal: %d\n", randomGetal);

//     int ingevoerdGetal = 0;
//      // Read user input from UART

//     while(ingevoerdGetal == 0)
//         {
//     scanf("%d", &ingevoerdGetal);
    
//     if(ingevoerdGetal == randomGetal) {
//         myUART_printf("Correct! The random number was %d.\n", randomGetal);
//     } 
//     else 
//     {
//         myUART_printf("Incorrect. The random number was %d.\n", randomGetal);
//     }
//         }
// }

// void app_main(void)
// {   
//     myUART_setup(115200); // baud rate 115200
//     randomGetal();
//     //myUART_printf("Random Getal: %d\n", getal);
// }
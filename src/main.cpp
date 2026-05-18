#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_task_wdt.h"

constexpr gpio_num_t LED_PINS[] = {
    GPIO_NUM_15,
    GPIO_NUM_16,
    GPIO_NUM_17,
    GPIO_NUM_18
};

constexpr uint8_t LED_COUNT = sizeof(LED_PINS) / sizeof(LED_PINS[0]);

constexpr gpio_num_t BUTTON_PIN = GPIO_NUM_19;

// ESP32-S3: GPIO1 = ADC1_CH0
constexpr adc_channel_t POT_CHANNEL = ADC_CHANNEL_0;

constexpr uint32_t MIN_DELAY_MS = 100;
constexpr uint32_t MAX_DELAY_MS = 1000;

constexpr uint32_t WDT_TIMEOUT_MS = 3000;

volatile bool freezeRequested = false;
volatile uint32_t lastInterruptTime = 0;

adc_oneshot_unit_handle_t adcHandle;

void IRAM_ATTR buttonIsrHandler(void* arg)
{
    const uint32_t now = xTaskGetTickCountFromISR();

    if ((now - lastInterruptTime) > pdMS_TO_TICKS(50))
    {
        freezeRequested = true;
        lastInterruptTime = now;
    }
}

void configureButton()
{
    gpio_config_t config = {};
    config.pin_bit_mask = 1ULL << BUTTON_PIN;
    config.mode = GPIO_MODE_INPUT;
    config.pull_up_en = GPIO_PULLUP_ENABLE;
    config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    config.intr_type = GPIO_INTR_NEGEDGE;

    gpio_config(&config);

    gpio_install_isr_service(0);

    gpio_isr_handler_add(
        BUTTON_PIN,
        buttonIsrHandler,
        nullptr
    );
}

void configureLeds()
{
    gpio_config_t config = {};
    config.mode = GPIO_MODE_OUTPUT;

    for (uint8_t i = 0; i < LED_COUNT; ++i)
    {
        config.pin_bit_mask |= (1ULL << LED_PINS[i]);
    }

    gpio_config(&config);
}

void configureAdc()
{
    adc_oneshot_unit_init_cfg_t unitConfig = {};
    unitConfig.unit_id = ADC_UNIT_1;

    adc_oneshot_new_unit(&unitConfig, &adcHandle);

    adc_oneshot_chan_cfg_t channelConfig = {};
    channelConfig.bitwidth = ADC_BITWIDTH_DEFAULT; // 12bit 0..4095
    channelConfig.atten = ADC_ATTEN_DB_12;

    adc_oneshot_config_channel(adcHandle, POT_CHANNEL, &channelConfig);
}

void configureWatchdog()
{
    esp_task_wdt_config_t config = {};
    config.timeout_ms = WDT_TIMEOUT_MS;
    config.idle_core_mask = 0;
    config.trigger_panic = true;

    ESP_ERROR_CHECK(
        esp_task_wdt_reconfigure(&config) // esp_task_wdt_init
    );

    ESP_ERROR_CHECK(
        esp_task_wdt_add(nullptr)
    );
}

void updateLeds(uint8_t value)
{
    for (uint8_t bit = 0; bit < LED_COUNT; ++bit)
    {
        const bool isBitSet = value & (1 << bit);

        gpio_set_level(LED_PINS[bit], isBitSet ? 1 : 0);
    }
}

uint32_t mapAdcToDelayMs(int adcValue)
{
    constexpr int ADC_MIN = 0;
    constexpr int ADC_MAX = 4095;

    return MIN_DELAY_MS +
           ((MAX_DELAY_MS - MIN_DELAY_MS) * adcValue) / (ADC_MAX - ADC_MIN);
}

uint32_t readSpeedDelayMs()
{
    int adcValue = 0;
    adc_oneshot_read(adcHandle, POT_CHANNEL, &adcValue);

    printf("adcValue: %d\n", adcValue);

    return mapAdcToDelayMs(adcValue);
}

void simulateSystemFreeze()
{
    while (true)
    {
        // Intentional freeze.
        // Watchdog should trigger reset.
    }
}

extern "C" void app_main()
{
    printf("App started\n");

    configureLeds();
    configureButton();
    configureAdc();
    configureWatchdog();

    uint8_t counter = 0;

    while (true)
    {
        if (freezeRequested)
        {
            freezeRequested = false;
            printf("Simulating freeze...\n");
            simulateSystemFreeze();
        }

        updateLeds(counter);

        counter++;

        if (counter >= (1 << LED_COUNT))
        {
            counter = 0;
        }

        const uint32_t delayMs = readSpeedDelayMs();

        esp_task_wdt_reset();

        vTaskDelay(pdMS_TO_TICKS(delayMs));
    }
}
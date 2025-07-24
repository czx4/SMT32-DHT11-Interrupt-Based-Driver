# DHT11 Driver for STM32 HAL

This is a configurable and non-blocking driver for the DHT11 temperature and humidity sensor using STM32 HAL libraries. It supports asynchronous communication with callbacks, GPIO/EXTI edge detection, and timer-based timeout handling. Tested and developed on NUCLEO-F303RE.

## Integration
1. Include the Driver

Add the driver files to your project:

Src/dht11.c

Inc/dht11.h

## Configuration & Usage Guide
### 1. Timer Selection

In your dht11.h file, define the timer you wish to use:

    #define DHT_TIM_6

Supported Timers:
DHT_TIM_1, DHT_TIM_2, ..., DHT_TIM_17

### 2. GPIO Selection

Also define the GPIO port and pin used for communication with the DHT11 sensor:

    #define DHT_GPIO_PORT GPIOA
    #define DHT_GPIO_PIN_5

Supported Pins:
DHT_GPIO_PIN_0 through DHT_GPIO_PIN_15

>  **⚠️ Note:**
>   If your selected Timer or GPIO pin isn't supported out of the box, you can add support manually by replicating the define pattern in the dht11.c file.

### 3. Including the Driver

Make sure to include the DHT11 driver in your main file:

#include "dht11.h"

### 4. Initialization

Before reading data, initialize the GPIO and Timer:

DHT11_Init();

### 5. Reading Sensor Data

Call the read function:

DHT11_Read(&temperature, &humidity, DHT11_callback);

temperature and humidity should be uint8_t variables.

DHT11_callback must match the prototype:
    
    void DHT11_callback(DHT11_StatusTypeDef status);

Status Codes

The status parameter in the callback can be:

    DHT11_OK – Reading successful

    DHT11_TIMEOUT – Sensor did not respond in time

    DHT11_CHECKSUM_MISMATCH – Data received but checksum failed

Example Code

Main Application:

    uint8_t temp, hum;
    char buffer[64];
    
    DHT11_Init();
    
    while (1)
    {
        DHT11_Read(&temp, &hum, DHT11_done);
    }

Callback Function:

    void DHT11_done(DHT11_StatusTypeDef status)
    {
        if (status == DHT11_OK) {
            int len = sprintf(buffer, "Humidity: %u\r\n", hum);
            HAL_UART_Transmit(&huart2, (uint8_t *)buffer, len, HAL_MAX_DELAY);
    
            len = sprintf(buffer, "Temperature: %u\r\n", temp);
            HAL_UART_Transmit(&huart2, (uint8_t *)buffer, len, HAL_MAX_DELAY);
        }
    }
## Custom interrupts
If you want to use the same exti/tim interrupt handlers and callbacks see the instructions for it in dht11.h file.

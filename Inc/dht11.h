#ifndef DHT11
#define DHT11
#include "stm32f3xx_hal.h"
#include <stdbool.h>
#include <string.h>

#define DHT11_OK 1
#define DHT11_ERROR 0

//wrong readings if all data is set to 255
typedef union{
    uint8_t raw[5];

    struct{
    uint8_t integral_rh;
    uint8_t fraction_rh;
    uint8_t integral_temp;
    uint8_t fraction_temp;
    uint8_t checksum;
    }values;
}DHT11_data;

extern volatile bool DHT11_data_ready;
extern TIM_HandleTypeDef DHT11_tim;

uint8_t DHT11_Init(TIM_TypeDef *TIM_instance, GPIO_TypeDef *GPIO_PORT, uint16_t GPIO_PIN);
uint8_t DHT11_Read(DHT11_data *data_ptr);
void DHT11_Service_Callback(void);// function to call in timer time elapsed callback

#endif

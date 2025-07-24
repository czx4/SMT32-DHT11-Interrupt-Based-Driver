
#ifndef INC_DHT11_H_
#define INC_DHT11_H_
#include "stm32XXXX_hal.h" //example "stm32f3xx_hal.h"

typedef enum {
	DHT11_OK = 0,
	DHT11_TIMEOUT,
	DHT11_CHECKSUM_MISMATCH
} DHT11_StatusTypeDef;

extern void DHT11_Init(void);
extern void DHT11_Read(uint8_t *temp,uint8_t *humi,void(*callback)(DHT11_StatusTypeDef status));
void DHT11_StateMachine(void);
void DHT11_GPIO_EDGE(void);
extern TIM_HandleTypeDef *DHT11_GetTIMHandle(void);
extern uint16_t DHT11_GetPin(void);

// === SELECT TIMER ===
#define DHT_TIM_X
// example: #define DHT_TIM_6

// === SELECT GPIO ===
#define DHT_GPIO_PORT GPIOX
//example: #define DHT_GPIO_PORT GPIOA

#define DHT_GPIO_PIN_X
//example: #define DHT_GPIO_PIN_0

#endif /* INC_DHT11_H_ */


// If you want to change something in the EXTI or TIM interrupt handler that the DHT11 uses include dht.h,
// delete the specified fragment from dht11.c paste this into your stm32fXXXX_it.c

//EXTI: delete this fragment from the define at the start of the file dht11.c (X is for the pin you use)

//void EXTIX_IRQHandler(void) {
//	HAL_GPIO_EXTI_IRQHandler(DHT_GPIO_PIN);
//}

// Also delete this from above DHT11_init:

//void HAL_GPIO_EXTI_Callback(uint16_t pin){
//	if(pin==DHT_GPIO_PIN){
//		DHT11_GPIO_EDGE();
//	}
//}

// Than add this code to the stmXXXX_it.c:
// (X is for the pin you use unless you use 9_5 or 15_10 which you can see how should look in dht11.c)
//void EXTIX_IRQHandler(void){
//    HAL_GPIO_EXTI_IRQHandler(DHT11_GetPin());
//}
//
// this is the same for every pin:
//void HAL_GPIO_EXTI_Callback(uint16_t pin){
//	if(pin==DHT11_GetPin()){
//		DHT11_GPIO_EDGE();
//	}
//}




//TIM: delete this fragment from the define at the start of the file dht11.c (X is for the TIM you use)

//void TIMX_IRQHandler(void)
//{
//    HAL_TIM_IRQHandler(DHT_HTIM);
//}

// Also delete this from above DHT11_init:

//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
//    if (htim == DHT_HTIM) {
//        DHT11_StateMachine();
//    }
//}

// Than add this code to the stmXXXX_it.c:
// (X is for the TIM you use, some timers have specific irqhandlers which you can see how should look in dht11.c)

//void TIMX_IRQHandler(void)
//{
//    HAL_TIM_IRQHandler(DHT11_GetTIMHandle());
//}

// this is the same for every TIM:
//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
//    if (htim == DHT11_GetTIMHandle()) {
//        DHT11_StateMachine();
//    }
//}


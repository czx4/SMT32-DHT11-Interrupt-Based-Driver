#include "dht11.h"
#include<string.h>


static uint8_t state;
static uint8_t data_index=0;
static uint8_t data[5]={0};
static uint8_t *temperature;
static uint8_t *humidity;
static uint8_t readComplete=1;
static uint8_t starting_conv=0;
static uint8_t EXTI_enable;

void DHT11_StateMachine(void);
void DHT11_GPIO_EDGE(void);
static uint32_t get_pclk1_prescaler(void);
static uint32_t get_pclk2_prescaler(void);
static void (*dht_callback)(DHT11_StatusTypeDef status) = NULL;



#ifdef DHT_TIM_1
static TIM_HandleTypeDef htim1;
#define DHT_TIM_IRQn TIM1_UP_TIM16_IRQn
#define DHT_HTIM (&htim1)
#define DHT_TIM_INSTANCE TIM1
void TIM1_UP_TIM16_IRQHandler(void) {
    HAL_TIM_IRQHandler(DHT_HTIM);
}
#endif

#ifdef DHT_TIM_2
static TIM_HandleTypeDef htim2;
#define DHT_TIM_IRQn TIM2_IRQn
#define DHT_HTIM (&htim2)
#define DHT_TIM_INSTANCE TIM2
void TIM2_IRQHandler(void)
{
    HAL_TIM_IRQHandler(DHT_HTIM);
}
#endif

#ifdef DHT_TIM_3
static TIM_HandleTypeDef htim3;
#define DHT_TIM_IRQn TIM3_IRQn
#define DHT_HTIM (&htim3)
#define DHT_TIM_INSTANCE TIM3
void TIM3_IRQHandler(void)
{
    HAL_TIM_IRQHandler(DHT_HTIM);
}
#endif

#ifdef DHT_TIM_4
static TIM_HandleTypeDef htim4;
#define DHT_TIM_IRQn TIM4_IRQn
#define DHT_HTIM (&htim4)
#define DHT_TIM_INSTANCE TIM4
void TIM4_IRQHandler(void)
{
    HAL_TIM_IRQHandler(DHT_HTIM);
}
#endif

#ifdef DHT_TIM_6
static TIM_HandleTypeDef htim6;
#define DHT_TIM_IRQn TIM6_DAC_IRQn
#define DHT_HTIM (&htim6)
#define DHT_TIM_INSTANCE TIM6
void TIM6_DAC_IRQHandler(void)
{
    HAL_TIM_IRQHandler(DHT_HTIM);
}
#endif

#ifdef DHT_TIM_7
static TIM_HandleTypeDef htim7;
#define DHT_TIM_IRQn TIM7_IRQn
#define DHT_HTIM (&htim7)
#define DHT_TIM_INSTANCE TIM7
void TIM7_IRQHandler(void)
{
    HAL_TIM_IRQHandler(DHT_HTIM);
}
#endif

#ifdef DHT_TIM_8
static TIM_HandleTypeDef htim8;
#define DHT_TIM_IRQn TIM8_UP_IRQn
#define DHT_HTIM (&htim8)
#define DHT_TIM_INSTANCE TIM8
void TIM8_UP_IRQHandler(void) {
    HAL_TIM_IRQHandler(DHT_HTIM);
}
#endif

#ifdef DHT_TIM_15
static TIM_HandleTypeDef htim15;
#define DHT_TIM_IRQn TIM1_BRK_TIM15_IRQn
#define DHT_HTIM (&htim15)
#define DHT_TIM_INSTANCE TIM15
void TIM1_BRK_TIM15_IRQHandler(void) {
    HAL_TIM_IRQHandler(DHT_HTIM);
}
#endif

#ifdef DHT_TIM_16
static TIM_HandleTypeDef htim16;
#define DHT_TIM_IRQn TIM1_UP_TIM16_IRQn
#define DHT_HTIM (&htim16)
#define DHT_TIM_INSTANCE TIM16
void TIM1_UP_TIM16_IRQHandler(void) {
    HAL_TIM_IRQHandler(DHT_HTIM);
}
#endif

#ifdef DHT_TIM_17
static TIM_HandleTypeDef htim17;
#define DHT_TIM_IRQn TIM1_TRG_COM_TIM17_IRQn
#define DHT_HTIM (&htim17)
#define DHT_TIM_INSTANCE TIM17
void TIM1_TRG_COM_TIM17_IRQHandler(void) {
    HAL_TIM_IRQHandler(DHT_HTIM);
}
#endif

#ifdef DHT_GPIO_PIN_0
#define DHT_GPIO_PIN GPIO_PIN_0
#define DHT_EXTI_IRQn EXTI0_IRQn
void EXTI0_IRQHandler(void) {
	HAL_GPIO_EXTI_IRQHandler(DHT_GPIO_PIN);
}
#endif

#ifdef DHT_GPIO_PIN_1
#define DHT_GPIO_PIN GPIO_PIN_1
#define DHT_EXTI_IRQn EXTI1_IRQn
void EXTI1_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(DHT_GPIO_PIN);
}
#endif

#ifdef DHT_GPIO_PIN_2
#define DHT_GPIO_PIN GPIO_PIN_2
#define DHT_EXTI_IRQn EXTI2_TSC_IRQn
void EXTI2_TSC_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(DHT_GPIO_PIN);
}
#endif

#ifdef DHT_GPIO_PIN_3
#define DHT_GPIO_PIN GPIO_PIN_3
#define DHT_EXTI_IRQn EXTI3_IRQn
void EXTI3_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(DHT_GPIO_PIN);
}
#endif

#ifdef DHT_GPIO_PIN_4
#define DHT_GPIO_PIN GPIO_PIN_4
#define DHT_EXTI_IRQn EXTI4_IRQn
void EXTI4_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(DHT_GPIO_PIN);
}
#endif

#ifdef DHT_GPIO_PIN_5
#define DHT_GPIO_PIN GPIO_PIN_5
#define DHT_EXTI_IRQn EXTI9_5_IRQn
void EXTI9_5_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(DHT_GPIO_PIN);
}
#endif

#ifdef DHT_GPIO_PIN_6
#define DHT_GPIO_PIN GPIO_PIN_6
#define DHT_EXTI_IRQn EXTI9_5_IRQn
void EXTI9_5_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(DHT_GPIO_PIN);
}
#endif

#ifdef DHT_GPIO_PIN_7
#define DHT_GPIO_PIN GPIO_PIN_7
#define DHT_EXTI_IRQn EXTI9_5_IRQn
void EXTI9_5_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(DHT_GPIO_PIN);
}
#endif

#ifdef DHT_GPIO_PIN_8
#define DHT_GPIO_PIN GPIO_PIN_8
#define DHT_EXTI_IRQn EXTI9_5_IRQn
void EXTI9_5_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(DHT_GPIO_PIN);
}
#endif

#ifdef DHT_GPIO_PIN_9
#define DHT_GPIO_PIN GPIO_PIN_9
#define DHT_EXTI_IRQn EXTI9_5_IRQn
void EXTI9_5_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(DHT_GPIO_PIN);
}
#endif

#ifdef DHT_GPIO_PIN_10
#define DHT_GPIO_PIN GPIO_PIN_10
#define DHT_EXTI_IRQn EXTI15_10_IRQn
void EXTI15_10_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(DHT_GPIO_PIN);
}
#endif

#ifdef DHT_GPIO_PIN_11
#define DHT_GPIO_PIN GPIO_PIN_11
#define DHT_EXTI_IRQn EXTI15_10_IRQn
void EXTI15_10_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(DHT_GPIO_PIN);
}
#endif

#ifdef DHT_GPIO_PIN_12
#define DHT_GPIO_PIN GPIO_PIN_12
#define DHT_EXTI_IRQn EXTI15_10_IRQn
void EXTI15_10_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(DHT_GPIO_PIN);
}
#endif

#ifdef DHT_GPIO_PIN_13
#define DHT_GPIO_PIN GPIO_PIN_13
#define DHT_EXTI_IRQn EXTI15_10_IRQn
void EXTI15_10_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(DHT_GPIO_PIN);
}
#endif

#ifdef DHT_GPIO_PIN_14
#define DHT_GPIO_PIN GPIO_PIN_14
#define DHT_EXTI_IRQn EXTI15_10_IRQn
void EXTI15_10_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(DHT_GPIO_PIN);
}
#endif

#ifdef DHT_GPIO_PIN_15
#define DHT_GPIO_PIN GPIO_PIN_15
#define DHT_EXTI_IRQn EXTI15_10_IRQn
void EXTI15_10_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(DHT_GPIO_PIN);
}
#endif

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim == DHT_HTIM) {
        DHT11_StateMachine();
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t pin){
	if(pin==DHT_GPIO_PIN){
		DHT11_GPIO_EDGE();
	}
}


TIM_HandleTypeDef *DHT11_GetTIMHandle(void){
	return DHT_HTIM;
}
uint16_t DHT11_GetPin(void){
	return DHT_GPIO_PIN;
}


void DHT11_Init(void){
#ifdef DHT_TIM_1
__HAL_RCC_TIM1_CLK_ENABLE();
#endif

#ifdef DHT_TIM_2
__HAL_RCC_TIM2_CLK_ENABLE();
#endif

#ifdef DHT_TIM_3
__HAL_RCC_TIM3_CLK_ENABLE();
#endif

#ifdef DHT_TIM_4
__HAL_RCC_TIM4_CLK_ENABLE();
#endif

#ifdef DHT_TIM_6
__HAL_RCC_TIM6_CLK_ENABLE();
#endif

#ifdef DHT_TIM_7
__HAL_RCC_TIM7_CLK_ENABLE();
#endif

#ifdef DHT_TIM_8
__HAL_RCC_TIM8_CLK_ENABLE();
#endif

#ifdef DHT_TIM_15
__HAL_RCC_TIM15_CLK_ENABLE();
#endif

#ifdef DHT_TIM_16
__HAL_RCC_TIM16_CLK_ENABLE();
#endif

#ifdef DHT_TIM_17
__HAL_RCC_TIM17_CLK_ENABLE();
#endif
	GPIO_InitTypeDef GPIO_InitStruct={0};
	GPIO_InitStruct.Pin=DHT_GPIO_PIN;
	GPIO_InitStruct.Mode=GPIO_MODE_OUTPUT_OD;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(DHT_GPIO_PORT, &GPIO_InitStruct);

	uint32_t timerClockFreq;

	if (DHT_TIM_INSTANCE == TIM1 || //APB2
		DHT_TIM_INSTANCE == TIM8 ||
		DHT_TIM_INSTANCE== TIM15 ||
		DHT_TIM_INSTANCE == TIM16 ||
		DHT_TIM_INSTANCE == TIM17){
	    timerClockFreq= HAL_RCC_GetPCLK2Freq();
	    if (get_pclk2_prescaler()!= 1) {
	        timerClockFreq *= 2;
	    }
	}
	else{ //APB1
		timerClockFreq=HAL_RCC_GetPCLK1Freq();
		if (get_pclk1_prescaler() !=1) {
		    timerClockFreq *= 2;
		}
	}
	DHT_HTIM->Instance=DHT_TIM_INSTANCE;
	DHT_HTIM->Init.Prescaler=(timerClockFreq/1000000)-1;
	DHT_HTIM->Init.CounterMode=TIM_COUNTERMODE_UP;
	DHT_HTIM->Init.Period=0xFFFF;
	DHT_HTIM->Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;
	DHT_HTIM->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	HAL_TIM_Base_Init(DHT_HTIM);
	HAL_NVIC_SetPriority(DHT_TIM_IRQn,0, 0);
	HAL_NVIC_EnableIRQ(DHT_TIM_IRQn);
	HAL_NVIC_SetPriority(DHT_EXTI_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DHT_EXTI_IRQn);
}

void DHT11_Read(uint8_t *temp,uint8_t *humi,void(*callback)(DHT11_StatusTypeDef status)){

if(readComplete==0){
	return;
}
readComplete=0;
state=0;
humidity=humi;
temperature=temp;
data_index=0;
starting_conv=0;
dht_callback=callback;
memset(data,0,5*sizeof(data[0]));

HAL_GPIO_WritePin(DHT_GPIO_PORT, DHT_GPIO_PIN, GPIO_PIN_RESET);
__HAL_TIM_SET_AUTORELOAD(DHT_HTIM,20000);
__HAL_TIM_SET_COUNTER(DHT_HTIM,0);
__HAL_TIM_CLEAR_FLAG(DHT_HTIM, TIM_FLAG_UPDATE);
__HAL_TIM_ENABLE_IT(DHT_HTIM, TIM_IT_UPDATE);
HAL_TIM_Base_Start_IT(DHT_HTIM);
}



void DHT11_StateMachine(void){
	switch(state){
	case 0:
		HAL_GPIO_WritePin(DHT_GPIO_PORT, DHT_GPIO_PIN, GPIO_PIN_SET);
		__HAL_TIM_SET_AUTORELOAD(DHT_HTIM,40);
		state++;
		break;

	case 1:
		GPIO_InitTypeDef GPIO_InitStruct={0};
		if(EXTI_enable==1){
			EXTI_enable=0;
			HAL_TIM_Base_Stop_IT(DHT_HTIM);
			GPIO_InitStruct.Pin=DHT_GPIO_PIN;
			GPIO_InitStruct.Mode=GPIO_MODE_OUTPUT_OD;
			GPIO_InitStruct.Pull = GPIO_NOPULL;
			GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
			HAL_GPIO_Init(DHT_GPIO_PORT, &GPIO_InitStruct);
			HAL_GPIO_WritePin(DHT_GPIO_PORT, DHT_GPIO_PIN, GPIO_PIN_SET);
			dht_callback(DHT11_TIMEOUT);
			readComplete=1;
			break;
		}
		GPIO_InitStruct.Pin=DHT_GPIO_PIN;
		GPIO_InitStruct.Mode=GPIO_MODE_IT_RISING_FALLING;
		GPIO_InitStruct.Pull=GPIO_NOPULL;
		HAL_GPIO_Init(DHT_GPIO_PORT, &GPIO_InitStruct);
		EXTI_enable=1;
		__HAL_TIM_SET_AUTORELOAD(DHT_HTIM,1000000);
		break;
	case 2:
		HAL_TIM_Base_Stop_IT(DHT_HTIM);
		if(*humidity==255 && *temperature==255){
			dht_callback(DHT11_CHECKSUM_MISMATCH);
		}
		else{
			dht_callback(DHT11_OK);
		}
		readComplete=1;
		break;
	}
}

void DHT11_GPIO_EDGE(void){
	if(EXTI_enable==0){
		return;
	}
	if(starting_conv<2||data_index>=41){
		starting_conv++;
		return;
	}
	if(HAL_GPIO_ReadPin(DHT_GPIO_PORT, DHT_GPIO_PIN)){//rising
		__HAL_TIM_SET_COUNTER(DHT_HTIM,0);
	}
	else{//falling
		uint32_t time=__HAL_TIM_GET_COUNTER(DHT_HTIM);
		data[data_index/8]<<=1;
		if(time>50){
			data[data_index/8]|=1;
		}
		else{
			data[data_index/8]|=0;
		}
		data_index++;
		if(data_index>=40){
			GPIO_InitTypeDef GPIO_InitStruct={0};
			GPIO_InitStruct.Pin=DHT_GPIO_PIN;
			GPIO_InitStruct.Mode=GPIO_MODE_OUTPUT_OD;
			GPIO_InitStruct.Pull = GPIO_NOPULL;
			GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
			HAL_GPIO_Init(DHT_GPIO_PORT, &GPIO_InitStruct);
			HAL_GPIO_WritePin(DHT_GPIO_PORT, DHT_GPIO_PIN, GPIO_PIN_SET);

			__HAL_TIM_SET_COUNTER(DHT_HTIM,0);
			__HAL_TIM_SET_AUTORELOAD(DHT_HTIM,1000000);
			if ((data[0] + data[1] + data[2] + data[3]) == data[4]){
				*humidity=data[0];
				*temperature=data[2];
			}
			else{
				*humidity=255;
				*temperature=255;
			}
			state++;
			EXTI_enable=0;
		}
	}
}



static uint32_t get_pclk1_prescaler(void) {
    uint32_t tmp = (RCC->CFGR & RCC_CFGR_PPRE1) >> RCC_CFGR_PPRE1_Pos;
    return (tmp < 4) ? 1 : (1 << (tmp - 3)); // 1, 2, 4, 8, 16
}

static uint32_t get_pclk2_prescaler(void) {
    uint32_t tmp = (RCC->CFGR & RCC_CFGR_PPRE2) >> RCC_CFGR_PPRE2_Pos;
    return (tmp < 4) ? 1 : (1 << (tmp - 3)); // 1, 2, 4, 8, 16
}


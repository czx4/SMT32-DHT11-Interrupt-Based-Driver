#include "dht11.h"

static void handle_uninit(void);
static void handle_ready(void);
static void handle_pd_start(void);
static void handle_dht_init(void);
static void handle_during_transmit(void);
static void handle_stab(void);


TIM_HandleTypeDef DHT11_tim;

enum // setting are for 72 Mhz initial clock, start trigger should be at most 55,55Hz, target trigger should be 100khz and stab trigger 1hz
{
    START_PRESCALER = 256-1, // for simplicity it will fire after 20ms as the datasheet doesnt name upper bound for pulling low 
    START_AUTORELOAD = 5625-1, 
    TARGET_PRESCALER = 72-1, // should fire every 10us
    TARGET_AUTORELOAD = 10-1,
    STAB_PRESCALER = 7200-1, // should fire every 1s (probably can go lower on that, but the datasheet states that 1 second is minimum)
    STAB_AUTORELOAD = 10000-1,
    MAX_WAIT_INTERVALS = 15
};

typedef enum{
    DURING_TRANSMIT,
    TRANSMIT_START,
    PULLUP_START_DHT,
    PULLDOWN_START_DHT,
    PULLUP_START,
    PULLDOWN_START,
    READY,
    UNINIT,
    STABILIZE_STATUS,
    NUM_STATES
} DHT_state;

typedef void (*state_func)(void);

static const state_func state_table[NUM_STATES] = 
{
    [UNINIT] = handle_uninit,
    [READY] = handle_ready,
    [PULLDOWN_START] = handle_pd_start,
    [PULLUP_START] = handle_dht_init,
    [PULLDOWN_START_DHT] = handle_dht_init,
    [PULLUP_START_DHT] = handle_dht_init,
    [TRANSMIT_START] = handle_dht_init,
    [DURING_TRANSMIT] = handle_during_transmit,
    [STABILIZE_STATUS] = handle_stab
};

static GPIO_TypeDef *GPIO_Port;
static uint16_t GPIO_Pin;

static volatile DHT_state current_state = UNINIT;
static volatile uint8_t time_inc_passed = 0;

static volatile uint8_t shift = 0;
static volatile uint8_t packet_num = 0;
static DHT11_data* data;
volatile bool DHT11_data_ready = false;
static volatile bool pin_state = GPIO_PIN_SET;
static const uint8_t min_wait_intervals[] = {
    [PULLUP_START] = 0,
    [PULLDOWN_START_DHT] = 4,
    [PULLUP_START_DHT] = 6,
    [TRANSMIT_START] = 3,
    [DURING_TRANSMIT] = 5
};

void DHT11_Service_Callback(void)
{
    state_table[current_state]();
}

uint8_t DHT11_Init(TIM_TypeDef *TIM_instance, GPIO_TypeDef *GPIO_PORT, uint16_t GPIO_PIN)
{
    GPIO_Port = GPIO_PORT;
    GPIO_Pin = GPIO_PIN;
    
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    DHT11_tim.Instance = TIM_instance;
    DHT11_tim.Init.Prescaler = START_PRESCALER;
    DHT11_tim.Init.CounterMode = TIM_COUNTERMODE_UP;
    DHT11_tim.Init.Period = START_AUTORELOAD;
    DHT11_tim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    if (HAL_TIM_Base_Init(&DHT11_tim) != HAL_OK){
        return DHT11_ERROR;
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&DHT11_tim, &sMasterConfig) != HAL_OK)
    {
        return DHT11_ERROR;
    }

    HAL_GPIO_DeInit(GPIO_Port, GPIO_Pin);
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIO_Port, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIO_Port, GPIO_Pin, GPIO_PIN_SET);

    current_state = READY;

    return DHT11_OK;
}

uint8_t DHT11_Read(DHT11_data *data_ptr)
{
    if (current_state != READY)
        return DHT11_ERROR;
    current_state = PULLDOWN_START;
    DHT11_data_ready = false;
    HAL_GPIO_WritePin(GPIO_Port, GPIO_Pin, GPIO_PIN_RESET);
    shift = 7;
    packet_num = 0;
    data = data_ptr;
    time_inc_passed = 0;
    pin_state = GPIO_PIN_SET;
    memset((void *) data->raw, 0, sizeof(*data));
    __HAL_TIM_SET_COUNTER(&DHT11_tim,0);
    __HAL_TIM_SET_PRESCALER(&DHT11_tim, START_PRESCALER);
    __HAL_TIM_SET_AUTORELOAD(&DHT11_tim, START_AUTORELOAD);
    __HAL_TIM_CLEAR_IT(&DHT11_tim, TIM_IT_UPDATE);
    if(HAL_TIM_Base_Start_IT(&DHT11_tim) != HAL_OK)
        return DHT11_ERROR;
    return DHT11_OK;
}

static void process_data(void)
{
    __HAL_TIM_SET_COUNTER(&DHT11_tim,0);
    __HAL_TIM_SET_PRESCALER(&DHT11_tim, STAB_PRESCALER);
    __HAL_TIM_SET_AUTORELOAD(&DHT11_tim, STAB_AUTORELOAD);
    current_state = STABILIZE_STATUS;
    HAL_GPIO_WritePin(GPIO_Port, GPIO_Pin, GPIO_PIN_SET);
    uint16_t sum = data->values.integral_rh + data->values.fraction_rh + data->values.integral_temp + data->values.fraction_temp;
    if((sum & 0xFF) != data->values.checksum)
    {
        memset((void *)data->raw, 255, sizeof(*data));
    }
    DHT11_data_ready = true;
}

static void error_reading(void)
{
    __HAL_TIM_SET_COUNTER(&DHT11_tim,0);
    __HAL_TIM_SET_PRESCALER(&DHT11_tim, STAB_PRESCALER);
    __HAL_TIM_SET_AUTORELOAD(&DHT11_tim, STAB_AUTORELOAD);
    current_state = STABILIZE_STATUS;
    HAL_GPIO_WritePin(GPIO_Port, GPIO_Pin, GPIO_PIN_SET);
    memset((void *)data->raw, 255, sizeof(*data));
    DHT11_data_ready = true;
}

static void handle_stab(void)
{
    HAL_TIM_Base_Stop_IT(&DHT11_tim);
    current_state = READY;
}

static void handle_uninit(void)
{
    HAL_TIM_Base_Stop_IT(&DHT11_tim);
    HAL_GPIO_WritePin(GPIO_Port, GPIO_Pin, GPIO_PIN_SET);
    memset((void *)data->raw, 255, sizeof(*data));
    DHT11_data_ready = true;
}

static void handle_ready(void)
{
    error_reading();
}

static void handle_pd_start(void)
{
    HAL_GPIO_WritePin(GPIO_Port, GPIO_Pin, GPIO_PIN_SET);
    current_state = PULLUP_START;
    __HAL_TIM_SET_COUNTER(&DHT11_tim, 0);
    __HAL_TIM_SET_PRESCALER(&DHT11_tim, TARGET_PRESCALER);
    __HAL_TIM_SET_AUTORELOAD(&DHT11_tim, TARGET_AUTORELOAD);
}

static void handle_dht_init(void)
{
    if(HAL_GPIO_ReadPin(GPIO_Port, GPIO_Pin) != pin_state)
    {
        if(time_inc_passed < min_wait_intervals[current_state])
        {
            error_reading();
            return;
        }
        pin_state = !pin_state;
        time_inc_passed = 0;
        --current_state;
    }

    if(++time_inc_passed == MAX_WAIT_INTERVALS)
        error_reading();
}

static void handle_during_transmit(void)
{
    if(HAL_GPIO_ReadPin(GPIO_Port, GPIO_Pin) == GPIO_PIN_RESET)
    {
        if(time_inc_passed > min_wait_intervals[current_state]) //1
        {
            data->raw[packet_num] |= (1<<shift);
        }
        if(--shift > 7)
        {
            ++packet_num;
            shift = 7;
        }
        //0
        pin_state = GPIO_PIN_RESET;
        time_inc_passed = 0;
        current_state = TRANSMIT_START;
        return;
    }

    if(++time_inc_passed == MAX_WAIT_INTERVALS){
        process_data();
    }
}

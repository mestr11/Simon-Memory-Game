/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/_intsup.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum {
  IDLE,
  DISPLAY_SEQUENCE,
  USER_INPUT,
  GAME_OVER,
} GameState;

typedef enum {
  RED = 0,
  BLUE,
  YELLOW,
  GREEN,
  NONE
} ColorIndex;

typedef struct {
  uint16_t button_pin;
  uint16_t light_pin;
  uint16_t note;
  uint8_t toggled;
} Color;

Color colors[] = {
  {GPIO_PIN_3, GPIO_PIN_12, (uint16_t) 261, 0},
  {GPIO_PIN_2, GPIO_PIN_11, (uint16_t) 293, 0},
  {GPIO_PIN_1, GPIO_PIN_6, (uint16_t) 329, 0},
  {GPIO_PIN_0, GPIO_PIN_5, (uint16_t) 349, 0},
};

typedef struct {
  GameState state;
  uint8_t sequence[64];
  uint8_t sequence_length;
  uint8_t input_index;
} GameContext;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define GAME_MAX_DELAY 500
#define GAME_DELAY_SHORT 200
#define DEBOUNCE_TIME 50
#define UART_MSG(msg) msg "\r\n"
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
GameContext game;
uint32_t rng = 198273;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM3_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */
void game_init(void);
void game_step(void);
void play_tone(uint16_t freq);
void stop_tone(void);
void toggle_color(Color *color);
void set_color(Color *color, uint8_t state);
uint8_t get_next_random_color(void);
uint8_t get_button_input(uint8_t reset);
void uart_print(char* msg);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM3_Init();
  MX_USART2_UART_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
  HAL_TIM_Base_Start_IT(&htim2);

  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_11|GPIO_PIN_12, GPIO_PIN_RESET);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    game_step();
    
    if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET) {
      HAL_Delay(DEBOUNCE_TIME);
      if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET) {
        while (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET);
        game_init();
      }
    }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 8399;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 9;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 83;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 4095;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LD2_Pin|Yellow_Light_Pin|Blue_Light_Pin|Red_Light_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PC0 PC1 PC2 PC3 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : LD2_Pin Yellow_Light_Pin Blue_Light_Pin Red_Light_Pin */
  GPIO_InitStruct.Pin = LD2_Pin|Yellow_Light_Pin|Blue_Light_Pin|Red_Light_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

uint8_t get_next_random_color() {
  rng = rng * 102934871 + 39165;
  return (uint8_t)((rng >> 16) & 0x03);
}

void play_tone(uint16_t freq) {
  uint32_t period = (1000000 / freq) - 1; // 1,000,00 HZ set by prescaler=83
  __HAL_TIM_SET_AUTORELOAD(&htim3, period);
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, period >> 2);
}

void stop_tone() {
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, 0);
}

void toggle_color(Color *color) {
  if (!color->toggled) {
    HAL_GPIO_WritePin(GPIOA, color->light_pin, GPIO_PIN_SET);
    play_tone(color->note);
    color->toggled = 1;
  }
  else{
    HAL_GPIO_WritePin(GPIOA, color->light_pin, GPIO_PIN_RESET);
    stop_tone();
    color->toggled = 0;
  }
}

void set_color(Color *color, uint8_t state) {
  color->toggled = !state;
  toggle_color(color);
}

void game_init() {
  rng = HAL_GetTick();

  game.sequence_length = 0;
  game.input_index = 0;
  game.state = DISPLAY_SEQUENCE;

  game.sequence[0] = get_next_random_color();
  game.sequence_length = 1;

  get_button_input(1); // Reset static variables
  uart_print(UART_MSG("Game started."));
}

void game_step() {
  switch (game.state) {
    case DISPLAY_SEQUENCE:

      // 1. Check if sequence is complete.
      if (game.input_index >= game.sequence_length) {
        game.state = USER_INPUT;
        game.input_index = 0;
        uart_print(UART_MSG("Reading user inputs..."));
        return;
      }

      // 2. Calculate sequence delay.
      uint32_t delay_modifier = (game.sequence_length >> 2) * 50;
      uint32_t sequence_delay = GAME_MAX_DELAY - delay_modifier;

      // 3. Display current sequence step.
      Color *current_color = &colors[game.sequence[game.input_index]];
      toggle_color(current_color);
      HAL_Delay(sequence_delay);
      toggle_color(current_color);
      HAL_Delay(100);

      // 4. Go to next color in sequence.
      game.input_index++;
      break;
  
    case USER_INPUT: {
      static ColorIndex last_color = NONE;

      // 1. Check if sequence is complete
      if (game.input_index >= game.sequence_length) {
        game.sequence[game.sequence_length] = get_next_random_color();
        game.sequence_length++;
        game.input_index = 0;
        game.state = DISPLAY_SEQUENCE;
        last_color = NONE;

        HAL_Delay(250); // Wait before displaying sequence
        return;
      }

      // 2. Get button pressed by user
      ColorIndex color = get_button_input(0);
      if (color == last_color)
        return;

      // 3. Case 1: Button was released (COLOR -> NONE), continue sequence.
      if (last_color != NONE && color == NONE) {
        set_color(&colors[last_color], 0);
        game.input_index++;
      }
      // 4. Case 2: Button was pressed (NONE -> COLOR).
      else if (last_color == NONE && color != NONE) {

        // 5 If button does not match sequence, fail IMMEDIATELY.
        if (color != game.sequence[game.input_index]) {
          game.state = GAME_OVER;
          last_color = NONE;

          uart_print(UART_MSG("Game over."));
          return;
        }
        // 6. Otherwise, display input.
        else {
          set_color(&colors[color], 1);
        }
      }

      // 7. Set last button
      last_color = color;
      break;
    }
    case GAME_OVER:
      play_tone(130);
      Color *correct_color = &colors[game.sequence[game.input_index]];

      for (int i = 0; i < 6; i ++) {
        HAL_GPIO_TogglePin(GPIOA, correct_color->light_pin);
        HAL_Delay(250);
      }
      stop_tone();
      game.state = IDLE;
      break;
    case IDLE:
      break;
  }
}

uint8_t get_button_input(uint8_t reset) {
  static uint8_t stable_button = NONE;
  static uint8_t candidate_button = NONE;
  static uint32_t last_debounce_time = 0;

  uint8_t current_button = NONE;

  // Clear static variables on game start
  if (reset) {
    stable_button = NONE;
    candidate_button = NONE;
    last_debounce_time = 0;
    return NONE;
  }

  // Read buttons
  for (uint8_t i = 0; i < 4; i++) {
    if (HAL_GPIO_ReadPin(GPIOC, colors[i].button_pin) == GPIO_PIN_RESET) {
      current_button = i;
      break;
    }
  }

  if (current_button != candidate_button) {
    candidate_button = current_button;
    last_debounce_time = HAL_GetTick();
  }

  if ((HAL_GetTick() - last_debounce_time) >= DEBOUNCE_TIME) {
    stable_button = candidate_button;
  }

  return stable_button;
}

void uart_print(char* msg) {
  uint8_t len = strlen(msg);
  HAL_UART_Transmit(&huart2, (uint8_t*)msg, len, HAL_MAX_DELAY  );
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

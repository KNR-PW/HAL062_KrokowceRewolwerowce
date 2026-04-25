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
#include "fdcan.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
volatile int Xready = 1;
volatile int Yready = 1;
volatile int Xsteps = 0;
volatile int Ysteps = 0;
volatile int Xupdate = 0;
volatile int Yupdate = 0;
int q = 0;
const int defaultPeriod = 80;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
int absol(int fk)
{
  if(fk < 0)
  {
    return -fk;
  }
  return fk;
}
void moveX(int steps, int stepPeriod)//step period podawany w 1/10 ms
{
  if(Xready == 0||steps == 0)
  {
    return;
  }
  if(steps > 0)
  {
    HAL_GPIO_WritePin(Xdir_GPIO_Port, Xdir_Pin, GPIO_PIN_RESET);
  }
  else 
  {
    HAL_GPIO_WritePin(Xdir_GPIO_Port, Xdir_Pin, GPIO_PIN_SET);
  }
  Xupdate = steps;
  Xready = 0;
  __HAL_TIM_SET_AUTORELOAD(&htim2, absol(steps));
  if(stepPeriod < 4)
  {
    stepPeriod = defaultPeriod;
  }
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3,  stepPeriod/2-1);
  __HAL_TIM_SET_AUTORELOAD(&htim1, stepPeriod-1);
  HAL_TIM_Base_Start_IT(&htim2);
  HAL_TIM_PWM_Start_IT(&htim1, TIM_CHANNEL_3);
}
void moveY(int steps, int stepPeriod)//step period podawany w 1/50 ms
{
  if(Yready == 0||steps == 0)
  {
    return;
  }
  if(steps > 0)
  {
    HAL_GPIO_WritePin(Ydir_GPIO_Port, Ydir_Pin, GPIO_PIN_RESET);
  }
  else 
  {
    HAL_GPIO_WritePin(Ydir_GPIO_Port, Ydir_Pin, GPIO_PIN_SET);
  }
  Yupdate = steps;
  Yready = 0;
  __HAL_TIM_SET_AUTORELOAD(&htim15, absol(steps));
  if(stepPeriod < 4)
  {
    stepPeriod = defaultPeriod;
  }
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2,  stepPeriod/2-1);
  __HAL_TIM_SET_AUTORELOAD(&htim3, stepPeriod-1);
  HAL_TIM_Base_Start_IT(&htim15);
  HAL_TIM_PWM_Start_IT(&htim3, TIM_CHANNEL_2);
}
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
  MX_TIM1_Init();
  MX_TIM3_Init();
  MX_TIM2_Init();
  MX_TIM15_Init();
  MX_FDCAN1_Init();
  /* USER CODE BEGIN 2 */
  // ONFIGURACJA FILTRA CAN (Sprzętowy bramkarz) ---
    FDCAN_FilterTypeDef sFilterConfig;

    sFilterConfig.IdType = FDCAN_STANDARD_ID;
    sFilterConfig.FilterIndex = 0;
    sFilterConfig.FilterType = FDCAN_FILTER_MASK;
    sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;

    sFilterConfig.FilterID1 = 0x000;
    sFilterConfig.FilterID2 = 0x000;

    if (HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig) != HAL_OK)
    {
        // Błąd konfiguracji filtra (warto tu wstawić np. mignięcie diodą błędu)
    }

    
HAL_FDCAN_ConfigGlobalFilter(&hfdcan1,
    FDCAN_ACCEPT_IN_RX_FIFO0,
    FDCAN_ACCEPT_IN_RX_FIFO0,
    FDCAN_REJECT_REMOTE,
    FDCAN_REJECT_REMOTE
);

HAL_FDCAN_Start(&hfdcan1);
    HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
    FDCAN_TxHeaderTypeDef TxHeader;
uint8_t TxData[8] = {1,2,3,4,5,6,7,8};

// konfiguracja nagłówka
TxHeader.Identifier = 0x123;
TxHeader.IdType = FDCAN_STANDARD_ID;
TxHeader.TxFrameType = FDCAN_DATA_FRAME;
TxHeader.DataLength = FDCAN_DLC_BYTES_8;
TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
TxHeader.BitRateSwitch = FDCAN_BRS_OFF;   // classic CAN
TxHeader.FDFormat = FDCAN_CLASSIC_CAN;    // nie CAN FD
TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
TxHeader.MessageMarker = 0;

  HAL_GPIO_WritePin(Xdir_GPIO_Port, Xdir_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(Ydir_GPIO_Port, Ydir_Pin, GPIO_PIN_RESET);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    if (HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan1) > 0)
  {
      HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);

      HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, TxData);
      HAL_Delay(1);
  }
  //HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, TxData);

  HAL_Delay(1000);
    /*
    if(Xready == 1&&Yready==1)
    {
      HAL_Delay(1000);
      switch(q)
      {
        case 0:
        {
          moveX(400, 50);
          moveY(400, 50);
          break;
        }
        case 1:
        {
          moveX(400, 10);
          moveY(-600, 10);
          break;
        }
        case 2:
        {
          moveX(400, 25);
          moveY(200, 25);
          break;
        }
      }
      q++;
      if(q == 3)
      {
        q = 0;
      }   
    }
      */
    
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  __HAL_FLASH_SET_LATENCY(FLASH_LATENCY_0);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV4;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim == &htim2) {
    HAL_TIM_PWM_Stop_IT(&htim1, TIM_CHANNEL_3);  
    HAL_TIM_Base_Stop_IT(&htim2);
    Xsteps+=Xupdate;
    Xupdate = 0;
    Xready = 1;
  }
  if (htim == &htim15) {
    HAL_TIM_PWM_Stop_IT(&htim3, TIM_CHANNEL_2);  
    HAL_TIM_Base_Stop_IT(&htim15);
    Ysteps+=Yupdate;
    Yupdate = 0;
    Yready = 1;
    HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
  }
  
}
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
    if (RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE)
    {
        FDCAN_RxHeaderTypeDef RxHeader;
        uint8_t RxData[8];
        HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);

        HAL_FDCAN_GetRxMessage(hfdcan,
            FDCAN_RX_FIFO0,
            &RxHeader,
            RxData);
    }
    HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
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

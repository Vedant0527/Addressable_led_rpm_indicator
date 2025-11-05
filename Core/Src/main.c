/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "WS2812_SPI.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define NUM_LED    8
//#define USE_BRIGHTNESS 1

// New RPM ranges for green->blue->red flow
#define RPM_GREEN_MAX       800    // 4 green LEDs (0-800 RPM)
#define RPM_TRANSITION1     1200   // 2 transition LEDs (800-1200 RPM)
#define RPM_BLUE_MAX        2000   // 4 blue LEDs (1200-2000 RPM)
#define RPM_TRANSITION2     2400   // 2 transition LEDs (2000-2400 RPM)
#define RPM_RED_MAX         3200   // 4 red LEDs (2400-3200 RPM)
#define RPM_MAX_LIMIT       3300   // Max RPM limit
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
//int LED_Data[NUM_LED][3];  // [LED_index][GRB]
int brightness = 50;       // 0-100

uint32_t ledOldMillis = 0, LednewMillis = 0;
const uint32_t Blink = 300;  // Faster blink for max RPM warning
bool isled = false, iled = false;
int rpm = 0;
char msg[100];
// Gradual RPM generation variables
uint32_t rpmUpdateTime = 0;
const uint32_t RPM_UPDATE_INTERVAL = 500;  // Update RPM every 500ms
const int RPM_INCREMENT = 50;              // Increase by 50 RPM each update
bool rpmDirection = true;                  // true = increasing, false = decreasing

// Buffer for sprintf

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */



void handleRPMDisplay(void)
{
    LednewMillis = HAL_GetTick();

    for (int i = NUM_LED; i > NUM_LED; i++) {
            setLED(i, 0, 0, 0);
        }

    // Clear all LEDs
    for (int i = 0; i < NUM_LED; i++) {
        setLED(i, 0, 0, 0);
    }

    // RPM above max threshold: Blink all red
    if (rpm > RPM_RED_MAX) {
        if (LednewMillis - ledOldMillis >= Blink) {
            ledOldMillis = LednewMillis;
            iled = !iled;
        }
        for (int i = 0; i < NUM_LED; i++) {
            if (iled) setLED(i, 255, 0, 0);  // Red on
            else      setLED(i, 0, 0, 0);    // Off
        }
        WS2812_Send();
        return;
    }

    // 0 - 800 RPM: 4 Green LEDs
    if (rpm <= RPM_GREEN_MAX) {
        for (int i = 0; i < 2; i++) {
            setLED(i, 0, 0, 255);  // Green
        }
    }

    // 801 - 1200 RPM: 4 Green + 2 Orange (transition)
    else if (rpm <= RPM_TRANSITION1) {
//        for (int i = 0; i < 4; i++) {
//            setLED(i, 0, 180, 0);  // Green
//        }
        for (int i = 0; i < 2; i++) {
            setLED(i, 0, 0, 255);  // Green
        }

        for (int i = 2; i < 3; i++) {
            setLED(i, 255, 100, 0);  // Orange
        }
    }

    // 1201 - 2000 RPM: 4 Blue LEDs
    else if (rpm <= RPM_BLUE_MAX) {
        for (int i = 0; i < 2; i++) {
            setLED(i, 0, 0, 255);  // Green
        }

        for (int i = 2; i < 3; i++) {
            setLED(i, 255, 100, 0);  // Orange
        }
        for (int i = 3; i < 5; i++) {
            setLED(i, 0, 255, 0);  // Blue
        }
    }

     //2001 - 2400 RPM: 4 Blue + 2 Purple (transition)
    else if (rpm <= RPM_TRANSITION2) {
            for (int i = 0; i < 2; i++) {
                setLED(i, 0, 0, 255);  // Green
            }

            for (int i = 2; i < 3; i++) {
                setLED(i, 255, 100, 0);  // Orange
            }
            for (int i = 3; i < 5; i++) {
                setLED(i, 0, 255, 0);  // Blue
            }
        for (int i = 5; i < 6; i++) {
            setLED(i, 255, 0, 255);  // Purple/Magenta
        }
    }

    // 2401 - 3200 RPM: 4 Red LEDs
    else if (rpm <= RPM_RED_MAX) {
    	for (int i = 0; i < 2; i++) {
    	                setLED(i, 0, 0, 255);  // Green
    	            }

    	            for (int i = 2; i < 3; i++) {
    	                setLED(i, 255, 100, 0);  // Orange
    	            }
    	            for (int i = 3; i < 5; i++) {
    	                setLED(i, 0, 255, 0);  // Blue
    	            }
    	        for (int i = 5; i < 6; i++) {
    	            setLED(i, 255, 0, 255);  // Purple/Magenta
    	        }
        for (int i = 6; i < 8; i++) {
            setLED(i, 255, 0, 0);  // Red
        }
    }



    // Send the LED data
    WS2812_Send();
}

//void handleRPMDisplay(void)
//{
//    LednewMillis = HAL_GetTick();
//
//    // Clear all LEDs
//    for (int i = 0; i < NUM_LED; i++) {
//        setLED(i, 0, 0, 0);
//    }
//
//    // RPM above max threshold: Blink all red
//    if (rpm > RPM_RED_MAX) {
//        if (LednewMillis - ledOldMillis >= Blink) {
//            ledOldMillis = LednewMillis;
//            iled = !iled;
//        }
//        for (int i = 0; i < NUM_LED; i++) {
//            if (iled) setLED(i, 255, 0, 0);  // Red on
//            else      setLED(i, 0, 0, 0);    // Off
//        }
//        WS2812_Send();
//        return;
//    }
//
//    // 0 - 800 RPM: 4 Green LEDs
//    if (rpm <= RPM_GREEN_MAX) {
//        for (int i = 0; i < 4; i++) {
//            setLED(i, 0, 255, 0);  // Green
//        }
//    }
//
//    // 801 - 1200 RPM: 4 Green + 2 Orange (transition)
//    else if (rpm <= RPM_TRANSITION1) {
//        for (int i = 0; i < 4; i++) {
//            setLED(i, 0, 255, 0);  // Green
//        }
//        for (int i = 4; i < 6; i++) {
//            setLED(i, 255, 100, 0);  // Orange
//        }
//    }
//
//    // 1201 - 2000 RPM: 4 Blue LEDs
//    else if (rpm <= RPM_BLUE_MAX) {
//        for (int i = 2; i < 6; i++) {
//            setLED(i, 0, 0, 255);  // Blue
//        }
//    }
//
//    // 2001 - 2400 RPM: 4 Blue + 2 Purple (transition)
//    else if (rpm <= RPM_TRANSITION2) {
//        for (int i = 2; i < 6; i++) {
//            setLED(i, 0, 0, 255);  // Blue
//        }
//        for (int i = 6; i < 8; i++) {
//            setLED(i, 255, 0, 255);  // Purple/Magenta
//        }
//    }
//
//    // 2401 - 3200 RPM: 4 Red LEDs
//    else if (rpm <= RPM_RED_MAX) {
//        for (int i = 4; i < 8; i++) {
//            setLED(i, 255, 0, 0);  // Red
//        }
//    }
//
//    // Send the LED data
//    WS2812_Send();
//}


void generateGradualRPM(void)
{
    uint32_t currentTime = HAL_GetTick();

    if (currentTime - rpmUpdateTime >= RPM_UPDATE_INTERVAL) {
        rpmUpdateTime = currentTime;

        if (rpmDirection) {
            // Increasing RPM
            rpm += RPM_INCREMENT;
            if (rpm >= RPM_MAX_LIMIT) {  // Go up to 3400 to test max blink
                rpmDirection = false;  // Start decreasing
            }
        } else {
            // Decreasing RPM
            rpm -= RPM_INCREMENT;
            if (rpm <= 0) {
                rpm = 0;
                rpmDirection = true;  // Start increasing again
            }
        }
    }
}

        // Print RPM value using sprintf
//        sprintf(rpmBuffer, "Current RPM: %d | Direction: %s | Status: ",
//                rpm, rpmDirection ? "Increasing" : "Decreasing");
//
//        // Add status based on RPM range
//        if (rpm > RPM_RED_MAX) {
//            strcat(rpmBuffer, "MAX RPM - BLINKING RED!\r\n");
//        } else if (rpm > RPM_TRANSITION2) {
//            strcat(rpmBuffer, "Red Zone\r\n");
//        } else if (rpm > RPM_BLUE_MAX) {
//            strcat(rpmBuffer, "Blue-Red Transition\r\n");
//        } else if (rpm > RPM_TRANSITION1) {
//            strcat(rpmBuffer, "Blue Zone\r\n");
//        } else if (rpm > RPM_GREEN_MAX) {
//            strcat(rpmBuffer, "Green-Blue Transition\r\n");
//        } else {
//            strcat(rpmBuffer, "Green Zone\r\n");
//        }
//
//        // Send via UART1 (fixed from huart2)
//        HAL_UART_Transmit(&huart1, (uint8_t*)rpmBuffer, strlen(rpmBuffer), 1000);
    }
}

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
  MX_SPI1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  // Initialize all LEDs to off
  for (int i = 0; i < NUM_LED; i++) {
      setLED(i, 0, 0, 0);
  }
  WS2812_Send();

  // Start with 0 RPM
//  rpm = 0;

  // Send startup message (fixed from huart2)
//  sprintf(rpmBuffer, "RPM LED Display Started!\r\nRPM Range: 0-%d\r\n", RPM_MAX_LIMIT);
//  HAL_UART_Transmit(&huart1, (uint8_t*)rpmBuffer, strlen(rpmBuffer), 1000);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // Generate gradual RPM from 0 to 3400 and back to 0
    generateGradualRPM();

    // Handle LED display based on RPM

    sprintf(msg, "RPM = %d \r\n", rpm);
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    handleRPMDisplay();


    // Small delay for smooth operation
    HAL_Delay(50);

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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 160;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_1LINE;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

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
  /* USER CODE BEGIN MX_GPIO_Init_1 */
  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

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

/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    quadspi.c
  * @brief   This file provides code for the configuration
  *          of the QUADSPI instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "quadspi.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

QSPI_HandleTypeDef hqspi;

/* QUADSPI init function */
void MX_QUADSPI_Init(void)
{

  /* USER CODE BEGIN QUADSPI_Init 0 */

  /* USER CODE END QUADSPI_Init 0 */

  /* USER CODE BEGIN QUADSPI_Init 1 */

  /*
  hqspi.Init.ClockPrescaler = 9; // 160M / (ClockPrescaler + 1) = 40M
  hqspi.Init.ClockPrescaler = 1; // 160M / (ClockPrescaler + 1) = 40M
  */
  
  /* USER CODE END QUADSPI_Init 1 */
  hqspi.Instance = QUADSPI;
  hqspi.Init.ClockPrescaler = 3;
  hqspi.Init.FifoThreshold = 32;
  hqspi.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_HALFCYCLE;
  hqspi.Init.FlashSize = 24;
  hqspi.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_2_CYCLE;
  hqspi.Init.ClockMode = QSPI_CLOCK_MODE_0;
  hqspi.Init.FlashID = QSPI_FLASH_ID_1;
  hqspi.Init.DualFlash = QSPI_DUALFLASH_DISABLE;
  if (HAL_QSPI_Init(&hqspi) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN QUADSPI_Init 2 */

  /* USER CODE END QUADSPI_Init 2 */

}

void HAL_QSPI_MspInit(QSPI_HandleTypeDef* qspiHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if(qspiHandle->Instance==QUADSPI)
  {
  /* USER CODE BEGIN QUADSPI_MspInit 0 */

  /* USER CODE END QUADSPI_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_QSPI;
    PeriphClkInitStruct.QspiClockSelection = RCC_QSPICLKSOURCE_PLL;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* QUADSPI clock enable */
    __HAL_RCC_QSPI_CLK_ENABLE();

    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    /**QUADSPI GPIO Configuration
    PE2     ------> QUADSPI_BK1_IO2
    PB2     ------> QUADSPI_CLK
    PD11     ------> QUADSPI_BK1_IO0
    PD12     ------> QUADSPI_BK1_IO1
    PD13     ------> QUADSPI_BK1_IO3
    PG6     ------> QUADSPI_BK1_NCS
    */
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_QUADSPI;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /* USER CODE BEGIN QUADSPI_MspInit 1 */

  /* USER CODE END QUADSPI_MspInit 1 */
  }
}

void HAL_QSPI_MspDeInit(QSPI_HandleTypeDef* qspiHandle)
{

  if(qspiHandle->Instance==QUADSPI)
  {
  /* USER CODE BEGIN QUADSPI_MspDeInit 0 */

  /* USER CODE END QUADSPI_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_QSPI_CLK_DISABLE();

    /**QUADSPI GPIO Configuration
    PE2     ------> QUADSPI_BK1_IO2
    PB2     ------> QUADSPI_CLK
    PD11     ------> QUADSPI_BK1_IO0
    PD12     ------> QUADSPI_BK1_IO1
    PD13     ------> QUADSPI_BK1_IO3
    PG6     ------> QUADSPI_BK1_NCS
    */
    HAL_GPIO_DeInit(GPIOE, GPIO_PIN_2);

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_2);

    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13);

    HAL_GPIO_DeInit(GPIOG, GPIO_PIN_6);

  /* USER CODE BEGIN QUADSPI_MspDeInit 1 */

  /* USER CODE END QUADSPI_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
static uint32_t qspi_lineMode = QSPI_INSTRUCTION_1_LINE;
void setQSPI_Line_Mode(uint32_t lineMode)
{
    qspi_lineMode = lineMode;
}

//QSPI??��?�命�??
//instruction:要�?��?��?��?�令
//address:??��?�到??�目??�地??
//dummyCycles:空�?�令?��??�数
//	instructionMode:??�令模�??;QSPI_INSTRUCTION_NONE,QSPI_INSTRUCTION_1_LINE,QSPI_INSTRUCTION_2_LINE,QSPI_INSTRUCTION_4_LINE  
//	addressMode:?��??模�??; QSPI_ADDRESS_NONE,QSPI_ADDRESS_1_LINE,QSPI_ADDRESS_2_LINE,QSPI_ADDRESS_4_LINE
//	addressSize:?��???���??;QSPI_ADDRESS_8_BITS,QSPI_ADDRESS_16_BITS,QSPI_ADDRESS_24_BITS,QSPI_ADDRESS_32_BITS
//	dataMode:?��?��模�??; QSPI_DATA_NONE,QSPI_DATA_1_LINE,QSPI_DATA_2_LINE,QSPI_DATA_4_LINE

HAL_StatusTypeDef QSPI_Send_CMD(uint32_t instruction,uint32_t address,uint32_t dummyCycles,uint32_t instructionMode,uint32_t addressMode,uint32_t addressSize,uint32_t dataMode)
{
    QSPI_CommandTypeDef Cmdhandler;
    
    Cmdhandler.Instruction=instruction;                 	//??�令
    Cmdhandler.Address=address;                            	//?��??
    Cmdhandler.DummyCycles=dummyCycles;                     //设置空�?�令?��??�数
    Cmdhandler.InstructionMode=instructionMode;				//??�令模�??
    Cmdhandler.AddressMode=addressMode;   					//?��??模�??
    Cmdhandler.AddressSize=addressSize;   					//?��???���??
    Cmdhandler.DataMode=dataMode;             				//?��?��模�??
    Cmdhandler.SIOOMode=QSPI_SIOO_INST_EVERY_CMD;       	//每次?��??��?��?�令
    Cmdhandler.AlternateByteMode=QSPI_ALTERNATE_BYTES_NONE; //??�交?��字�??
    Cmdhandler.DdrMode=QSPI_DDR_MODE_DISABLE;           	//?��?��DDR模�??
    Cmdhandler.DdrHoldHalfCycle=QSPI_DDR_HHC_ANALOG_DELAY;

    return HAL_QSPI_Command(&hqspi,&Cmdhandler,5000);
}

//QSPI?��?��??��?�长度�?�数?��
//buf:?��?��?��?��缓冲?��首地??
//datalen:要�?��?��?�数?��?���??
//返�?��??:0,�??�??
//    ?���??,??�误�?????
HAL_StatusTypeDef QSPI_Receive(uint8_t* buf,uint32_t datalen)
{
    hqspi.Instance->DLR=datalen-1;                           //??�置?��?��?���??
    return HAL_QSPI_Receive(&hqspi,buf,5000);
}

//QSPI??��?��?��?�长度�?�数?��
//buf:??��?�数?��缓冲?��首地??
//datalen:要�?��?��?�数?��?���??
//返�?��??:0,�??�??
//    ?���??,??�误�?????
HAL_StatusTypeDef QSPI_Transmit(uint8_t* buf,uint32_t datalen)
{
    hqspi.Instance->DLR=datalen-1;                            //??�置?��?��?���??
    return HAL_QSPI_Transmit(&hqspi,buf,5000);
}

void setQspiPara(uint32_t ClockPrescaler, uint32_t ClockMode)
{
    QSPI_HandleTypeDef *phqspi = &hqspi;

    __HAL_QSPI_ENABLE(phqspi);

    /* Configure QSPI Clock Prescaler and Sample Shift */
    MODIFY_REG(phqspi->Instance->CR, QUADSPI_CR_PRESCALER, (phqspi->Init.ClockPrescaler << QUADSPI_CR_PRESCALER_Pos));

    /* Configure QSPI Flash Size, CS High Time and Clock Mode */
    MODIFY_REG(phqspi->Instance->DCR, QUADSPI_DCR_CKMODE, phqspi->Init.ClockMode);

    /* Enable the QSPI peripheral */
    __HAL_QSPI_ENABLE(phqspi);
}

/* USER CODE END 1 */

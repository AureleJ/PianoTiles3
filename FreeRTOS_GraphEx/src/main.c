/**
 ******************************************************************************
 * @file    main.c
 * @author  MCD Application Team
 * @version V1.0.1
 * @date    26-February-2014
 * @brief   This file provides main program functions
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
 *
 * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *        http://www.st.com/software_license_agreement_liberty_v2
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************************
 */
/* Mise a jour du 31/12/2018 : P. Foubet
 * Mise a jour du 15/02/2018 : F. Crison ( update Freertos et librairies)
 */
/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "interfaces.h"
#include "display.h"
#include "screen.h"
#include "stm32f4xx_hal.h"
#include <stdio.h>
#include "GUI.h"

#define MODE 0 // 0 = graphes, 1 = console, 2 = Console+Trace
#define STACK_SIZE 128 // A ajuster !!

TIM_HandleTypeDef htim3;

/* sleep en millisecondes */
void msleep(uint32_t ms)
{
	vTaskDelay(pdMS_TO_TICKS(ms));
}

void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == TIM3)
    {
        GPIO_InitTypeDef GPIO_InitStruct = {0};

        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_TIM3_CLK_ENABLE();

        GPIO_InitStruct.Pin = GPIO_PIN_4;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    }
}

void User_Buttons_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_14;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
}

void User_PWM_Buzzer_Init(void)
{
    TIM_OC_InitTypeDef sConfigOC = {0};

    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 90 - 1;
    htim3.Init.Period = 500 - 1;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_PWM_Init(&htim3);

    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 250;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1);
}

/* Threads functions ---------------------------------------------------------*/
#define NLMIN 5  // No ligne de depart
#define NLMAX 35 // No de ligne maxi. de l'ecran
#define HLPIX 9  // Nb de pixels hauteur font
void affTexte(char *m)
{
	static int l = NLMIN;
	GUI_DispStringAtCEOL(" ", 0, l * HLPIX);
	GUI_DispStringAtCEOL(m, 0, l * HLPIX);
	l++;
	if (l == NLMAX)
		l = NLMIN;
}



void Screen_Thread(void *p)
{
	int XScreenSize   = LCD_GetXSize();
	int YScreenSize   = LCD_GetYSize();
	int ui_box_width  = XScreenSize / 2;
	int ui_box_height = YScreenSize / 7;

	GUI_Clear();
	display_init_screen();

	DrawNote_t val;
	val.piste_num = 1;
	val.height = 30;

	val.y= (ui_box_height + 1) - val.height;

	int score = 67;
	int last_score = 0;

    while (1)
    {
    	clear_note(val);

		val.y += 2;

		if (val.y >= YScreenSize - 3) {
			val.y = (ui_box_height + 1) - val.height;
			val.piste_num = (val.piste_num + 1) % 4;
		}

		display_note(val, val.y);

		Draw_note_line();
		Draw_bottom_line();

		if (score != last_score)
		{
			display_score(score);
			last_score = score;
		}

		msleep(20);
    }
}

void Cursor_Thread(void *p)
{
	while (1)
	{
		if (MODE)
			msleep(1000); // Toutes les secondes
		else
			msleep(50);   /* Cadence de lecture idéale de 50 ms */

		uint8_t btn1 = (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_2) == GPIO_PIN_SET);
		uint8_t btn2 = (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_3) == GPIO_PIN_SET);
		uint8_t btn3 = (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_4) == GPIO_PIN_SET);
		uint8_t btn4 = (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_5) == GPIO_PIN_SET);

		if (btn1 || btn2 || btn3 || btn4)
		{
			HAL_GPIO_WritePin(GPIOG, GPIO_PIN_14, GPIO_PIN_SET);
			HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
		}
		else
		{
			HAL_GPIO_WritePin(GPIOG, GPIO_PIN_14, GPIO_PIN_RESET);
			HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
		}

		/* Capture input event and update cursor */
		if (GUI_Initialized == 1)
		{
			BSP_Pointer_Update();
			if (MODE == 2)
				affTexte("Cursor update !\n");
		}
	}
}


/**
 * @brief  Main program.
 */
int main(void)
{

	HAL_Init();

	interfaces_init();
	User_Buttons_Init();
	User_PWM_Buzzer_Init();

	GUI_Init();
	GUI_Initialized = 1;

	/* Creation des threads */
	if (!(pdPASS == xTaskCreate(Cursor_Thread, "Cursor", STACK_SIZE * 4, NULL, 1, NULL)))
		goto err;
	if (!(pdPASS == xTaskCreate(Screen_Thread, "Screen", STACK_SIZE * 4, NULL, 1, NULL)))
			goto err;

	/* on lance le systeme ! */
	vTaskStartScheduler();

	err:              // En principe jamais atteint !
	while (1);

	return 0;
}

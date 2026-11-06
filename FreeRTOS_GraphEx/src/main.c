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
#include "queue.h"
#include "interfaces.h"
#include "display.h"
#include "screen.h"
#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <stdlib.h>
#include "GUI.h"
#include <time.h>

#define MODE 0 // 0 = graphes, 1 = console, 2 = Console+Trace
#define STACK_SIZE 128 // A ajuster !!
#define UI_BUFF_SIZE 32

TIM_HandleTypeDef htim3;

QueueHandle_t xNotesQueue;
QueueHandle_t xPressedPisteQueue;
SemaphoreHandle_t xScoreSemaphore = NULL;

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

    GPIO_InitStruct.Pin = GPIO_PIN_13;
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

typedef enum {
    DO    = 262,
    DO_D  = 277,
    RE    = 294,
    RE_D  = 311,
    MI    = 330,
    FA    = 349,
    FA_D  = 370,
    SOL   = 392,
    SOL_D = 415,
    LA    = 440,
    LA_D  = 466,
    SI    = 494
} PITCHES;

typedef enum {
    CROCHE          = 300,
    NOIRE           = 600,
    NOIRE_POINTEE   = 900,
    BLANCHE         = 1200,
    BLANCHE_POINTEE = 1800,
    RONDE           = 2400
} TEMPOS;

typedef struct {
    uint32_t time_ms;
    uint16_t duration_ms;
    PITCHES  pitch;
    uint8_t pist_num;
} note_t;

const note_t PARTITION[] =
{
    /* Frère Jacques, Frère Jacques */
    {     0, NOIRE,   DO,  0 },
    {   600, NOIRE,   RE,  1 },
    {  1200, NOIRE,   MI,  2 },
    {  1800, NOIRE,   DO,  3 },
    {  2400, NOIRE,   DO,  0 },
    {  3000, NOIRE,   RE,  1 },
    {  3600, NOIRE,   MI,  2 },
    {  4200, NOIRE,   DO,  3 },

    /* Dormez-vous ? Dormez-vous ? */
    {  4800, NOIRE,   MI,  0 },
    {  5400, NOIRE,   FA,  1 },
    {  6000, BLANCHE, SOL, 2 },
    {  7200, NOIRE,   MI,  3 },
    {  7800, NOIRE,   FA,  0 },
    {  8400, BLANCHE, SOL, 1 },

    /* Sonnez les matines, sonnez les matines */
    {  9600,  CROCHE, SOL, 2 },
    {  9900,  CROCHE, LA,  3 },
    { 10200,  CROCHE, SOL, 0 },
    { 10500,  CROCHE, FA,  1 },
    { 10800,  CROCHE, MI,  2 },
    { 11100,  CROCHE, DO,  3 },
    { 11400,  CROCHE, MI,  0 },
    { 11700,  CROCHE, DO,  1 },
    { 12000,  CROCHE, SOL, 2 },
    { 12300,  CROCHE, LA,  3 },
    { 12600,  CROCHE, SOL, 0 },
    { 12900,  CROCHE, FA,  1 },
    { 13200,  CROCHE, MI,  2 },
    { 13500,  CROCHE, DO,  3 },
    { 13800,  CROCHE, MI,  0 },
    { 14100,  CROCHE, DO,  1 },

    /* Ding, Dang, Dong ! Ding, Dang, Dong ! */
    { 14400, NOIRE,   DO,  2 },
    { 15000, NOIRE,   SOL, 3 },
    { 15600, BLANCHE, DO,  0 },
    { 16800, NOIRE,   DO,  1 },
    { 17400, NOIRE,   SOL, 2 },
    { 18000, BLANCHE, DO,  3 },
};

typedef struct {
	uint8_t pist_num;
	uint8_t is_failed;
} PressedPiste_t;

typedef struct {
    DrawNote_t note;
    int active;
} UiSlot_t;

#define PARTITION_LENGTH (sizeof(PARTITION) / sizeof(PARTITION[0]))
#define PITCH_COUNT (sizeof(PITCHES) / sizeof(PITCHES[0]))

#define TIMER_TICK_HZ 1000000UL
#define BUZZER_DUTY_PERCENT 25

void Buzzer_SetFrequency(uint16_t freq_hz)
{
    if (freq_hz == 0)
    {
        HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
        return;
    }

    uint32_t arr = (TIMER_TICK_HZ / freq_hz) - 1;
    uint32_t ccr = (arr * BUZZER_DUTY_PERCENT) / 100;

    __HAL_TIM_SET_AUTORELOAD(&htim3, arr);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, ccr);
    __HAL_TIM_SET_COUNTER(&htim3, 0);

    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
}

static int protectedScore = 0;

void get_score(int *score)
{
	if( xSemaphoreTake(xScoreSemaphore, portMAX_DELAY) == pdTRUE )
	{
		*score = protectedScore;
		xSemaphoreGive(xScoreSemaphore);
	}
}

void set_score(int score)
{
	if( xSemaphoreTake(xScoreSemaphore, portMAX_DELAY) == pdTRUE )
	{
		protectedScore = score;
		xSemaphoreGive(xScoreSemaphore);
	}
}

#define FALL_DURATION_MS 5000
#define HIT_TOLERANCE_MS 250
#define HIT_LINE_OFFSET 40
#define SONG_GAP_MS 1000

#define FALL_TICKS pdMS_TO_TICKS(FALL_DURATION_MS)

#define BUFF_SIZE 32
#define NUM_PISTES 4

typedef struct {
    uint8_t active;
    uint8_t piste_num;
    uint32_t partitionIndex;
    TickType_t hit_tick;
} BuffNote_t;

static inline int32_t tick_diff(TickType_t a, TickType_t b)
{
    return (int32_t)(a - b);
}

void Screen_Thread(void *p)
{
    int YScreenSize = LCD_GetYSize();

    GUI_Clear();
    display_init_screen();

    const int topY = YScreenSize / 7 + 1;
    const int hitY = YScreenSize - HIT_LINE_OFFSET;
    const int travel = hitY - topY;
    const int32_t fallTicks = (int32_t)FALL_TICKS;

    UiSlot_t slots[UI_BUFF_SIZE];
    for (int i = 0; i < UI_BUFF_SIZE; i++)
        slots[i].active = 0;

    int last_score = -1;
    TickType_t xLastWake = xTaskGetTickCount();

    while (1)
    {
        TickType_t now = xTaskGetTickCount();

        DrawNote_t temp;
        while (xQueueReceive(xNotesQueue, &temp, 0) == pdPASS)
        {
            for (int i = 0; i < UI_BUFF_SIZE; i++)
            {
                if (!slots[i].active)
                {
                    slots[i].note = temp;
                    int h = (int)((int32_t)pdMS_TO_TICKS(temp.duration_ms) * travel / fallTicks) - 2;
                    if (h < 3) h = 3;
                    slots[i].note.height = h;
                    slots[i].note.y = topY - h;
                    slots[i].active = 1;
                    break;
                }
            }
        }

        PressedPiste_t temp2;
        int max = 0;
        int index = 0;
        int check = 0;

		while (xQueueReceive(xPressedPisteQueue, &temp2, 0) == pdPASS)
		{
			for (int i = 0; i < UI_BUFF_SIZE; i++)
			{
				if(!slots[i].active) continue;

				if (!temp2.is_failed && slots[i].note.piste_num == temp2.pist_num)
				{
					if (slots[i].note.y > max) {
						max = slots[i].note.y;
						index = i;
						check = 1;
					}
				}
			}

			if (check)
			{
				slots[index].note.pressed = true;
				check = 0;
			}
		}

        for (int i = 0; i < UI_BUFF_SIZE; i++)
        {
            if (slots[i].active)
                clear_note(slots[i].note);
        }

        for (int i = 0; i < UI_BUFF_SIZE; i++)
        {
            if (!slots[i].active)
                continue;

            int32_t elapsed = tick_diff(now, slots[i].note.spawn_ms);
            int yBottom = topY + (int)(elapsed * travel / fallTicks);
            slots[i].note.y = yBottom - slots[i].note.height;

            if (slots[i].note.y >= YScreenSize - 3)
            {
                slots[i].active = 0;
            }
            else
            {
                display_note(slots[i].note, slots[i].note.y);
            }
        }

        Draw_note_line();
        Draw_bottom_line();

        int s;
        get_score(&s);
        if (s != last_score)
        {
            display_score(s);
            last_score = s;
        }

        vTaskDelayUntil(&xLastWake, pdMS_TO_TICKS(20));
    }
}

void Engine_Thread(void *p)
{
    BuffNote_t buff[BUFF_SIZE];
    for (int i = 0; i < BUFF_SIZE; i++)
        buff[i].active = 0;

    uint8_t previousBtn[NUM_PISTES] = {0};

    const TickType_t tolTicks = pdMS_TO_TICKS(HIT_TOLERANCE_MS);
    const TickType_t songTicks = pdMS_TO_TICKS(PARTITION[PARTITION_LENGTH - 1].time_ms + PARTITION[PARTITION_LENGTH - 1].duration_ms + SONG_GAP_MS);

    TickType_t xLastWakeTime = xTaskGetTickCount();
    TickType_t songStart = xTaskGetTickCount();
    uint32_t noteIndex = 0;

    uint8_t buzzerOn = 0;
    TickType_t buzzerStop = 0;

    while (1)
    {
        TickType_t now = xTaskGetTickCount();

        while (noteIndex < PARTITION_LENGTH)
        {
            TickType_t spawn = songStart + pdMS_TO_TICKS(PARTITION[noteIndex].time_ms);
            if (tick_diff(now, spawn) < 0)
                break;

            DrawNote_t drawNote = {
                .piste_num = PARTITION[noteIndex].pist_num,
                .spawn_ms = spawn,
                .duration_ms = PARTITION[noteIndex].duration_ms,
				.pressed = false
            };
            xQueueSend(xNotesQueue, &drawNote, 0);

            for (int i = 0; i < BUFF_SIZE; i++)
            {
                if (!buff[i].active)
                {
                    buff[i].active = 1;
                    buff[i].piste_num = PARTITION[noteIndex].pist_num;
                    buff[i].partitionIndex = noteIndex;
                    buff[i].hit_tick = spawn + FALL_TICKS;
                    break;
                }
            }
            noteIndex++;
        }

        if (noteIndex >= PARTITION_LENGTH && tick_diff(now, songStart + songTicks) >= 0)
        {
            songStart += songTicks;
            noteIndex = 0;
        }

        uint8_t btn[NUM_PISTES];
        btn[0] = (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_2) == GPIO_PIN_SET);
        btn[1] = (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_3) == GPIO_PIN_SET);
        btn[2] = (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_4) == GPIO_PIN_SET);
        btn[3] = (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_5) == GPIO_PIN_SET);

        for (uint8_t piste = 0; piste < NUM_PISTES; piste++)
        {
            if (!btn[piste] || previousBtn[piste])
                continue;

            int best = -1;
            int32_t bestErr = (int32_t)tolTicks + 1;
            for (int k = 0; k < BUFF_SIZE; k++)
            {
                if (!buff[k].active || buff[k].piste_num != piste)
                    continue;

                int32_t err = tick_diff(now, buff[k].hit_tick);
                if (err < 0) err = -err;
                if (err < bestErr)
                {
                    bestErr = err;
                    best = k;
                }
            }

            // Si la note est valide
            if (best >= 0)
            {
                note_t n = PARTITION[buff[best].partitionIndex];
                buff[best].active = 0;

                PressedPiste_t pressedPiste = {
                	.pist_num = piste,
                	.is_failed = 0
                };
                xQueueSend(xPressedPisteQueue, &pressedPiste, 0);

                Buzzer_SetFrequency(n.pitch);
                buzzerStop = now + pdMS_TO_TICKS(n.duration_ms);
                buzzerOn = 1;

                int score;
                get_score(&score);
				set_score(score + 10);
            }
            
            // Si la note est invalide
            else
            {
                PressedPiste_t pressedPiste = {
                	.pist_num = piste,
                	.is_failed = 1
                };
                xQueueSend(xPressedPisteQueue, &pressedPiste, 0);

                int score;
                get_score(&score);
                set_score(score - 10);

                Buzzer_SetFrequency(DO_D);
                buzzerStop = now + pdMS_TO_TICKS(150);
                buzzerOn = 1;
            }
        }

        for (uint8_t piste = 0; piste < NUM_PISTES; piste++)
        {
            previousBtn[piste] = btn[piste];
        }

        for (int k = 0; k < BUFF_SIZE; k++)
        {
            if (buff[k].active && tick_diff(now, buff[k].hit_tick + tolTicks) > 0)
            {
                buff[k].active = 0;
            }
        }

        if (buzzerOn && tick_diff(now, buzzerStop) >= 0)
        {
            Buzzer_SetFrequency(0);
            buzzerOn = 0;
        }

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(5));
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

    xNotesQueue = xQueueCreate(10, sizeof(DrawNote_t));
    xPressedPisteQueue = xQueueCreate(10, sizeof(PressedPiste_t));

    xScoreSemaphore = xSemaphoreCreateMutex();

    srand(SysTick->VAL);

    /* Creation des threads */
    if (!(pdPASS == xTaskCreate(Screen_Thread, "Screen", STACK_SIZE * 4, NULL, 1, NULL)))
        goto err;
	if (!(pdPASS == xTaskCreate(Engine_Thread, "Engine", STACK_SIZE * 4, NULL, 2, NULL)))
		goto err;

    vTaskStartScheduler();

err:
    while (1);

    return 0;
}

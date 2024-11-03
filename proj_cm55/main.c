/****************************************************************************
* File Name        : main.c
*
* Description      : This source file contains the main routine for CM55 CPU
*
* Related Document : See README.md
*
*****************************************************************************
* Copyright 2023-2024, Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
*
* This software, including source code, documentation and related
* materials ("Software") is owned by Cypress Semiconductor Corporation
* or one of its affiliates ("Cypress") and is protected by and subject to
* worldwide patent protection (United States and foreign),
* United States copyright laws and international treaty provisions.
* Therefore, you may use this Software only as provided in the license
* agreement accompanying the software package from which you
* obtained this Software ("EULA").
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software
* source code solely for use in connection with Cypress's
* integrated circuit products.  Any reproduction, modification, translation,
* compilation, or representation of this Software except as specified
* above is prohibited without the express written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer
* of such system or application assumes all risk of such use and in doing
* so agrees to indemnify Cypress against all liability.
*****************************************************************************/

#include "cyhal.h"
#include "cybsp.h"

#ifdef COMPONENT_FREERTOS
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#endif

#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"
#include "cy_lvgl_port.h"
#include "cypm_utils.h"
#include "cy_syspm.h"

/*****************************************************************************
 * Macros
 *****************************************************************************/
#define BLINKY_LED_TASK_NAME                ("CM55 Blinky Task")
#define BLINKY_LED_TASK_STACK_SIZE          (configMINIMAL_STACK_SIZE * 4)
#define BLINKY_LED_TASK_PRIORITY            (configMAX_PRIORITIES - 1)
#define BLINKY_LED_TASK_DELAY_MSEC          (500)

#define DISPLAY_TASK_NAME                   ("Display Demo Task")
#define DISPLAY_TASK_STACK_SIZE             (1024 * 8)  // LVGL needs more stack
#define DISPLAY_TASK_PRIORITY               (configMAX_PRIORITIES - 1)
#define DISPLAY_TASK_PERIOD_MS              (5)         // LVGL timer period

#define DISPLAY_ACTIVE_PERIOD_MS         (10000)    // Active display time before dimming
#define DISPLAY_REFRESH_ACTIVE_MS        (30)       // Active refresh rate
#define DISPLAY_REFRESH_IDLE_MS          (200)      // Idle refresh rate
#define SYSTEM_DEEP_SLEEP_MS            (5000)      // Time before entering deep sleep

/*****************************************************************************
 * Function Prototypes
 *****************************************************************************/
static void cm55_blinky_task(void * arg);
static void display_task(void * arg);
static void display_timer_callback(TimerHandle_t xTimer);
static void system_sleep_timer_callback(TimerHandle_t xTimer);
static bool system_deep_sleep_callback(cy_stc_syspm_callback_params_t *callbackParams);

/*****************************************************************************
 * Function Name: main
 ******************************************************************************
 * Summary:
 * This is the main function for CM55 non-secure application. 
 *    1. It initializes the device and board peripherals.
 *    3. It creates the FreeRTOS application task 'cm55_blinky_task'
 *    4. It starts the RTOS task scheduler.
 *
 * Parameters:
 *  void
 *
 * Return:
 *  int
 *
 *****************************************************************************/
int main(void)
{
    cy_rslt_t result;

    /* Initialize the device and board peripherals */
    result = cybsp_init();

    /* Board init failed. Stop program execution */
    if (CY_RSLT_SUCCESS != result)
    {
        CY_ASSERT(0);
    }

    /* Enable global interrupts */
    __enable_irq();

    /* Create the display activity timer */
    display_timer = xTimerCreate("Display Timer", 
                                pdMS_TO_TICKS(DISPLAY_ACTIVE_PERIOD_MS),
                                pdFALSE, NULL, display_timer_callback);

    /* Create the system sleep timer */
    system_sleep_timer = xTimerCreate("Sleep Timer",
                                    pdMS_TO_TICKS(SYSTEM_DEEP_SLEEP_MS),
                                    pdFALSE, NULL, system_sleep_timer_callback);

    /* Register deep sleep callback */
    static cy_stc_syspm_callback_params_t callbackParams = {NULL, NULL};
    static cy_stc_syspm_callback_t deepSleepCb = {
        .callback = system_deep_sleep_callback,
        .type = CY_SYSPM_DEEPSLEEP,
        .skipMode = 0,
        .callbackParams = &callbackParams,
    };
    Cy_SysPm_RegisterCallback(&deepSleepCb);

    /* Create the FreeRTOS Task */
    result = xTaskCreate(cm55_blinky_task, BLINKY_LED_TASK_NAME, 
                        BLINKY_LED_TASK_STACK_SIZE, NULL, 
                        BLINKY_LED_TASK_PRIORITY, NULL);

    
    if( pdPASS == result )
    {
        /* Create the Display Demo Task */
        result = xTaskCreate(display_task, DISPLAY_TASK_NAME,
                            DISPLAY_TASK_STACK_SIZE, NULL,
                            DISPLAY_TASK_PRIORITY, NULL);

        /* Start timers */
        xTimerStart(display_timer, 0);
        xTimerStart(system_sleep_timer, 0);

        /* Start the RTOS Scheduler */
        vTaskStartScheduler();
    }

    return 0;
}

/*******************************************************************************
 * Function Name: cm55_blinky_task 
 *******************************************************************************
 * Summary:
 * This is the FreeRTOS task callback function. 
 * It toggles "CYBSP_LED_GREEN" LED in a tight for loop with a frequency 
 * specified by BLINKY_LED_TASK_DELAY_MSEC Macro
 *
 * Parameters:
 *  void * arg
 *
 * Return:
 *  void
 *
 *******************************************************************************/

static void cm55_blinky_task(void * arg)
{
    CY_UNUSED_PARAMETER(arg);

    cyhal_gpio_init(CYBSP_LED_GREEN, CYHAL_GPIO_DIR_OUTPUT, 
                    CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);

    for (;;)
    {
        cyhal_gpio_toggle(CYBSP_LED_GREEN);
        vTaskDelay(BLINKY_LED_TASK_DELAY_MSEC);
    }
}

/*******************************************************************************
 * Function Name: display_task
 *******************************************************************************
 * Summary:
 * This task initializes the LVGL library, display, and runs the LVGL demo
 * application. It periodically calls the LVGL timer handler to update the display.
 *
 * Parameters:
 *  void * arg
 *
 * Return:
 *  void
 *******************************************************************************/
static void display_task(void * arg)
{
    cy_rslt_t result;
    CY_UNUSED_PARAMETER(arg);
    uint32_t current_refresh_rate = DISPLAY_REFRESH_ACTIVE_MS;

    /* Initialize LVGL */
    lv_init();

    /* Initialize LVGL port */
    result = cy_lvgl_port_init();
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Start LVGL demo */
    lv_demo_widgets();

    for(;;)
    {
        /* Update refresh rate based on display state */
        current_refresh_rate = display_active ? 
                             DISPLAY_REFRESH_ACTIVE_MS : 
                             DISPLAY_REFRESH_IDLE_MS;

        if (!display_active)
        {
            /* Dim the display in inactive state */
            // Note: Implementation depends on your display driver
            // lv_disp_set_brightness(lv_disp_get_default(), 50);
        }

        /* Periodically call the lv_timer handler */
        lv_timer_handler();
        
        /* Reset system sleep timer on any display activity */
        if (lv_disp_get_inactive_time(NULL) < 1000)
        {
            xTimerReset(system_sleep_timer, 0);
        }

        vTaskDelay(pdMS_TO_TICKS(current_refresh_rate));
    }
}

static void display_timer_callback(TimerHandle_t xTimer)
{
    display_active = false;
}

static void system_sleep_timer_callback(TimerHandle_t xTimer)
{
    /* Prepare system for deep sleep */
    Cy_SysPm_CpuEnterDeepSleep(CY_SYSPM_WAIT_FOR_INTERRUPT);
}

static bool system_deep_sleep_callback(cy_stc_syspm_callback_params_t *callbackParams)
{
    /* Perform any necessary cleanup before deep sleep */
    cyhal_gpio_write(CYBSP_LED_GREEN, CYBSP_LED_STATE_OFF);
    
    return true;
}

/* [] END OF FILE */

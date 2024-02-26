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

/*****************************************************************************
 * Macros
 *****************************************************************************/
#define BLINKY_LED_TASK_NAME                ("CM55 Blinky Task")
#define BLINKY_LED_TASK_STACK_SIZE          (configMINIMAL_STACK_SIZE * 4)
#define BLINKY_LED_TASK_PRIORITY            (configMAX_PRIORITIES - 1)
#define BLINKY_LED_TASK_DELAY_MSEC          (500)

/*****************************************************************************
 * Function Prototypes
 *****************************************************************************/
static void cm55_blinky_task(void * arg);

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

    /* Create the FreeRTOS Task */
    result = xTaskCreate(cm55_blinky_task, BLINKY_LED_TASK_NAME, 
                        BLINKY_LED_TASK_STACK_SIZE, NULL, 
                        BLINKY_LED_TASK_PRIORITY, NULL);

    if( pdPASS == result )
    {
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

/* [] END OF FILE */

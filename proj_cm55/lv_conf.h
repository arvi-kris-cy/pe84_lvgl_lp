/**
 * @file lv_conf.h
 * Configuration file for v8.3.11
 */

#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/* Color depth: 1 (1 byte per pixel), 8 (RGB332), 16 (RGB565), 32 (ARGB8888) */
#define LV_COLOR_DEPTH 16

/* Maximal horizontal and vertical resolution */
#define LV_HOR_RES_MAX          1024
#define LV_VER_RES_MAX          600

/* Default display refresh period. LVG will redraw changed areas with this period time */
#define LV_DISP_DEF_REFR_PERIOD 30

/* Enable demos */
#define LV_USE_DEMO_WIDGETS     1
#define LV_USE_DEMO_BENCHMARK   0
#define LV_USE_DEMO_STRESS      0
#define LV_USE_DEMO_KEYPAD_AND_ENCODER 0
#define LV_USE_DEMO_MUSIC       0

/* Memory settings */
#define LV_MEM_CUSTOM 1
#if LV_MEM_CUSTOM == 0
    #define LV_MEM_SIZE (64U * 1024U)
#else
    #define LV_MEM_CUSTOM_INCLUDE "stdlib.h"
    #define LV_MEM_CUSTOM_ALLOC   malloc
    #define LV_MEM_CUSTOM_FREE    free
#endif

/* Enable GPU */
#define LV_USE_GPU_ARM2D        0
#define LV_USE_GPU_STM32_DMA2D  0
#define LV_USE_GPU_SWM341_DMA2D 0
#define LV_USE_GPU_NXP_PXP      0
#define LV_USE_GPU_NXP_VG_LITE  0

/* Default screen refresh period */
#define LV_DISP_DEF_REFR_PERIOD 30

/* Buffering */
#define LV_DISP_DEF_DRAW_BUF_SIZE (LV_HOR_RES_MAX * 40)

/* Enable logging */
#define LV_USE_LOG 1
#if LV_USE_LOG
    #define LV_LOG_LEVEL LV_LOG_LEVEL_WARN
    #define LV_LOG_PRINTF 1
#endif

/* Enable asserts */
#define LV_USE_ASSERT_NULL          1
#define LV_USE_ASSERT_MEM          1
#define LV_USE_ASSERT_STR          1
#define LV_USE_ASSERT_OBJ          1
#define LV_USE_ASSERT_STYLE        1

/* Reduce default refresh period when idle */
#define LV_DISP_DEF_REFR_PERIOD 30

/* Enable built-in power saving features */
#define LV_USE_PERF_MONITOR     1
#define LV_USE_MEM_MONITOR      1

/* Optimize memory usage */
#define LV_MEM_CUSTOM_INCLUDE   "stdlib.h"
#define LV_MEM_CUSTOM          1
#define LV_MEMCPY_MEMSET_STD   1

/* Reduce animation time to save power */
#define LV_DISP_DEF_REFR_PERIOD    30
#define LV_INDEV_DEF_READ_PERIOD   30

/* Enable partial display refresh */
#define LV_USE_DRAW_MASKS          1

#endif /*LV_CONF_H*/ 
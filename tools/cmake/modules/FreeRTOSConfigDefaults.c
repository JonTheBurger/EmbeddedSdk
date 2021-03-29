#include "FreeRTOS.h"

/** Define something in this translation unit to suppress compiler warnings in case all macros are false.
 * -fdata-sections & --gc-sections will discard this.
 */
char FindFreeRTOS_WarningSuppressor = '\0';

///** Example implementations from https://www.freertos.org/a00110.html#configSUPPORT_STATIC_ALLOCATION */
///// @{

#if (defined(IMPLEMENT_DEFAULT_vApplicationGetIdleTaskMemory) && (IMPLEMENT_DEFAULT_vApplicationGetIdleTaskMemory))
void vApplicationGetIdleTaskMemory(StaticTask_t** ppxIdleTaskTCBBuffer,
                                   StackType_t** ppxIdleTaskStackBuffer,
                                   uint32_t* pulIdleTaskStackSize)
{
  static StaticTask_t xIdleTaskTCB;
  static StackType_t  uxIdleTaskStack[configMINIMAL_STACK_SIZE];
  *ppxIdleTaskTCBBuffer   = &xIdleTaskTCB;
  *ppxIdleTaskStackBuffer = uxIdleTaskStack;
  *pulIdleTaskStackSize   = configMINIMAL_STACK_SIZE;
}
#endif /* IMPLEMENT_DEFAULT_vApplicationGetIdleTaskMemory */

#if (defined(IMPLEMENT_DEFAULT_vApplicationGetTimerTaskMemory) && (IMPLEMENT_DEFAULT_vApplicationGetTimerTaskMemory))
void vApplicationGetTimerTaskMemory(StaticTask_t** ppxTimerTaskTCBBuffer,
                                    StackType_t** ppxTimerTaskStackBuffer,
                                    uint32_t* pulTimerTaskStackSize)
{
  static StaticTask_t xTimerTaskTCB;
  static StackType_t  uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];
  *ppxTimerTaskTCBBuffer    = &xTimerTaskTCB;
  *ppxTimerTaskStackBuffer  = uxTimerTaskStack;
  *pulTimerTaskStackSize    = configTIMER_TASK_STACK_DEPTH;
}
#endif /* IMPLEMENT_DEFAULT_vApplicationGetTimerTaskMemory */

///// @}

// std
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// 3rd party
#include <FreeRTOS.h>
#include <queue.h>
#include <semphr.h>
#include <task.h>

#define STRINGIFY(expr)  #expr
#define XSTRINGIFY(expr) STRINGIFY(expr)

typedef struct Task {
  TaskHandle_t Handle;
  StaticTask_t ControlBlock;
  StackType_t  Stack[configMINIMAL_STACK_SIZE];
} Task;

static Task TaskA;
static Task TaskB;

void HelloWorld(void* parameters)
{
  while (true)
  {
    printf("Hello From %s!\n", (const char*)parameters);
    vTaskDelay(100);
  }
}

void CreateTasks()
{
    TaskA.Handle = xTaskCreateStatic(HelloWorld, STRINGIFY(TaskA), sizeof(TaskA.Stack) / sizeof(*TaskA.Stack),
                                     STRINGIFY(TaskA), 1, TaskA.Stack, &TaskA.ControlBlock);
    TaskB.Handle = xTaskCreateStatic(HelloWorld, STRINGIFY(TaskB), sizeof(TaskB.Stack) / sizeof(*TaskB.Stack),
                                     STRINGIFY(TaskB), 1, TaskB.Stack, &TaskB.ControlBlock);
}

int main()
{
  CreateTasks();
  vTaskStartScheduler();

  return 0;
}

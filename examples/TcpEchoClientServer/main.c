// std
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// 3rd party
#include <FreeRTOS.h>
#include <FreeRTOS_IP.h>
#include <FreeRTOS_Sockets.h>
#include <queue.h>
#include <semphr.h>
#include <task.h>

#define STRINGIFY(expr)  #expr
#define XSTRINGIFY(expr) STRINGIFY(expr)

/* Default MAC address configuration.  The demo creates a virtual network
 * connection that uses this MAC address by accessing the raw Ethernet data
 * to and from a real network connection on the host PC.  See the
 * configNETWORK_INTERFACE_TO_USE definition for information on how to configure
 * the real network connection to use. */
uint8_t ucMACAddress[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

typedef struct Task {
  TaskHandle_t Handle;
  StaticTask_t ControlBlock;
  StackType_t  Stack[configMINIMAL_STACK_SIZE];
} Task;

static Task             NetTask;
static Task             ServerTask;
static const TickType_t DEFAULT_TIMEOUT = pdMS_TO_MIN_TICKS(2000);

bool CheckSocketResult(BaseType_t result, const char* name)
{
  bool ok = true;
  if (result == 0) { printf("%s succeeded\n", name); }
  else
  {
    ok = false;
    printf("%s failed with code %ld\n", name, result);
  }
  return ok;
}

void NetTask_Run(void* parameters)
{
  (void)parameters;
  BaseType_t               socket_result  = 0;
  struct freertos_sockaddr remote_address = {
    .sin_port = FreeRTOS_htons((uint16_t)10000),
    .sin_addr = FreeRTOS_inet_addr_quick(127, 0, 0, 1),
  };

  Socket_t socket = FreeRTOS_socket(FREERTOS_AF_INET, FREERTOS_SOCK_STREAM, FREERTOS_IPPROTO_TCP);
  configASSERT(socket != FREERTOS_INVALID_SOCKET);
  socket_result = FreeRTOS_setsockopt(socket, 0, FREERTOS_SO_RCVTIMEO, &DEFAULT_TIMEOUT, sizeof(DEFAULT_TIMEOUT));
  CheckSocketResult(socket_result, "[Client] sockopt RCVTIMEO");
  socket_result = FreeRTOS_setsockopt(socket, 0, FREERTOS_SO_SNDTIMEO, &DEFAULT_TIMEOUT, sizeof(DEFAULT_TIMEOUT));
  CheckSocketResult(socket_result, "[Client] sockopt SNDTIMEO");

  static const char tx[] = "Hello From Client!";

  do
  {
    socket_result = FreeRTOS_connect(socket, &remote_address, sizeof(remote_address));
  } while (!CheckSocketResult(socket_result, "[Client] connect"));

  while (true)
  {
    size_t total_sent = 0;
    while (total_sent < sizeof(tx))
    {
      size_t     tx_size = sizeof(tx) - total_sent;
      BaseType_t sent    = FreeRTOS_send(socket, &tx[total_sent], tx_size, 0);

      if (sent >= 0) { total_sent += sent; }
      else
      {
        CheckSocketResult(sent, "[Client] send");
      }
    }
    vTaskDelay(DEFAULT_TIMEOUT);
  }

  FreeRTOS_shutdown(socket, FREERTOS_SHUT_RDWR);
  FreeRTOS_closesocket(socket);
  vTaskDelete(NULL);
}

void ServerTask_Run(void* parameters)
{
  (void)parameters;
  static const TickType_t  WAIT_FOREVER    = portMAX_DELAY;
  static const BaseType_t  MAX_CONNECTIONS = 5;
  struct freertos_sockaddr bind_address    = {
    .sin_port = FreeRTOS_htons((uint16_t)10000),
  };
  BaseType_t socket_result = 0;

  Socket_t listen_socket = FreeRTOS_socket(FREERTOS_AF_INET, FREERTOS_SOCK_STREAM, FREERTOS_IPPROTO_TCP);
  configASSERT(listen_socket != FREERTOS_INVALID_SOCKET);
  socket_result = FreeRTOS_setsockopt(listen_socket, 0, FREERTOS_SO_RCVTIMEO, &WAIT_FOREVER, sizeof(WAIT_FOREVER));
  CheckSocketResult(socket_result, "[Server] sockopt RCVTIMEO");
  socket_result = FreeRTOS_bind(listen_socket, &bind_address, sizeof(bind_address));
  CheckSocketResult(socket_result, "[Server] bind");
  socket_result = FreeRTOS_listen(listen_socket, MAX_CONNECTIONS);
  CheckSocketResult(socket_result, "[Server] listen");

  struct freertos_sockaddr client;
  socklen_t                client_size      = sizeof(client);
  Socket_t                 connected_socket = FreeRTOS_accept(listen_socket, &client, &client_size);
  configASSERT(connected_socket != FREERTOS_INVALID_SOCKET);

  static char rx[32];
  memset(rx, 0, sizeof(rx));

  while (true)
  {
    BaseType_t rx_size = FreeRTOS_recv(connected_socket, rx, sizeof(rx), 0);
    if (rx_size > 0) { printf("[Server] Received: %s\n", rx); }
    else if (rx_size < 0)
    {
      CheckSocketResult(rx_size, "[Server] recv");
    }
  }
  FreeRTOS_shutdown(connected_socket, FREERTOS_SHUT_RDWR);
  FreeRTOS_closesocket(connected_socket);
  vTaskDelete(NULL);
}

void CreateTasks()
{
  ServerTask.Handle =
    xTaskCreateStatic(ServerTask_Run, STRINGIFY(ServerTask_Run), sizeof(ServerTask.Stack) / sizeof(*ServerTask.Stack),
                      STRINGIFY(ServerTask_Run), 1, ServerTask.Stack, &ServerTask.ControlBlock);
  NetTask.Handle =
    xTaskCreateStatic(NetTask_Run, STRINGIFY(NetTask_Run), sizeof(NetTask.Stack) / sizeof(*NetTask.Stack),
                      STRINGIFY(NetTask_Run), 1, NetTask.Stack, &NetTask.ControlBlock);
}

int main()
{
  //  static const uint8_t ucIPAddress[4]      = { 10, 10, 10, 200 };
  static const uint8_t ucIPAddress[4] = { 127, 0, 0, 1 };
  static const uint8_t ucNetMask[4]   = { 255, 0, 0, 0 };
  //  static const uint8_t ucGatewayAddress[4] = { 10, 10, 10, 1 };
  static const uint8_t ucGatewayAddress[4] = { 127, 0, 0, 1 };
  /* The following is the address of an OpenDNS server. */
  static const uint8_t ucDNSServerAddress[4] = { 208, 67, 222, 222 };
  FreeRTOS_IPInit(ucIPAddress, ucNetMask, ucGatewayAddress, ucDNSServerAddress, ucMACAddress);
  CreateTasks();
  vTaskStartScheduler();

  return 0;
}

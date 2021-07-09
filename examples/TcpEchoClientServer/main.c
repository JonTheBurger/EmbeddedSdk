#include <pcap/pcap.h>

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
uint8_t ucMACAddress[6] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55 };

typedef struct Task {
  TaskHandle_t Handle;
  StaticTask_t ControlBlock;
  StackType_t  Stack[configMINIMAL_STACK_SIZE];
} Task;

static Task             NetTask;
static Task             ServerTask;
static const TickType_t DEFAULT_TIMEOUT = pdMS_TO_MIN_TICKS(6000);

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
    .sin_len    = sizeof(struct freertos_sockaddr),
    .sin_family = FREERTOS_AF_INET,
    .sin_port   = FreeRTOS_htons((uint16_t)15000),
    //.sin_addr   = FreeRTOS_inet_addr_quick(127, 0, 0, 1),
    .sin_addr   = FreeRTOS_inet_addr_quick(192, 168, 1, 100),
  };

  vTaskDelay(pdMS_TO_TICKS(1000));
  Socket_t socket;

  static const char tx[] = "Hello From Client!";

  do
  {
    socket = FreeRTOS_socket(FREERTOS_AF_INET, FREERTOS_SOCK_STREAM, FREERTOS_IPPROTO_TCP);
    configASSERT(socket != FREERTOS_INVALID_SOCKET);
    socket_result = FreeRTOS_setsockopt(socket, 0, FREERTOS_SO_RCVTIMEO, &DEFAULT_TIMEOUT, sizeof(DEFAULT_TIMEOUT));
    CheckSocketResult(socket_result, "[Client] sockopt RCVTIMEO");
    socket_result = FreeRTOS_setsockopt(socket, 0, FREERTOS_SO_SNDTIMEO, &DEFAULT_TIMEOUT, sizeof(DEFAULT_TIMEOUT));
    CheckSocketResult(socket_result, "[Client] sockopt SNDTIMEO");
    socket_result = FreeRTOS_connect(socket, &remote_address, sizeof(remote_address));
    if (socket_result != 0)
    {
      if (FreeRTOS_closesocket(socket) == 1)
      {
        printf("[Clinet] socketclose succeeded\n");
      }
      else
      {
        printf("[Clinet] socketclose failed\n");
      }
    }
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
  //vTaskDelay(pdMS_TO_TICKS(1000));
  struct freertos_sockaddr bind_address    = {
    //.sin_len    = sizeof(struct freertos_sockaddr),
    //.sin_family = FREERTOS_AF_INET,
    .sin_port   = FreeRTOS_htons((uint16_t)15000),
    //.sin_addr   = FreeRTOS_inet_addr_quick(127, 0, 0, 1),
    //.sin_addr   = FreeRTOS_inet_addr_quick(192, 168, 1, 100),
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
  printf("[Server] accept succeeded\n");

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

void netmain()
{
  //  static const uint8_t ucIPAddress[4]      = { 10, 10, 10, 200 };
  // static const uint8_t ucIPAddress[4] = { 127, 0, 0, 1 };
  // 192 168 1 10
  static const uint8_t ucIPAddress[4] = { 192, 168, 1, 100 };
  static const uint8_t ucNetMask[4]   = { 255, 255, 255, 0 };
  static const uint8_t ucGatewayAddress[4] = { 192, 168, 1, 1 };
  /* The following is the address of an OpenDNS server. */
  static const uint8_t ucDNSServerAddress[4] = { 208, 67, 222, 222 };
  FreeRTOS_IPInit(ucIPAddress, ucNetMask, ucGatewayAddress, ucDNSServerAddress, ucMACAddress);
  CreateTasks();
  vTaskStartScheduler();
}

void netifmain() {
  pcap_if_t *alldevs;
  pcap_if_t *d;
  int i=0;
  char errbuf[PCAP_ERRBUF_SIZE];
  
  /* Retrieve the device list from the local machine */
  if (pcap_findalldevs(&alldevs, NULL /* auth is not needed */, &alldevs, errbuf) == -1)
  {
      fprintf(stderr,"Error in pcap_findalldevs_ex: %s\n", errbuf);
      exit(1);
  }
  
  /* Print the list */
  for(d= alldevs; d != NULL; d= d->next)
  {
      printf("%d. %s", ++i, d->name);
      if (d->description)
          printf(" (%s)\n", d->description);
      else
          printf(" (No description available)\n");
  }
  
  if (i == 0)
  {
      printf("\nNo interfaces found! Make sure WinPcap is installed.\n");
      return;
  }

  /* We don't need any more the device list. Free it */
  pcap_freealldevs(alldevs);
  return 0;
}

int main()
{
  netmain();
  return 0;
}

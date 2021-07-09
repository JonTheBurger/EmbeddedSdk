#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <FreeRTOS.h>
#include <FreeRTOS_IP.h>
#include <FreeRTOS_Sockets.h>

static BaseType_t xCaseInsensitiveStringCompare(const char* lhs, const char* rhs)
{
  BaseType_t diff = 0;

  if ((lhs != NULL) && (rhs != NULL))
  {
    while ((diff == 0) && (*lhs != '\0') && (*rhs != '\0'))
    {
      diff = toupper(*lhs++) - toupper(*rhs++);
    }

    if (diff == 0)
    {
      if ((*lhs == '\0') && (*rhs != '\0')) { diff = -*rhs; }
      else if ((*lhs != '\0') && (*rhs == '\0'))
      {
        diff = *lhs;
      }
    }
  }
  
  return diff;
}

const char* pcApplicationHostnameHook(void)
{
  /* Assign the name "FreeRTOS" to this network node.  This function will
   * be called during the DHCP: the machine will be registered with an IP
   * address plus this name. */
  return mainHOST_NAME;
}

BaseType_t xApplicationDNSQueryHook(const char* pcName)
{
  BaseType_t xReturn;

  /* Determine if a name lookup is for this node.  Two names are given
   * to this node: that returned by pcApplicationHostnameHook() and that set
   * by mainDEVICE_NICK_NAME. */
  if (xCaseInsensitiveStringCompare(pcName, pcApplicationHostnameHook()) == 0) { xReturn = pdPASS; }
  else if (xCaseInsensitiveStringCompare(pcName, mainDEVICE_NICK_NAME) == 0)
  {
    xReturn = pdPASS;
  }
  else
  {
    xReturn = pdFAIL;
  }

  return xReturn;
}

void vApplicationIPNetworkEventHook(eIPCallbackEvent_t eNetworkEvent)
{
  uint32_t          ulIPAddress, ulNetMask, ulGatewayAddress, ulDNSServerAddress;
  char              cBuffer[16];
  static BaseType_t xTasksAlreadyCreated = pdFALSE;

  /* If the network has just come up...*/
  if (eNetworkEvent == eNetworkUp)
  {
    /* Create the tasks that use the IP stack if they have not already been
     * created. */
    if (xTasksAlreadyCreated == pdFALSE)
    {
      /* See the comments above the definitions of these pre-processor
       * macros at the top of this file for a description of the individual
       * demo tasks. */

#if (mainCREATE_TCP_ECHO_TASKS_SINGLE == 1)
      {
        vStartTCPEchoClientTasks_SingleTasks(mainECHO_CLIENT_TASK_STACK_SIZE, mainECHO_CLIENT_TASK_PRIORITY);
      }
#endif /* mainCREATE_TCP_ECHO_TASKS_SINGLE */

      xTasksAlreadyCreated = pdTRUE;
    }

    /* Print out the network configuration, which may have come from a DHCP
     * server. */
    FreeRTOS_GetAddressConfiguration(&ulIPAddress, &ulNetMask, &ulGatewayAddress, &ulDNSServerAddress);
    FreeRTOS_inet_ntoa(ulIPAddress, cBuffer);
    printf("\r\n\r\nIP Address: %s\r\n", cBuffer);

    FreeRTOS_inet_ntoa(ulNetMask, cBuffer);
    printf("Subnet Mask: %s\r\n", cBuffer);

    FreeRTOS_inet_ntoa(ulGatewayAddress, cBuffer);
    printf("Gateway Address: %s\r\n", cBuffer);

    FreeRTOS_inet_ntoa(ulDNSServerAddress, cBuffer);
    printf("DNS Server Address: %s\r\n\r\n\r\n", cBuffer);
  }
  else
  {
    printf("Application idle hook network down\n");
  }
}

/*
 * Callback that provides the inputs necessary to generate a randomized TCP
 * Initial Sequence Number per RFC 6528.  THIS IS ONLY A DUMMY IMPLEMENTATION
 * THAT RETURNS A PSEUDO RANDOM NUMBER SO IS NOT INTENDED FOR USE IN PRODUCTION
 * SYSTEMS.
 */
extern uint32_t ulApplicationGetNextSequenceNumber(uint32_t ulSourceAddress,
                                                   uint16_t usSourcePort,
                                                   uint32_t ulDestinationAddress,
                                                   uint16_t usDestinationPort)
{
  (void)ulSourceAddress;
  (void)usSourcePort;
  (void)ulDestinationAddress;
  (void)usDestinationPort;

  return uxRand();
}

/* First statically allocate the buffers, ensuring an additional ipBUFFER_PADDING
bytes are allocated to each buffer.  This example makes no effort to align
the start of the buffers, but most hardware will have an alignment requirement.
If an alignment is required then the size of each buffer must be adjusted to
ensure it also ends on an alignment boundary.  Below shows an example assuming
the buffers must also end on an 8-byte boundary. */
#define BUFFER_SIZE            (ipTOTAL_ETHERNET_FRAME_SIZE + ipBUFFER_PADDING)
#define BUFFER_SIZE_ROUNDED_UP ((BUFFER_SIZE + 7) & ~0x07UL)
static uint8_t ucBuffers[ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS][BUFFER_SIZE_ROUNDED_UP];

/* Next provide the vNetworkInterfaceAllocateRAMToBuffers() function, which
simply fills in the pucEthernetBuffer member of each descriptor. */
void vNetworkInterfaceAllocateRAMToBuffers(
  NetworkBufferDescriptor_t pxNetworkBuffers[ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS])
{
  BaseType_t x;

  for (x = 0; x < ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS; x++)
  {
    /* pucEthernetBuffer is set to point ipBUFFER_PADDING bytes in from the
    beginning of the allocated buffer. */
    pxNetworkBuffers[x].pucEthernetBuffer = &(ucBuffers[x][ipBUFFER_PADDING]);

    /* The following line is also required, but will not be required in
    future versions. */
    *((uint32_t*)&ucBuffers[x][0]) = (uint32_t) & (pxNetworkBuffers[x]);
  }
}

// Pretend to always have 4MiB available
size_t xPortGetMinimumEverFreeHeapSize(void) {
  return 4194304;
}

size_t xPortGetFreeHeapSize(void) {
  return 4194304;
}

set(CMAKE_C_STANDARD 11)

set(FreeRTOS_ROOT ${PROJECT_SOURCE_DIR}/external/FreeRTOS-LTS/FreeRTOS/FreeRTOS-Kernel)
set(FreeRTOS_LIBRARY_TYPE OBJECT)
set(FreeRTOS_HEAP 3)
#set(FreeRTOS_CONFIG_H ${CMAKE_CURRENT_LIST_DIR}/FreeRTOSConfig.h)
find_package(FreeRTOS 10.4.3 REQUIRED)

add_executable(HelloTasks HelloTasks/main.c)
target_link_libraries(HelloTasks
  PRIVATE
    FreeRTOS::Kernel
)

add_executable(TcpEchoClientServer TcpEchoClientServer/main.c)
target_link_libraries(TcpEchoClientServer
  PRIVATE
    FreeRTOS::Kernel
)

list(APPEND CMAKE_PREFIX_PATH "C:/Tools/npcap-sdk")
find_package(PCap REQUIRED)
set(tcpdir ${PROJECT_SOURCE_DIR}/external/FreeRTOS-LTS/FreeRTOS/FreeRTOS-Plus-TCP)
file(GLOB tcpFiles LIST_DIRECTORIES false "${tcpdir}/*.c")
target_sources(TcpEchoClientServer
  PRIVATE
    ${tcpFiles}
    ${tcpdir}/portable/BufferManagement/BufferAllocation_2.c
    ${PROJECT_SOURCE_DIR}/tools/cmake/modules/FreeRTOSIPConfigDefaults.c
    #${tcpdir}/portable/NetworkInterface/linux/NetworkInterface.c
    ${tcpdir}/portable/NetworkInterface/WinPCap/NetworkInterface.c
    #${tcpdir}/portable/NetworkInterface/WinPCap/FaultInjection.c
)
target_include_directories(TcpEchoClientServer
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${tcpdir}/include
    #${tcpdir}/portable/Compiler/GCC
    ${tcpdir}/portable/Compiler/MSVC
)
target_link_libraries(TcpEchoClientServer
  PRIVATE
    PCap::PCap
)


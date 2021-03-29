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

if (UNIX)
  add_executable(TcpEchoClientServer TcpEchoClientServer/main.c)
  target_link_libraries(TcpEchoClientServer
    PRIVATE
      FreeRTOS::Kernel
  )

  find_package(PkgConfig)
  pkg_check_modules(PCAP REQUIRED IMPORTED_TARGET libpcap)
  set(tcpdir ${PROJECT_SOURCE_DIR}/external/FreeRTOS-LTS/FreeRTOS/FreeRTOS-Plus-TCP)
  file(GLOB tcpFiles LIST_DIRECTORIES false "${tcpdir}/*.c")
  target_sources(TcpEchoClientServer
    PRIVATE
      ${tcpFiles}
      ${tcpdir}/portable/BufferManagement/BufferAllocation_1.c
      ${tcpdir}/portable/NetworkInterface/linux/NetworkInterface.c
      ${PROJECT_SOURCE_DIR}/tools/cmake/modules/FreeRTOSIPConfigDefaults.c
  )
  target_include_directories(TcpEchoClientServer
    PRIVATE
      ${CMAKE_CURRENT_LIST_DIR}
      ${tcpdir}/include
      ${tcpdir}/portable/Compiler/GCC
  )
  target_link_libraries(TcpEchoClientServer
    PRIVATE
      PkgConfig::PCAP
  )
endif()

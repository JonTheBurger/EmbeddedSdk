cmake_minimum_required(VERSION 3.10)
set(PCap_FOUND FALSE)

find_package(PkgConfig QUIET)
if (PkgConfig_FOUND)
  if (PCap_FIND_VERSION)
    set(pcapModuleSpec "${PCap_FIND_VERSION}")
  endif()
  if (PCap_FIND_VERSION_EXACT)
    set(pcapModuleSpec "==${pcapModuleSpec}")
  else()
    set(pcapModuleSpec ">=${pcapModuleSpec}")
  endif()

  pkg_check_modules(PCAP IMPORTED_TARGET "libpcap${pcapModuleSpec}")
  if (PCap_FOUND)
    add_library(PCap::PCap ALIAS PckConfig::PCAP)
  endif()
endif()

if (NOT PCap_FOUND AND CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(pcapLibDir Lib/x64)
  #if (CMAKE_SIZEOF_VOID_P EQUAL 8)
  #  set(pcapLibDir Lib/x64)
  #endif()

  find_path(PCap_INCLUDE_DIRS
    pcap.h
    PATHS
      Include
      include
  )

#C:\Users\JonPo\Projects\vcpkg\installed\x64-windows\bin
  find_library(PCap_wpcap_LIBRARY
    wpcap
    PATHS
      ${pcapLibDir}
  )
  add_library(PCap::wpcap SHARED IMPORTED)
  target_include_directories(PCap::wpcap INTERFACE ${PCap_INCLUDE_DIRS})
  set_target_properties(PCap::wpcap
    PROPERTIES
      IMPORTED_LOCATION C:/Windows/System32/wpcap.dll
      IMPORTED_IMPLIB C:/Tools/npcap-sdk/Lib/x64/wpcap.lib
      #IMPORTED_IMPLIB ${PCap_wpcap_LIBRARY}
  )

  find_library(PCap_Packet_LIBRARY
    Packet
    PATHS
      ${pcapLibDir}
  )
  add_library(PCap::Packet SHARED IMPORTED)
  target_include_directories(PCap::Packet INTERFACE ${PCap_INCLUDE_DIRS})
  set_target_properties(PCap::Packet
    PROPERTIES
      IMPORTED_LOCATION C:/Windows/System32/Packet.dll
      IMPORTED_IMPLIB C:/Tools/npcap-sdk/Lib/x64/Packet.lib
      #IMPORTED_IMPLIB ${PCap_Packet_LIBRARY}
  )

  if (PCap_wpcap_LIBRARY AND PCap_Packet_LIBRARY AND PCap_INCLUDE_DIRS)
    file(READ "${PCap_INCLUDE_DIRS}/pcap/pcap.h" pcapHeader)
    string(REGEX MATCH "#define PCAP_VERSION_MAJOR [0-9]+" PCap_VERSION_MAJOR "${pcapHeader}")
    string(REGEX MATCH "#define PCAP_VERSION_MINOR [0-9]+" PCap_VERSION_MINOR "${pcapHeader}")
    string(REGEX MATCH "[0-9]+" PCap_VERSION_MAJOR "${PCap_VERSION_MAJOR}")
    string(REGEX MATCH "[0-9]+" PCap_VERSION_MINOR "${PCap_VERSION_MINOR}")

    set(PCap_FOUND TRUE)
    set(PCap_VERSION ${PCap_VERSION_MAJOR}.${PCap_VERSION_MINOR})

    add_library(PCap::PCap INTERFACE IMPORTED)
    target_link_libraries(PCap::PCap INTERFACE PCap::wpcap PCap::Packet)
  endif()
endif()

if (NOT PCap_FOUND AND NOT PCap_QUIET)
  if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    message("PCap_wpcap_LIBRARY ${PCap_wpcap_LIBRARY}")
    message("PCap_Packet_LIBRARY ${PCap_Packet_LIBRARY}")
    message(WARNING "\nCONSIDER: \n\
    1. Download https://nmap.org/npcap/ \n\
    2. Unzip to C:\\Tools\\npcap-sdk \n\
    3. set(PCap_ROOT C:\\Tools\\npcap-sdk) \n"
    )
  elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(WARNING "CONSIDER RUNNING: `sudo apt-get install libpcap-dev`")
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PCap DEFAULT_MSG PCap_FOUND)

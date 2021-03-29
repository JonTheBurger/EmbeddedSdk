#[=======================================================================[.rst:
FindFreeRTOS
------------

.. _FreeRTOS: https://github.com/FreeRTOS/FreeRTOS-Kernel
.. _FreeRTOSConfig: https://www.freertos.org/a00110.html
.. _Heap Implementation: https://www.freertos.org/a00111.html
.. _Linux Simulator: https://www.freertos.org/FreeRTOS-simulator-for-Linux.html
.. _Windows Simulator: https://www.freertos.org/FreeRTOS-Windows-Simulator-Emulator-for-Visual-Studio-and-Eclipse-MingW.html

Sets up targets for the FreeRTOS-Kernel library from a source distribution.

Cache Variables
^^^^^^^^^^^^^^^
.. variable:: FreeRTOS_ROOT

  Additional paths to search for the FreeRTOS Kernel. Should be the root path where the FreeRTOS Kernel source is
  located, such as /usr/local/src/FreeRTOS-Kernel. If not defined, this module will attempt to auto-deduce the location
  within the project's source.

Configuration Variables
^^^^^^^^^^^^^^^^^^^^^^^
Set any of the following cache variables to configure how FreeRTOS is built

.. variable:: FreeRTOS_CONFIG_H

  Path the the user's FreeRTOSConfig_.h file. If not defined, uses a provided example FreeRTOSConfig.h and prints a
  diagnostic message.

.. variable:: FreeRTOS_HEAP

  A number between 1 and 5, inclusive. Determines which FreeRTOS `Heap Implementation`_ to use. From the docs:
    1. the very simplest, does not permit memory to be freed.
    2. permits memory to be freed, but does not coalescence adjacent free blocks.
    3. simply wraps the standard malloc() and free() for thread safety.
    4. coalescences adjacent free blocks to avoid fragmentation. Includes absolute address placement option.
    5. as per heap_4, with the ability to span the heap across multiple non-adjacent memory areas.
  If no number is provided, heap_3 is used by default. 0 will leave the heap symbols unresolved.

.. variable:: FreeRTOS_PORT

  Specifies which portable subdirectory to use.
  If no combination is provided, ``GCC/ARM_CM0`` will be used by default on non-hosted :variable:`CMAKE_SYSTEM_NAME`
  (such as ``Generic``, as is typical for embedded systems). The `Linux Simulator`_ and `Windows Simulator`_ will be
  auto-deduced when building for their respective platforms, and ``FreeRTOS_PORT`` should be left to the default in this
  cas.

.. variable:: FreeRTOS_LIBRARY_TYPE

  By default, the FreeRTOS library type follows the standard :command:`add_library` conventions set by your project
  (``SHARED`` if :variable:`BUILD_SHARED_LIBS` is ``ON``, ``STATIC`` otherwise). This can be overriden manually.
  For example, you may prefer ``FreeRTOS::Kernal`` to be an ``OBJECT`` library. This gives the FreeRTOS sources access
  to all symbols in library it is linked to, which may make implementing hooks such as ``vApplicationGetIdleTaskMemory``
  easier. Note that the non-``ALIASED`` ``FreeRTOS_Kernel`` is not marked as ``IMPORTED``, so you may use
  :command:`target_sources` to append the source list, as well as :command:`target_include_directories` etc. The
  Linux and Windows simulators' default configurations can be built as standalone libraries without issue.

Result Variables
^^^^^^^^^^^^^^^^
.. variable:: FreeRTOS_FOUND

  True if FreeRTOS was found and configured successfully

.. variable:: FreeRTOS_VERSION

  Semantic version string of FreeRTOS

.. variable:: FreeRTOS_VERSION_MAJOR

  Major version; increment indicates breaking changes

.. variable:: FreeRTOS_VERSION_MINOR

  Minor version; incrememnt indicates new features

.. variable:: FreeRTOS_VERSION_PATCH

  Patch version; incrememnt indicates bug fixes

Imported Targets
^^^^^^^^^^^^^^^^
  ``FreeRTOS::Kernel``

  Standalone FreeRTOS-Kernel. Alias for ``FreeRTOS_Kernel``. Prefer to link against this target.

  ``FreeRTOS_Kernel``

  Standalone FreeRTOS-Kernel. This target is not marked as ``IMPORTED``, so you may add additional usage requirements to
  it, such as :command:`target_source`, :command:`target_include_directories`, etc.

Example Usages:

.. code-block:: cmake

  # If you installed the FreeRTOS source somewhere that FindFreeRTOS.cmake cannot auto-detect, specify it as below:
  set(FreeRTOS_ROOT /opt/FreeRTOS-Kernel)

  # Use heap_3
  set(FreeRTOS_HEAP 3)

  # Compile to object files instead of a static library
  set(FreeRTOS_LIBRARY_TYPE OBJECT)

  # Provide our FreeRTOSConfig.h
  set(FreeRTOS_CONFIG_H external/FreeRTOSConfig.h)

  find_package(FreeRTOS 10.4.3 REQUIRED)
  # If you need to modify the usage requirements of the FreeRTOS_Kernel, you can modify its target properties here.

  add_executable(main main.c)

  target_link_libraries(main
    PRIVATE
      FreeRTOS::Kernel
  )
#]=======================================================================]
cmake_minimum_required(VERSION 3.10)
include_guard(GLOBAL)

# Helps give a hint to user explaining how to plug in a custom install location if necessary
find_path(FreeRTOS_ROOT
  NAMES
    croutine.c
    list.c
    queue.c
    tasks.c
    timers.c
  PATHS
    "${PROJECT_SOURCE_DIR}/3rdparty/"
    "${PROJECT_SOURCE_DIR}/External/"
    "${PROJECT_SOURCE_DIR}/external/"
    "${PROJECT_SOURCE_DIR}/Libraries/"
    "${PROJECT_SOURCE_DIR}/libraries/"
    "${PROJECT_SOURCE_DIR}/Libs/"
    "${PROJECT_SOURCE_DIR}/libs/"
    "${PROJECT_SOURCE_DIR}/third_party/"
  PATH_SUFFIXES
    FreeRTOS-Kernel
    FreeRTOS
    FreeRTOS/FreeRTOS/Source
    FreeRTOS/Source
    amazon-freertos/freertos_kernel
    freertos_kernel
  DOC "Additional paths searched to find FreeRTOS"
)

# If the sources were successfully found:
if (FreeRTOS_ROOT)
  # Set as found
  set(FreeRTOS_FOUND TRUE)
  mark_as_advanced(FreeRTOS_FOUND)

  # Read version string from list.c because it's the smallest of the source files and exists in old versions
  file(READ "${FreeRTOS_ROOT}/list.c" listC)
  string(REGEX MATCH "FreeRTOS( Kernel)? V[0-9]+(\\.[0-9]+)*" versionString "${listC}")
  # Parse the version
  string(REGEX MATCH "[0-9]+(\.[0-9]+)*" FreeRTOS_VERSION "${versionString}")
  # Pad with zeroes so minor/patch/tweak are filled in automatically; no major becomes error; convert to list
  string(REPLACE "." ";" versionNumbers "${FreeRTOS_VERSION}.0.0.0")
  list(GET versionNumbers 0 FreeRTOS_VERSION_MAJOR)
  list(GET versionNumbers 1 FreeRTOS_VERSION_MINOR)
  list(GET versionNumbers 2 FreeRTOS_VERSION_PATCH)
  list(GET versionNumbers 3 FreeRTOS_VERSION_TWEAK)

  # Add targets
  if (NOT TARGET FreeRTOS::Kernel)
    set(FreeRTOS_INCLUDE_DIRS ${FreeRTOS_ROOT}/include)
    # glob for sources because FreeRTOS tends to add new ones, and updating source list 50 times is more evil than glob.
    file(GLOB freertosSources LIST_DIRECTORIES false
      "${FreeRTOS_ROOT}/*.c"
      "${FreeRTOS_ROOT}/include/*.h"
    )
    # Add library, let user override the library type if they really want to
    add_library(FreeRTOS_Kernel
      ${FreeRTOS_LIBRARY_TYPE}
        ${freertosSources}
        ${freertosIncludes}
    )
    # Add the heap the user wants to use
    set(freertosHeaps 1 2 3 4 5)
    set(FreeRTOS_HEAP 3 CACHE STRING "FreeRTOS Heap Implementation")
    if (FreeRTOS_HEAP IN_LIST freertosHeaps)
      target_sources(FreeRTOS_Kernel PRIVATE ${FreeRTOS_ROOT}/portable/MemMang/heap_${FreeRTOS_HEAP}.c)
    endif()
    # Include user's FreeRTOSConfig.h
    if (NOT FreeRTOS_CONFIG_H)
      message(STATUS "Using default FreeRTOSConfig.h from ${CMAKE_CURRENT_LIST_DIR}")
      set(FreeRTOS_CONFIG_H ${CMAKE_CURRENT_LIST_DIR}/FreeRTOSConfig.h)
    endif()
    get_filename_component(configHDir ${FreeRTOS_CONFIG_H} DIRECTORY)
    list(APPEND FreeRTOS_INCLUDE_DIRS ${configHDir})
    target_sources(FreeRTOS_Kernel PUBLIC ${FreeRTOS_CONFIG_H})

    # Deduce which port to use
    if (CMAKE_SYSTEM_NAME MATCHES "Windows")
      list(APPEND FreeRTOS_INCLUDE_DIRS ${FreeRTOS_ROOT}/portable/MSVC-MingW)
      target_sources(FreeRTOS_Kernel
        PUBLIC
          ${FreeRTOS_ROOT}/portable/MSVC-MingW/portmacro.h
        PRIVATE
          ${FreeRTOS_ROOT}/portable/MSVC-MingW/port.c
          ${CMAKE_CURRENT_LIST_DIR}/FreeRTOSConfigDefaults.c
      )
      find_package(Threads REQUIRED)
      target_link_libraries(FreeRTOS_Kernel PUBLIC Threads::Threads winmm)
    elseif (CMAKE_SYSTEM_NAME MATCHES "Linux")
      list(APPEND FreeRTOS_INCLUDE_DIRS ${FreeRTOS_ROOT}/portable/ThirdParty/GCC/Posix)
      list(APPEND FreeRTOS_INCLUDE_DIRS ${FreeRTOS_ROOT}/portable/ThirdParty/GCC/Posix/utils)
      target_sources(FreeRTOS_Kernel
        PUBLIC
          ${FreeRTOS_ROOT}/portable/ThirdParty/GCC/Posix/portmacro.h
          ${FreeRTOS_ROOT}/portable/ThirdParty/GCC/Posix/utils/wait_for_event.h
        PRIVATE
          ${FreeRTOS_ROOT}/portable/ThirdParty/GCC/Posix/port.c
          ${FreeRTOS_ROOT}/portable/ThirdParty/GCC/Posix/utils/wait_for_event.c
          ${CMAKE_CURRENT_LIST_DIR}/FreeRTOSConfigDefaults.c
      )
      # Warn about gdbinit setup
      set(badGdbInit TRUE)
      if(EXISTS $ENV{HOME}/.gdbinit)
        file(READ $ENV{HOME}/.gdbinit gdbinit)
        string(REGEX MATCH "handle SIGUSR1 nostop noignore noprint" sigusr1Handled "${gdbinit}")
        string(REGEX MATCH "handle SIGALRM nostop noignore noprint" sigalrmHandled "${gdbinit}")
        if (sigusr1Handled AND sigalrmHandled)
          set(badGdbInit FALSE)
        endif()
      endif()
      if(badGdbInit)
        message(WARNING "\
 Your gdbinit file (located at $ENV{HOME}/.gdbinit) either does not exist or
 is missing the following 2 lines required for debugging the FreeRTOS Linux
 Simulator properly:
 handle SIGUSR1 nostop noignore noprint
 handle SIGALRM nostop noignore noprint
 For more details, see:
 https://www.freertos.org/FreeRTOS-simulator-for-Linux.html#gdb_debugging_tips")
      endif()
      find_package(Threads REQUIRED)
      target_link_libraries(FreeRTOS_Kernel PUBLIC Threads::Threads)
    else()
      list(APPEND FreeRTOS_INCLUDE_DIRS ${FreeRTOS_ROOT}/portable/${FreeRTOS_PORT})
      # Who knows what we'll find in there (sometimes even assembly), so just grab it all
      file(GLOB portSources LIST_DIRECTORIES false "${FreeRTOS_ROOT}/*")
      target_sources(FreeRTOS_Kernel PRIVATE portSources )
    endif()

    # Make installed library relocatable.
    foreach(indir IN LISTS FreeRTOS_INCLUDE_DIRS)
      target_include_directories(FreeRTOS_Kernel PUBLIC $<BUILD_INTERFACE:${indir}>)
    endforeach()
    # Version a library (useful perhaps if Linux users wish to build FreeRTOS simulator as shared library).
    set_target_properties(FreeRTOS_Kernel PROPERTIES
      VERSION   FreeRTOS_VERSION_MAJOR
      SOVERSION FreeRTOS_VERSION
    )
    add_library(FreeRTOS::Kernel ALIAS FreeRTOS_Kernel)
  endif()
endif()

# Let CMake handle the details
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FreeRTOS
  HANDLE_COMPONENTS
  HANDLE_VERSION_RANGE
  NAME_MISMATCHED
  VERSION_VAR
    FreeRTOS_VERSION
)

# Hide <PackageName>_ROOT only if package is found
if (${CMAKE_FIND_PACKAGE_NAME_FOUND})
  mark_as_advanced(FreeRTOS_ROOT)
endif()


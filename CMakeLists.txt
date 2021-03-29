cmake_minimum_required(VERSION 3.16)
project(EmbeddedSdk LANGUAGES C ASM)

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  set(IS_TOP_LEVEL TRUE)
else()
  set(IS_TOP_LEVEL FALSE)
  set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/tools/cmake/modules PARENT_SCOPE)
endif()
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/tools/cmake/modules)

option(EMBEDDEDSDK_BUILD_EXAMPLES "Build PC examples for Embedded SDK" ${IS_TOP_LEVEL})

if (EMBEDDEDSDK_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

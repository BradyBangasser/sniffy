cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 23)

project(sniffy VERSION 0.0.1)

if (DEFINED sniffy_SHARED_LIBS)
    set(BUILD_SHARED_LIBS "${libsniffy_SHARED_LIBS}")
endif ()

add_library(sniffy src/arrest.c)
add_library(sniffy::sniffy ALIAS sniffy)

target_include_directories(sniffy PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include")

include(GenerateExportHeader)
generate_export_header(sniffy EXPORT_FILE_NAME)

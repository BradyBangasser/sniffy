cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

project(sniffy VERSION 0.0.1)

if (DEFINED SNIFFY_SHARED_LIBS)
    set(BUILD_SHARED_LIBS "${SNIFFY_SHARED_LIBS}")
endif ()

enable_testing()

add_library(sniffy src/arrest.c src/person.c)
add_library(sniffy::sniffy ALIAS sniffy)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/include)

target_include_directories(sniffy PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(sniffy PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include")
include(GenerateExportHeader)
generate_export_header(sniffy EXPORT_FILE_NAME)
set_target_properties(sniffy PROPERTIES PUBLIC_HEADER "src/arrest.h;src/charge.h;src/person.h")
add_custom_command(TARGET sniffy POST_BUILD COMMAND cp ${CMAKE_SOURCE_DIR}/src/*.h ${CMAKE_BINARY_DIR}/include/)

add_executable(ArrestInitializationTest test/arrest_initialization.c)
target_link_libraries(ArrestInitializationTest sniffy)
target_include_directories(ArrestInitializationTest PRIVATE ${CMAKE_BINARY_DIR}/include)
add_test(NAME ArrestInitializationTest COMMAND ArrestInitializationTest)

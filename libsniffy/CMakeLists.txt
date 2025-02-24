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

find_library(LIBMYSQLCLIENT mysqlclient REQUIRED HINTS /usr/lib64/mysql/)
find_library(LIBSSL ssl REQUIRED)
find_library(LIBCRYPT crypt REQUIRED)

add_library(sniffy src/arrest.c src/person.c)
add_library(sniffy::sniffy ALIAS sniffy)

add_library(snifftest test/tdatabase.c)
add_library(snifftest::snifftest ALIAS snifftest)

target_link_libraries(sniffy ${LIBMYSQLCLIENT} ${LIBSSL} ${LIBCRYPT})
target_link_libraries(snifftest ${LIBMYSQLCLIENT} ${LIBSSL} ${LIBCRYPT})

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/include)

target_include_directories(sniffy PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(sniffy PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include")
include(GenerateExportHeader)
generate_export_header(sniffy EXPORT_FILE_NAME)
set_target_properties(sniffy PROPERTIES PUBLIC_HEADER "src/arrest.h;src/charge.h;src/person.h")
add_custom_command(TARGET sniffy POST_BUILD COMMAND cp ${CMAKE_SOURCE_DIR}/src/*.h ${CMAKE_BINARY_DIR}/include/)

function(addtest files)
    add_executable(${files}.test.out tests/${files})
    target_link_libraries(${files}.test.out sniffy snifftest)
    target_include_directories(${files}.test.out PRIVATE ${CMAKE_BINARY_DIR}/include)
    add_test(NAME ${files} COMMAND ${files}.test.out)
endfunction()

addtest(arrest_initialization.c)
addtest(person_initialization.c)

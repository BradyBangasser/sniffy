cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

set(LIBSNIFF_SQL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/sql)

project(sniffy VERSION 0.0.1)

if (DEFINED SNIFFY_SHARED_LIBS)
    set(BUILD_SHARED_LIBS "${SNIFFY_SHARED_LIBS}")
endif ()

enable_testing()

find_library(LIBMYSQLCLIENT mysqlclient REQUIRED HINTS /usr/lib64/mysql/)
find_package(OpenSSL REQUIRED)

include(CTest)

add_library(sniffy src/arrest.c src/person.c)
add_library(sniffy::sniffy ALIAS sniffy)

add_library(snifftest test/tdatabase.c)
add_library(snifftest::snifftest ALIAS snifftest)

target_link_libraries(sniffy ${LIBMYSQLCLIENT} OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(snifftest ${LIBMYSQLCLIENT} OpenSSL::SSL OpenSSL::Crypto)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/include)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/tests)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/tests/include)

configure_file(${CMAKE_SOURCE_DIR}/test/test.h.in ${CMAKE_BINARY_DIR}/tests/include/test.h)

target_include_directories(sniffy PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(sniffy PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(snifftest PRIVATE ${CMAKE_BINARY_DIR}/include ${CMAKE_BINARY_DIR}/tests/include)
include(GenerateExportHeader)
generate_export_header(sniffy EXPORT_FILE_NAME)
set_target_properties(sniffy PROPERTIES PUBLIC_HEADER "src/arrest.h;src/charge.h;src/person.h")
add_custom_command(TARGET sniffy POST_BUILD COMMAND cp ${CMAKE_SOURCE_DIR}/src/*.h ${CMAKE_BINARY_DIR}/include/)
add_custom_command(TARGET sniffy POST_BUILD COMMAND cp ${CMAKE_SOURCE_DIR}/test/*.h ${CMAKE_BINARY_DIR}/tests/include/)

function(addtest files)
    add_executable(${files}.test.out tests/${files})
    set_target_properties(${files}.test.out PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests")
    target_link_libraries(${files}.test.out sniffy snifftest ${LIBMYSQLCLIENT} OpenSSL::SSL OpenSSL::Crypto)
    target_include_directories(${files}.test.out PRIVATE ${CMAKE_BINARY_DIR}/include ${CMAKE_BINARY_DIR}/tests/include)
    add_test(NAME ${files} COMMAND tests/${files}.test.out)
endfunction()

addtest(arrest_initialization.c)
addtest(person_initialization.c)
addtest(person_set_name.c)
addtest(person_fetch_by_id.c)
addtest(person_upsert.c)

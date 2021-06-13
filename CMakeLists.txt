#
# Copyright © Dougie Lawson 2020, All rights reserved.
#

cmake_minimum_required(VERSION 3.0)

project(glowmarkt C)

add_executable(gTS gasToSQL.c)
add_executable(jP jParse.c)
add_executable(mkCfg mkconfig.c)
add_executable(rdCfg readCfg.c)
add_executable(mB mosqBloop4eva.c)
add_executable(m mosq.c)
add_executable(mE mosqElec.c)
add_executable(mG mosqGas.c)
add_executable(mJ mosqJSON.c)

find_library(lib_config config)
find_library(lib_mariadb mariadb)
find_library(lib_mosq mosquitto)
find_library(lib_json-c json-c)

list(APPEND EXTRA_INCLUDES "/usr/local/include/json")

include_directories("/usr/include/mariadb/mysql")
include_directories("/usr/include/mariadb")

target_link_libraries(gTS ${lib_config} ${lib_mosq} ${lib_mariadb} ${lib_json-c})
target_link_libraries(mJ ${lib_config} ${lib_mosq} ${lib_json-c})
target_link_libraries(mG ${lib_config} ${lib_mosq} ${lib_json-c})
target_link_libraries(mE ${lib_config} ${lib_mosq} ${lib_json-c})
target_link_libraries(mB ${lib_config} ${lib_mosq} ${lib_json-c})
target_link_libraries(m ${lib_config} ${lib_mosq})
target_link_libraries(jP ${lib_json-c})
target_link_libraries(mkCfg ${lib_config})
target_link_libraries(rdCfg ${lib_config})


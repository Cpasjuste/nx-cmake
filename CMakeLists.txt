cmake_minimum_required(VERSION 3.0)
#set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_SYSTEM_NAME "Generic")

# delete cmake cache folder before changing this options
option(BUILD_NX "Build with NX (SWITCH) support" ON)

# setup toolchain
include(Toolchain.cmake)

project(hello_cmake)

# flags
set(FLAGS -Wall)

#####################
# NX PLATORM
#####################
if (BUILD_NX)
    set(INCLUDES
            source
            ${DEVKITPRO}/libnx/include
            ${DEVKITPRO}/portlibs/armv8-a/include/freetype2
            )
    list(APPEND SOURCES
            source/main.c
            )
    list(APPEND FLAGS -D__NX__
            -ffunction-sections -fomit-frame-pointer
            )
    set(LIBS
            ${DEVKITPRO}/libnx/lib/libnx.a
            )
endif (BUILD_NX)

add_executable(${CMAKE_PROJECT_NAME}.elf ${SOURCES})
target_include_directories(${CMAKE_PROJECT_NAME}.elf PRIVATE ${INCLUDES})
target_compile_options(${CMAKE_PROJECT_NAME}.elf PRIVATE ${FLAGS})
target_link_libraries(${CMAKE_PROJECT_NAME}.elf ${LIBS})

#####################
# NX (SWITCH) target
#####################
if (BUILD_NX)
    set_target_properties(${CMAKE_PROJECT_NAME}.elf PROPERTIES LINK_FLAGS "-specs=${DEVKITPRO}/libnx/switch.specs")
    add_custom_target(${CMAKE_PROJECT_NAME}.nro
            DEPENDS ${CMAKE_PROJECT_NAME}.elf
            COMMAND elf2nro ${CMAKE_PROJECT_NAME}.elf ${CMAKE_PROJECT_NAME}.nro)
endif (BUILD_NX)

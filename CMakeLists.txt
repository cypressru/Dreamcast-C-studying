cmake_minimum_required(VERSION 3.26)

set(KOS_BASE /opt/toolchains/dc/kos)
set(KOS_CC_BASE /opt/toolchains/dc/sh-elf)
set(KOS_SUBARCH pristine)
set(KOS_PORTS ${KOS_BASE}/../kos-ports)

include(${KOS_BASE}/utils/cmake/dreamcast.toolchain.cmake)

project(untitled C)

set(CMAKE_C_STANDARD 23)

add_executable(untitled main.c)
target_link_libraries(untitled png parallax mp3 z m c gcc)

kos_add_romdisk(untitled romdisk)
kos_add_binary(untitled wfont.bin wfont)

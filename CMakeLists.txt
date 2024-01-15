cmake_minimum_required(VERSION 3.27)
set(KOS_BASE /opt/toolchains/dc/kos)
set(KOS_CC_BASE /opt/toolchains/dc/sh-elf)
set(KOS_SUBARCH pristine)
set(KOS_PORTS ${KOS_BASE}/../kos-ports)
include(${KOS_BASE}/utils/cmake/dreamcast.toolchain.cmake)



LINK_LIBRARIES(png z m -Wl,--start-group -lstdc++ -lkallisti -lc -lgcc -Wl,--end-group)
link_directories(/opt/toolchains/dc/kos-ports/lib)
add_link_options(-lm -lz -lpng)


project(untitled C)

set(CMAKE_C_STANDARD 23)
SET(CMAKE_EXECUTABLE_SUFFIX_C ".elf")
SET(CMAKE_EXECUTABLE_SUFFIX_CXX ".elf")

add_executable(untitled main.c)

kos_add_romdisk(untitled romdisk)
kos_add_binary(untitled wfont.bin wfont)


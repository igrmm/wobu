cmake_minimum_required(VERSION 3.5)
project(wobu)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(LIBS SDL2 SDL2_image)

foreach(LIB IN LISTS LIBS)
    find_package(${LIB} REQUIRED)
endforeach()

add_executable(${PROJECT_NAME}
    #WIN32
    src/jsonffile.c
    src/wobu.c
    src/app.c
    src/tilesetw.c
    src/modelw.c
    src/statusw.c
    src/menuw.c
    src/toolsw.c
    src/map.c
    src/propertiesw.c
    )

if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    if (DEFINED ENV{SANITIZERS})
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} $ENV{SANITIZERS}")
    endif()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror=vla)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${SDL2_INCLUDE_DIRS})

if(MINGW)
    target_link_libraries(${PROJECT_NAME} PRIVATE mingw32)
endif()

list(INSERT LIBS 0 m SDL2main)
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBS})

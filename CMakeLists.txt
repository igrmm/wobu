cmake_minimum_required(VERSION 3.5)
project(wobu)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_subdirectory(external/SDL EXCLUDE_FROM_ALL)
add_subdirectory(external/SDL_image EXCLUDE_FROM_ALL)
add_subdirectory(external/SDL_ttf EXCLUDE_FROM_ALL)

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
    src/txt.c
    )

if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    if (DEFINED ENV{SANITIZERS})
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} $ENV{SANITIZERS}")
    endif()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror=vla)
endif()

if(MINGW)
    target_link_libraries(${PROJECT_NAME} PRIVATE mingw32)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
    m
    SDL2::SDL2main
    SDL2::SDL2
    SDL2_image
    SDL2_ttf
    )

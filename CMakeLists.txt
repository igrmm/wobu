cmake_minimum_required(VERSION 3.0)
project(wobu)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(LIBS SDL2 SDL2_image)

foreach(LIB IN LISTS LIBS)
    find_package(${LIB} REQUIRED)
endforeach()

add_executable(${PROJECT_NAME}
    WIN32
    src/wobu.c
    )

target_include_directories(${PROJECT_NAME} PUBLIC ${SDL2_INCLUDE_DIRS})

if(MINGW)
    target_link_libraries(${PROJECT_NAME} PRIVATE mingw32)
endif()

list(INSERT LIBS 0 SDL2main)
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBS})

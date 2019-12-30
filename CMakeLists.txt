cmake_minimum_required(VERSION 3.10)

# project name
project(mineraker)

# main executable
add_executable(${PROJECT_NAME} src/main.cpp)
# add header files to include
target_include_directories(mineraker PRIVATE ${PROJECT_SOURCE_DIR}/src)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${mineraker_SOURCE_DIR}/cmake")

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)

include_directories(${SDL2_INCLUDE_DIR}
                    ${SDL2_IMAGE_INCLUDE_DIR}
                    ${SDL2_TTF_INCLUDE_DIR})
target_link_libraries(mineraker ${SDL2_LIBRARY}
                             ${SDL2_IMAGE_LIBRARIES}
                             ${SDL2_TTF_LIBRARIES})

# add link libraries to include
#target_link_libraries(mineraker ${SDL2_LIBRARIES})

set_target_properties(mineraker PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

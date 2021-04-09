cmake_minimum_required(VERSION 3.9.5)

project(basic_window LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(X11 REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(${PROJECT_NAME}
  basic_window.cpp)

target_link_libraries(${PROJECT_NAME} PUBLIC ${X11_LIBRARIES} ${OPENGL_LIBRARIES})
target_include_directories(${PROJECT_NAME} PUBLIC ${X11_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR})

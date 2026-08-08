cmake_minimum_required(VERSION 3.10)
project(MazeGame CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenGL, GLU, GLUT
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

include_directories(
    ${OPENGL_INCLUDE_DIR}
    ${GLUT_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(SOURCES
    main.cpp
    graphics/Transform.cpp
    graphics/Camera.cpp
    graphics/Light.cpp
    objects/GameObject.cpp
    objects/Primitives.cpp
    objects/ComplexObject.cpp
    game/Scene.cpp
    game/Player.cpp
    game/Game.cpp
)

add_executable(MazeGame ${SOURCES})

target_link_libraries(MazeGame
    ${OPENGL_LIBRARIES}
    ${GLUT_LIBRARIES}
)

if(WIN32)
    target_link_libraries(MazeGame glu32 freeglut opengl32)
endif()

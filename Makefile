# Makefile for 3D Maze Game

CXX = g++
CXXFLAGS = -O2 -Wall -std=c++11
INCLUDES = -I.

# OS Detection
UNAME_S := $(shell uname -s 2>/dev/null || echo Windows)

ifeq ($(UNAME_S), Linux)
    LIBS = -lGL -lGLU -lglut -lm
else ifeq ($(UNAME_S), Darwin)
    LIBS = -framework OpenGL -framework GLUT
else
    # Windows (MinGW / MYSYS)
    LIBS = -lfreeglut -lglu32 -lopengl32 -lm
endif

SOURCES = main.cpp \
          graphics/Transform.cpp \
          graphics/Camera.cpp \
          graphics/Light.cpp \
          objects/GameObject.cpp \
          objects/Primitives.cpp \
          objects/ComplexObject.cpp \
          game/Scene.cpp \
          game/Player.cpp \
          game/Game.cpp

OBJECTS = $(SOURCES:.cpp=.o)
TARGET = MazeGame

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(TARGET) $(OBJECTS) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET) $(TARGET).exe

.PHONY: all clean

# OpenGL project build file for Windows + MinGW.
# Running `make` builds every source file and starts the program.

CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Iinclude
LDFLAGS  := -Llib
LDLIBS   := -lglfw3 -lopengl32 -lgdi32

TARGET      := cog_demo.exe
CPP_SOURCES := $(wildcard src/*.cpp)
C_SOURCES   := $(wildcard src/*.c)
SOURCES     := $(CPP_SOURCES) $(C_SOURCES)
HEADERS     := $(wildcard src/*.h)

.PHONY: all build run rebuild clean help test

all: run

build: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SOURCES) $(LDFLAGS) $(LDLIBS) -o $@

run: build
	./$(TARGET)

test: build
	./$(TARGET) --self-test

rebuild: clean build

clean:
	@cmd /C "if exist $(TARGET) del /F /Q $(TARGET)"

help:
	@echo Available commands:
	@echo   make                  Build and run the project
	@echo   mingw32-make          Build and run if make is not installed
	@echo   mingw32-make build    Build without running
	@echo   mingw32-make run      Build and run the project
	@echo   mingw32-make test     Check animation and transformations
	@echo   mingw32-make rebuild  Clean and rebuild everything
	@echo   mingw32-make clean    Delete the generated executable

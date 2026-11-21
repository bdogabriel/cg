APP_DIRS := math render draw edit ui input command core io
BACKEND  ?= gl
PLATFORM ?= glfw

SRC  := src/main.cpp $(foreach d,$(APP_DIRS),$(wildcard src/$d/*.cpp) $(wildcard src/$d/*/*.cpp)) \
        $(wildcard src/backend/$(BACKEND)/*.cpp) \
        $(wildcard src/platform/$(PLATFORM)/*.cpp)
INC  := $(foreach d,$(APP_DIRS),-Isrc/$d) -Isrc/platform

ifeq ($(BACKEND),gl)
BACKEND_LIBS := -lGL -lGLEW
endif
ifeq ($(PLATFORM),glfw)
PLATFORM_LIBS := -lglfw
endif

BEAR ?= bear --

all:
	$(BEAR) g++ -Wall -Wextra -std=c++20 -o exe $(INC) $(SRC) $(PLATFORM_LIBS) $(BACKEND_LIBS)

run: all
	./exe

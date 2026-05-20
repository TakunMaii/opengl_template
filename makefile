.PHONY: all build run

HEADER=
ROOT_SOURCE=src/main.c src/glad.c
SOURCE=

CC=gcc
EXE_EXT :=

CPPFLAGS := -I external/include
CFLAGS := -Wall -Wextra
LDFLAGS :=
LDLIBS :=
RUNTIME_LIB :=
MKDIR_CMD = mkdir -p $(dir $@)
COPY_CMD = cp $(RUNTIME_LIB) $(RUNTIME_COPY)

ifeq ($(OS),Windows_NT)
EXE_EXT := .exe
LDFLAGS += -L external/lib
LDLIBS += -lglfw3dll -lgdi32
RUNTIME_LIB := external/lib/glfw3.dll
MKDIR_CMD = if not exist "$(subst /,\,$(dir $@))" mkdir "$(subst /,\,$(dir $@))"
COPY_CMD = copy /Y "$(subst /,\,$(RUNTIME_LIB))" "$(subst /,\,$(RUNTIME_COPY))" >NUL
else
UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

ifeq ($(UNAME_S),Darwin)
ifeq ($(UNAME_M),arm64)
MACOS_LIB_DIR := external/lib/macos/lib-arm64
else ifeq ($(UNAME_M),x86_64)
MACOS_LIB_DIR := external/lib/macos/lib-x86_64
else
MACOS_LIB_DIR := external/lib/macos/lib-universal
endif

LDFLAGS += -L $(MACOS_LIB_DIR)
LDLIBS += -lglfw3
LDLIBS += -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo -framework CoreFoundation
RUNTIME_LIB := $(MACOS_LIB_DIR)/libglfw.3.dylib
else ifneq (,$(findstring MINGW,$(UNAME_S)))
LDFLAGS += -L external/lib
LDLIBS += -lglfw3dll -lgdi32
RUNTIME_LIB := external/lib/glfw3.dll
else ifneq (,$(findstring MSYS,$(UNAME_S)))
LDFLAGS += -L external/lib
LDLIBS += -lglfw3dll -lgdi32
RUNTIME_LIB := external/lib/glfw3.dll
else ifeq ($(UNAME_S),Linux)
LDFLAGS += -L external/lib
LDLIBS += -lglfw3 -lGL -lm -lpthread -ldl
LDLIBS += -lX11 -lXrandr -lXi -lXxf86vm -lXcursor
else
$(error Unsupported platform: $(UNAME_S))
endif
endif

TARGET=build/main$(EXE_EXT)
BUILD_ARTIFACTS := $(TARGET)

ifneq ($(strip $(RUNTIME_LIB)),)
RUNTIME_COPY := build/$(notdir $(RUNTIME_LIB))
BUILD_ARTIFACTS += $(RUNTIME_COPY)
endif

all: run

build: $(BUILD_ARTIFACTS)

run: build
	./$(TARGET)

$(TARGET): $(HEADER) $(ROOT_SOURCE) $(SOURCE)
	$(MKDIR_CMD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(ROOT_SOURCE) $(SOURCE) -o $(TARGET) $(LDFLAGS) $(LDLIBS)

ifneq ($(strip $(RUNTIME_COPY)),)
$(RUNTIME_COPY): $(RUNTIME_LIB)
	$(MKDIR_CMD)
	$(COPY_CMD)
endif

# Fenster - Root Makefile for CLion indexing
# This Makefile provides build targets for CLion to understand the project structure

CFLAGS ?= -Wall -Wextra -std=c99 -I.
CXXFLAGS ?= -Wall -Wextra -std=c++11 -I.

# Platform-specific libraries
ifeq ($(OS),Windows_NT)
	LDFLAGS = -lgdi32 -lwinmm
	AUDIO_LDFLAGS = -lwinmm
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Darwin)
		LDFLAGS = -framework Cocoa
		AUDIO_LDFLAGS = -framework AudioToolbox
	else
		LDFLAGS = -lX11
		AUDIO_LDFLAGS = -lasound
	endif
endif

# Audio support
AUDIO_CFLAGS = $(CFLAGS) -DFENSTER_AUDIO
AUDIO_FULL_LDFLAGS = $(LDFLAGS) $(AUDIO_LDFLAGS)

# Default target
.PHONY: all
all: examples

# Build all examples
.PHONY: examples
examples: minimal-c minimal-cxx minimal-go drawing-c input-c sound-c fire-go image-go

# C Examples
minimal-c: examples/minimal-c/main
examples/minimal-c/main: examples/minimal-c/main.c fenster.h
	cd examples/minimal-c && $(CC) main.c -I../.. -o main $(CFLAGS) $(LDFLAGS)

drawing-c: examples/drawing-c/main
examples/drawing-c/main: examples/drawing-c/main.c fenster.h
	cd examples/drawing-c && $(CC) main.c -I../.. -o main $(CFLAGS) $(LDFLAGS)

input-c: examples/input-c/main
examples/input-c/main: examples/input-c/main.c fenster.h
	cd examples/input-c && $(CC) main.c -I../.. -o main $(CFLAGS) $(LDFLAGS)

sound-c: examples/sound-c/main
examples/sound-c/main: examples/sound-c/main.c fenster.h fenster_audio.h
	cd examples/sound-c && $(CC) main.c -I../.. -o main $(AUDIO_CFLAGS) $(AUDIO_FULL_LDFLAGS)

# C++ Examples
minimal-cxx: examples/minimal-cxx/main
examples/minimal-cxx/main: examples/minimal-cxx/main.cc fenster.h
	cd examples/minimal-cxx && $(CXX) main.cc -I../.. -o main $(CXXFLAGS) $(LDFLAGS)

# Go Examples (for indexing purposes, actual build handled by go)
.PHONY: minimal-go fire-go image-go
minimal-go:
	@echo "Go example: examples/minimal-go (use 'go run' to build)"

fire-go:
	@echo "Go example: examples/fire-go (use 'go run' to build)"

image-go:
	@echo "Go example: examples/image-go (use 'go run' to build)"

# Zig Examples (for indexing purposes)
.PHONY: minimal-zig
minimal-zig:
	@echo "Zig example: examples/minimal-zig (use 'zig build-exe' to build)"

# Doom example (complex build)
.PHONY: doom-c
doom-c: examples/doom-c/doomgeneric
examples/doom-c/doomgeneric: examples/doom-c/*.c examples/doom-c/*.h fenster.h
	cd examples/doom-c && $(MAKE) -f Makefile.fenster

# Clean targets
.PHONY: clean
clean:
	find examples -name "main" -type f -delete
	find examples -name "*.exe" -type f -delete
	find examples -name "doomgeneric" -type f -delete
	find examples -name "*.o" -type f -delete

# Install dependencies (for reference)
.PHONY: install-deps
install-deps:
	@echo "Install dependencies for your platform:"
	@echo "Ubuntu/Debian: sudo apt-get install build-essential libx11-dev libasound2-dev"
	@echo "Fedora/RHEL: sudo dnf install gcc-c++ libX11-devel alsa-lib-devel"
	@echo "macOS: xcode-select --install"
	@echo "Windows: Install MinGW-w64 or use Visual Studio"

# Help target
.PHONY: help
help:
	@echo "Fenster Build System"
	@echo "==================="
	@echo ""
	@echo "Targets:"
	@echo "  all          - Build all examples"
	@echo "  examples     - Build all examples"
	@echo "  minimal-c    - Build minimal C example"
	@echo "  minimal-cxx  - Build minimal C++ example"
	@echo "  drawing-c    - Build drawing C example"
	@echo "  input-c      - Build input C example"
	@echo "  sound-c      - Build sound C example"
	@echo "  doom-c       - Build Doom example"
	@echo "  clean        - Clean build artifacts"
	@echo "  install-deps - Show dependency installation instructions"
	@echo "  help         - Show this help"
	@echo ""
	@echo "Files for CLion indexing:"
	@echo "  fenster.h      - Main header file"
	@echo "  fenster_audio.h - Audio support header"
	@echo "  examples/      - Example implementations"

# Development targets for CLion
.PHONY: index-files
index-files:
	@echo "Header files for indexing:"
	@find . -name "*.h" -type f
	@echo ""
	@echo "Source files for indexing:"
	@find . -name "*.c" -type f
	@find . -name "*.cpp" -type f
	@find . -name "*.go" -type f
	@find . -name "*.zig" -type f

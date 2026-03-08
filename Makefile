# Choose build type: Debug or Release
BUILD_TYPE ?= Debug

#CMAKE_CC := -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc
#CMAKE_CXX := -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++


BUILD_DIR := build/$(BUILD_TYPE)

EXECUTABLE_DIR := $(BUILD_DIR)/bin

# Select executable
EXECUTABLE := DodoEditor

all: configure build

configure:
	@echo "Configuring CMake with Ninja ($(BUILD_TYPE))..."
	@if [ ! -d $(BUILD_DIR) ]; then mkdir -p $(BUILD_DIR); fi
	cd $(BUILD_DIR) && cmake -G Ninja \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DCMAKE_COLOR_DIAGNOSTICS=ON \
		$(CMAKE_CC) $(CMAKE_CXX) \
		$(CURDIR)

build:
	@echo "Building with Ninja ($(BUILD_TYPE))..."
	cd $(BUILD_DIR) && ninja

clean:
	@echo "Cleaning..."
	rm -rf $(BUILD_DIR)

debug:
	$(MAKE) BUILD_TYPE=Debug all

release:
	$(MAKE) BUILD_TYPE=Release all

run-debug:
	$(MAKE) BUILD_TYPE=Debug all
	@echo "Running Debug build..."
	cd $(EXECUTABLE_DIR) && ./$(EXECUTABLE)

run-release:
	$(MAKE) BUILD_TYPE=Release all
	@echo "Running Release build..."
	cd $(EXECUTABLE_DIR) && ./$(EXECUTABLE)

format:
	clang-format -i -style=file \
	$(shell find Dodo/src Sandbox/src DodoEditor/src Game/src \
	-type f \( -name "*.cpp" -o -name "*.h" \))

.PHONY: all configure build clean debug release
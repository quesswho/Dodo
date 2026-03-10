# Choose build type: Debug or Release
BUILD_TYPE ?= Debug

#CMAKE_CC := -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc
#CMAKE_CXX := -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++


BUILD_DIR := build

EXECUTABLE_DIR := $(BUILD_DIR)/bin

# Select executable
EXECUTABLE := DodoEditor

clean:
	@echo "Cleaning..."
	rm -rf $(BUILD_DIR)

run:
	@echo "Running Debug build..."
	cd $(EXECUTABLE_DIR)/Debug && ./$(EXECUTABLE)

run-release:
	@echo "Running Release build..."
	cd $(EXECUTABLE_DIR)/Release && ./$(EXECUTABLE)

format:
	clang-format -i -style=file \
	$(shell find Dodo/src Sandbox/src DodoEditor/src Game/src \
	-type f \( -name "*.cpp" -o -name "*.h" \))

.PHONY: all configure build clean debug release
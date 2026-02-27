# Choose build type: Debug or Release
BUILD_TYPE ?= Debug


BUILD_DIR := build/$(BUILD_TYPE)

EXECUTABLE_DIR := $(BUILD_DIR)/bin

# Select executable
EXECUTABLE := Sandbox

all: configure build

configure:
	@echo "Configuring CMake with Ninja ($(BUILD_TYPE))..."
	@if [ ! -d $(BUILD_DIR) ]; then mkdir -p $(BUILD_DIR); fi
	cd $(BUILD_DIR) && cmake -G Ninja \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DCMAKE_COLOR_DIAGNOSTICS=ON \
		$(CURDIR)

build:
	@echo "Building with Ninja ($(BUILD_TYPE))..."
	cd $(BUILD_DIR) && ninja -v

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

.PHONY: all configure build clean debug release
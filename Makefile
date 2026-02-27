BUILD_DIR := build

all: configure build

configure:
	@echo "Configuring CMake with Ninja..."
	@if [ ! -d $(BUILD_DIR) ]; then mkdir $(BUILD_DIR); fi
	cd $(BUILD_DIR) && cmake -G Ninja -DCMAKE_COLOR_DIAGNOSTICS=ON ..

build:
	@echo "Building with Ninja..."
	cd $(BUILD_DIR) && ninja -v

clean:
	@echo "Cleaning..."
	rm -rf $(BUILD_DIR)

.PHONY: all configure build clean
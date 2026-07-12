BUILD_DIR  := build
CMAKE_ARGS := -DCMAKE_BUILD_TYPE=Release

.PHONY: all configure build run test generate plots clean

all: build

configure:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake $(CMAKE_ARGS) ..

build: configure
	@cd $(BUILD_DIR) && make -j$$(nproc)
	@echo "\nBuild complete. Binaries in $(BUILD_DIR)/bin/"

run: build
	@echo "\nRunning cache simulation..."
	@./$(BUILD_DIR)/bin/cache_sim

test: build
	@echo "\nRunning test_anomaly..."
	@./$(BUILD_DIR)/bin/test_anomaly
	@echo "\nRunning test_pollution..."
	@./$(BUILD_DIR)/bin/test_pollution

generate:
	@echo "\nGenerating test sequences..."
	@python3 scripts/generate_testcases.py

plots: generate
	@echo "\nGenerating plots..."
	@python3 scripts/generate_plots.py
	@python3 scripts/generate_combined_plot.py
	@echo "Plots saved to results/plots/"

clean:
	@echo "Removing build directory..."
	@rm -rf $(BUILD_DIR)
	@echo "Done."

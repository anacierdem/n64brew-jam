all: jam.z64

V=1
SOURCE_DIR=src
BUILD_DIR=build
include $(N64_INST)/include/n64.mk

# DEBUG_FLAGS = -DNDEBUG

# TODO: simplify these
$(BUILD_DIR)/box2d/src/collision/%.o: box2d/src/collision/%.cpp
	@mkdir -p $(dir $@)
	@echo "    [CXX] $<"
	$(CXX) -c $(CXXFLAGS) -o $@ $<

$(BUILD_DIR)/box2d/src/common/%.o: box2d/src/common/%.cpp
	@mkdir -p $(dir $@)
	@echo "    [CXX] $<"
	$(CXX) -c $(CXXFLAGS) -o $@ $<

$(BUILD_DIR)/box2d/src/dynamics/%.o: box2d/src/dynamics/%.cpp
	@mkdir -p $(dir $@)
	@echo "    [CXX] $<"
	$(CXX) -c $(CXXFLAGS) -o $@ $<

$(BUILD_DIR)/box2d/src/rope/%.o: box2d/src/rope/%.cpp
	@mkdir -p $(dir $@)
	@echo "    [CXX] $<"
	$(CXX) -c $(CXXFLAGS) -o $@ $<

SRC = main.c rdl.c geometry.c
CXX_SRC = game.cpp rope.cpp box.cpp enemy.cpp hand.cpp blade.cpp $(wildcard box2d/src/collision/*.cpp) $(wildcard box2d/src/common/*.cpp) $(wildcard box2d/src/dynamics/*.cpp) $(wildcard box2d/src/rope/*.cpp)
OBJS = $(CXX_SRC:%.cpp=$(BUILD_DIR)/%.o) $(SRC:%.c=$(BUILD_DIR)/%.o) # order is important to init dfs
DEPS = $(SRC:%.c=$(BUILD_DIR)/%.d) $(CXX_SRC:%.cpp=$(BUILD_DIR)/%.d)

# Audio
assets = $(wildcard assets/*.wav)
assets_conv = $(addprefix filesystem/,$(notdir $(assets:%.wav=%.wav64)))
$(BUILD_DIR)/jam.dfs: $(assets_conv)

libdragon:
	$(MAKE) -C ./libdragon install

jam.z64: libdragon
jam.z64: N64_ROM_TITLE="Jam"
jam.z64: CFLAGS+=-Wno-error -Iinclude $(DEBUG_FLAGS)
jam.z64: CXXFLAGS+=-Wno-error -Iinclude -Ibox2d/include -Ibox2d/src $(DEBUG_FLAGS)
jam.z64: $(BUILD_DIR)/jam.dfs

filesystem/%.wav64: assets/%.wav
	@mkdir -p $(dir $@)
	@echo "    [AUDIO] $@"
	@$(N64_AUDIOCONV) -o filesystem $<

$(BUILD_DIR)/jam.elf: $(OBJS)

clean:
	$(MAKE) -C ./libdragon clean
	rm -rf $(BUILD_DIR)/* jam.z64 $(assets_conv)

-include $(DEPS)

.PHONY: all clean libdragon

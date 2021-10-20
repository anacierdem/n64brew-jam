all: jam.z64

V=1
SOURCE_DIR=src
BUILD_DIR=build
include $(N64_INST)/include/n64.mk

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
CXX_SRC = b2d.cpp $(wildcard box2d/src/collision/*.cpp) $(wildcard box2d/src/common/*.cpp) $(wildcard box2d/src/dynamics/*.cpp) $(wildcard box2d/src/rope/*.cpp)
OBJS = $(SRC:%.c=$(BUILD_DIR)/%.o) $(CXX_SRC:%.cpp=$(BUILD_DIR)/%.o)
DEPS = $(SRC:%.c=$(BUILD_DIR)/%.d)

libdragon:
	$(MAKE) -C ./libdragon install

jam.z64: libdragon
jam.z64: N64_ROM_TITLE="Jam"
jam.z64: CFLAGS=-Wno-error -Iinclude
jam.z64: CXXFLAGS=-Wno-error -Ibox2d/include -Ibox2d/src

$(BUILD_DIR)/jam.elf: $(OBJS)

clean:
	rm -rf $(BUILD_DIR)/* jam.z64

-include $(DEPS)

.PHONY: all clean libdragon

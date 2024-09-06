CCC := arm-linux-gnueabihf-gcc -std=gnu99
# CCC := gcc


# Find all sub-folders
DIR := $(shell find $(ROOT) -type d)
DIR := $(subst ./, , $(DIR))


# DIR := $(subst ., , $(DIR))

# Find all files in current folder and sub-folders
SRC := $(foreach dir, $(DIR), $(wildcard $(dir)/*.c))

# Include folders
INC := $(foreach dir, $(DIR), $(addprefix -I, $(dir)))

BUILD_PATH := ../API.img


# Convert source to object files
OBJ := $(patsubst %.c, $(BUILD_PATH)/%.o, $(SRC))

# Specify build directory
BUILD_DIR := $(foreach dir, $(DIR), $(addprefix $(BUILD_PATH)/, $(dir)))


LIBPATH   :=
LIBLINK   :=

# Build object files
$(BUILD_PATH)/%.o : %.c $(BUILD_DIR)
	$(CCC) -c $(INC) $< -o $@ $(LIBPATH) $(LIBLINK) -lpthread -g -lrt

# Build run file
sbl_tool.bin: $(OBJ)
	@echo pxhoang: $(DIR)
	$(CCC) $(OBJ) -o $@ $(LIBPATH) $(LIBLINK) -lpthread -g -lrt

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_PATH)
	mkdir -p $@

.PHONY: all clean
all: main
clean:
	rm -rf main $(BUILD_PATH)
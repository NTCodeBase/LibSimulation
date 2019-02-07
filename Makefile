################################################################################
COMPILER_PREFIX ?=
COMPILER_SUFFIX ?=
COMPILER_NAME   ?= g++

COMPILER    := $(COMPILER_PREFIX)$(COMPILER_NAME)$(COMPILER_SUFFIX)
ALL_CCFLAGS ?= -g -W -O3 -lstdc++fs -DNDEBUG -std=c++17 $(FLAG_FLTO)

################################################################################
ROOT_PATH := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
INCLUDES += -I$(ROOT_PATH)/
INCLUDES += -I$(ROOT_PATH)/../LibCommon
INCLUDES += -I$(ROOT_PATH)/../LibParticle
INCLUDES += -I$(ROOT_PATH)/../LibCommon/Externals/debugbreak
INCLUDES += -I$(ROOT_PATH)/../LibCommon/Externals/fmt/include
INCLUDES += -I$(ROOT_PATH)/../LibCommon/Externals/glm
INCLUDES += -I$(ROOT_PATH)/../LibCommon/Externals/json/single_include/nlohmann
INCLUDES += -I$(ROOT_PATH)/../LibCommon/Externals/spdlog/include
INCLUDES += -I$(ROOT_PATH)/../LibCommon/Externals/tbb_linux/include

################################################################################
OUTPUT_DIR := ../../Build/Linux
OBJ_DIR    := ../../Build/Linux/OBJS
LIB_NAME   := libLibSimulation.a

LIB_SRC     := $(shell find $(ROOT_PATH)/LibSimulation -name *.cpp)
LIB_OBJ     := $(patsubst %.cpp, %.o, $(LIB_SRC))
COMPILE_OBJ := $(patsubst $(ROOT_PATH)/%, $(OBJ_DIR)/%, $(LIB_OBJ))
COMPILE_OBJ_SUBDIR  := $(patsubst $(ROOT_PATH)/%, $(OBJ_DIR)/%, $(dir $(LIB_OBJ)))

################################################################################
all: create_out_dir $(OUTPUT_DIR)/$(LIB_NAME)

create_out_dir:
	mkdir -p $(OUTPUT_DIR)
	mkdir -p $(OBJ_DIR)
	mkdir -p $(COMPILE_OBJ_SUBDIR)

$(OUTPUT_DIR)/$(LIB_NAME): $(COMPILE_OBJ)
	$(GCC_PREFIX)gcc-ar$(GCC_SUFFIX) -rsv $(OUTPUT_DIR)/$(LIB_NAME) $(COMPILE_OBJ)
	$(GCC_PREFIX)gcc-ranlib$(GCC_SUFFIX) $(OUTPUT_DIR)/$(LIB_NAME)

$(COMPILE_OBJ): $(OBJ_DIR)/%.o: $(ROOT_PATH)/%.cpp
	$(COMPILER) $(INCLUDES) $(ALL_CCFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)/LibSimulation
	rm $(OUTPUT_DIR)/$(LIB_NAME)

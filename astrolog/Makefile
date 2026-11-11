# AstroLog Makefile
# Targets: all, run, test, clean

CC       := gcc
CXX      := g++
CFLAGS   := -std=c11 -Wall -Wextra -Wpedantic -O2 -Iinclude
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -O2 -Iinclude $(GTEST_INC)
GTEST_INC  ?=
GTEST_LIBS ?= -lgtest -lgtest_main -lpthread

SRC_DIR   := src
TEST_DIR  := tests
BUILD_DIR := build
DATA_DIR  := data

APP      := $(BUILD_DIR)/astrolog
TEST_BIN := $(BUILD_DIR)/run_tests

MAIN_OBJ     := $(BUILD_DIR)/main.o
MODULE_SRCS  := $(filter-out $(SRC_DIR)/main.c,$(wildcard $(SRC_DIR)/*.c))
MODULE_OBJS  := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(MODULE_SRCS))
TEST_SRCS    := $(wildcard $(TEST_DIR)/*.cpp)
TEST_OBJS    := $(patsubst $(TEST_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(TEST_SRCS))
DEPS         := $(MAIN_OBJ:.o=.d) $(MODULE_OBJS:.o=.d) $(TEST_OBJS:.o=.d)

.PHONY: all run test clean

all: $(APP)

$(APP): $(MAIN_OBJ) $(MODULE_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/%.o: $(TEST_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(TEST_BIN): $(TEST_OBJS) $(MODULE_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(GTEST_LIBS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# The first run copies the sample data so there is something to explore.
run: all
	@test -f $(DATA_DIR)/observations.txt || cp $(DATA_DIR)/sample_data.txt $(DATA_DIR)/observations.txt
	./$(APP) $(DATA_DIR)/observations.txt

test: $(TEST_BIN)
	./$(TEST_BIN)

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(DATA_DIR)/report.txt

-include $(DEPS)

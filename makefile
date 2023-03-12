CC:=g++
CPP_FLAGS:=-g -Wall -O3 -std=c++20 -std=gnu++20
.PHONY: e2e test clean
.DEFAULT_GOAL:=e2e

LIB_DIR:=src
TEST_DIR:=test
UNIT_TEST_DIR:=$(TEST_DIR)/unit
BUILD_DIR:=build
RMDIR_COMMAND=rm -f -r
# MKDIR_COMMAND=mkdir -p $(dir $<)
MK_BUILD_DIR_COMMAND=mkdir -p $(BUILD_DIR)/$(dir $<)

ifeq ($(OS), Windows_NT)
	RMDIR_COMMAND=if exist $(BUILD_DIR) rmdir /q /s
	# MKDIR_COMMAND=if not exist $(subst /,\,$(dir $<)) mkdir $(subst /,\,$(dir $<))
	MK_BUILD_DIR_COMMAND=if not exist $(BUILD_DIR)\$(subst /,\,$(dir $<)) mkdir $(BUILD_DIR)\$(subst /,\,$(dir $<))
endif

INCLUDES:=-include test/Arduino.h -Itest
DEFINES:=-D LOCAL_SSID=testSSID -D PASSPHRASE=testPhrase

DEPFLAGS=-MT $@ -MMD -MP -MF $(BUILD_DIR)/$*.d

E2E_SRCS:=$(wildcard $(TEST_DIR)/*.cpp) $(wildcard $(LIB_DIR)/utils/*series.cpp) $(wildcard $(LIB_DIR)/rower/*.cpp)
E2E_OBJS:=$(E2E_SRCS:%.cpp=$(BUILD_DIR)/%.o)

e2e: $(E2E_OBJS)
	@$(CC) $(CPP_FLAGS) $(INCLUDES) $(DEFINES) -o $(BUILD_DIR)/run_e2e_test.out $^

TEST_SRCS:=$(wildcard $(LIB_DIR)/utils/*series.cpp)  $(wildcard $(LIB_DIR)/rower/*.cpp) $(UNIT_TEST_DIR)/catch_amalgamated.cpp $(filter-out $(TEST_DIR)/main.cpp, $(wildcard $(TEST_DIR)/*.cpp)) $(wildcard $(TEST_SRC))

TEST_OBJS:=$(TEST_SRCS:%.cpp=$(BUILD_DIR)/%.o)

test: $(TEST_OBJS)
	@$(CC) $(CPP_FLAGS) $(INCLUDES) $(DEFINES) -o $(BUILD_DIR)/tests.out $^ 
	@./$(BUILD_DIR)/tests.out

$(BUILD_DIR)/%.o: %.cpp
	@$(MK_BUILD_DIR_COMMAND)
	@$(CC) $(CPP_FLAGS) $(INCLUDES) $(DEFINES) -c $< -o $@ $(DEPFLAGS)
	
clean:
	$(RMDIR_COMMAND) $(BUILD_DIR)

include $(wildcard $(TEST_OBJS:%.o=%.d))
include $(wildcard $(E2E_OBJS:%.o=%.d))
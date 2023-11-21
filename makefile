CC:=g++
CPP_FLAGS:=-ggdb -Og -Wall -std=c++20 -std=gnu++20
.PHONY: e2e test clean calibrate
.DEFAULT_GOAL:=e2e

LIB_DIR:=src
TEST_DIR:=test
UNIT_TEST_DIR:=$(TEST_DIR)/unit
BUILD_DIR:=build
RMDIR_COMMAND=rm -f -r
MK_BUILD_DIR_COMMAND=mkdir -p $(dir $<)
MAKE=make
TARGET_ENVIRONMENT=generic

ifeq ($(OS), Windows_NT)
	RMDIR_COMMAND=if exist $(BUILD_DIR) rmdir /q /s
	MK_BUILD_DIR_COMMAND=if not exist $(subst /,\,$(BUILD_DIR))\$(subst /,\,$(dir $<)) mkdir $(subst /,\,$(BUILD_DIR))\$(subst /,\,$(dir $<))
	MAKE=mingw32-make
endif

INCLUDES:=-include test/Arduino.h -Itest
override DEFINES+=-D LOCAL_SSID=testSSID -D PASSPHRASE=testPhrase

DEPFLAGS=-MT $@ -MMD -MP -MF $(BUILD_DIR)/$*.d

E2E_SRCS:=$(wildcard $(TEST_DIR)/*.cpp) $(wildcard $(LIB_DIR)/utils/*series.cpp) $(wildcard $(LIB_DIR)/rower/*.cpp)
E2E_OBJS:=$(E2E_SRCS:%.cpp=$(BUILD_DIR)/e2e/%.o)

e2e: BUILD_DIR:=$(BUILD_DIR)/e2e
e2e: $(E2E_OBJS)
	@$(CC) $(CPP_FLAGS) $(INCLUDES) $(DEFINES) -o $(BUILD_DIR)/run_e2e_test.out $^

TEST_SRCS:=$(wildcard $(LIB_DIR)/utils/*series.cpp) $(wildcard $(LIB_DIR)/rower/*.cpp) $(UNIT_TEST_DIR)/catch_amalgamated.cpp $(filter-out $(TEST_DIR)/main.cpp, $(wildcard $(TEST_DIR)/*.cpp)) $(wildcard $(TEST_SRC))

TEST_OBJS:=$(TEST_SRCS:%.cpp=$(BUILD_DIR)/test/%.o)

test: BUILD_DIR:=$(BUILD_DIR)/test
test: DEFINES+=-D UNIT_TEST
test: $(TEST_OBJS)
	@$(CC) $(CPP_FLAGS) $(INCLUDES) $(DEFINES) -o $(BUILD_DIR)/tests.out $^
	@./$(BUILD_DIR)/tests.out

$(BUILD_DIR)/test/%.o: %.cpp
	@$(MK_BUILD_DIR_COMMAND)
	@$(CC) $(CPP_FLAGS) $(INCLUDES) $(DEFINES) -c $< -o $@ $(DEPFLAGS)

$(BUILD_DIR)/e2e/%.o: %.cpp
	@$(MK_BUILD_DIR_COMMAND)
	@$(CC) $(CPP_FLAGS) $(INCLUDES) $(DEFINES) -c $< -o $@ $(DEPFLAGS)

calibrate:
ifeq ("$(wildcard $(BUILD_DIR)\e2e\$(TARGET_ENVIRONMENT))","")
	@echo Calibrateing new environment: $(TARGET_ENVIRONMENT)
	@echo Cleaning...
	@$(RMDIR_COMMAND) $(BUILD_DIR)
endif
	@test/calibration/$(TARGET_ENVIRONMENT)/run-test.bat
	@echo > $(BUILD_DIR)\e2e\$(TARGET_ENVIRONMENT)

clean:
	$(RMDIR_COMMAND) $(BUILD_DIR)

include $(wildcard $(TEST_OBJS:%.o=%.d))
include $(wildcard $(E2E_OBJS:%.o=%.d))
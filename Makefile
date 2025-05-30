CXX ?= g++
CXXFLAGS = -std=c++23 -Wall -Wextra -Wconversion -Wpedantic -Wnrvo -Werror -fanalyzer -fopenmp -flto=auto -fvisibility=hidden -pipe
RELFLAGS = -Ofast -march=native -Wno-analyzer-malloc-leak
DBGFLAGS = -DDEBUG -Og -ggdb3 -fsanitize=undefined,leak,address -Wno-analyzer-use-of-uninitialized-value
PFLFLAGS = -DDEBUG -Og -ggdb3 -Wno-analyzer-use-of-uninitialized-value -Wno-analyzer-malloc-leak
NOPTFLAGS = -Og

TST_DIR = ./test
INC_DIR = ./inc
BIN_DIR = ./bin
DBG_BIN_DIR = ./dbg_bin

TST_SRCS = $(wildcard $(TST_DIR)/*.cc)
TST_SRCS += $(wildcard $(TST_DIR)/*.cpp)
INCS = $(wildcard $(INC_DIR)/*.h)
INCS += $(wildcard $(INC_DIR)/*.hpp)

.PHONY: all
all: CXXFLAGS += $(RELFLAGS)
all: format build
.PHONY: nopt
nopt: CXXFLAGS += $(NOPTFLAGS)
nopt: format build
TST_TGTS = $(patsubst %.cpp,$(BIN_DIR)/%,$(notdir $(TST_SRCS)))
build: $(TST_TGTS) $(INCS)
$(BIN_DIR)/%: $(TST_DIR)/%.cpp $(INCS)
	$(CXX) -I $(INC_DIR) $(CXXFLAGS) $< -o $@
$(BIN_DIR)/%: $(TST_DIR)/%.cc $(INCS)
	$(CXX) -I $(INC_DIR) $(CXXFLAGS) $< -o $@

PFP_TST_TGT = $(DBG_BIN_DIR)/profile
__DBG_TST_TGTS = $(patsubst %.cpp,$(DBG_BIN_DIR)/%,$(notdir $(TST_SRCS)))
DBG_TST_TGTS = $(filter-out $(PFP_TST_TGT),$(__DBG_TST_TGTS))
.PHONY: debug
debug: CXXFLAGS += $(DBGFLAGS)
debug: format
debug: $(DBG_TST_TGTS)
$(DBG_BIN_DIR)/%: $(TST_DIR)/%.cpp $(INCS)
	$(CXX) -I $(INC_DIR) $(CXXFLAGS) $< -o $@
$(DBG_BIN_DIR)/%: $(TST_DIR)/%.cc $(INCS)
	$(CXX) -I $(INC_DIR) $(CXXFLAGS) $< -o $@
.PHONY: profile
profile: CXXFLAGS += $(PFLFLAGS)
profile: format
profile: $(PFP_TST_TGT)
$(PFP_TST_TGT): $(TST_DIR)/profile.cpp $(INCS)
	$(CXX) -I $(INC_DIR) $(CXXFLAGS) $< -o $@

.PHONY: format
format: $(SRCS) $(TST_SRCS) $(TST_HDRS) $(INCS)
	clang-format -i $(TST_SRCS) $(INCS)

.PHONY: clean
clean:
	rm -rf $(TST_TGTS) $(DBG_TST_TGTS) $(PFP_TST_TGT)

.PHONY: help
help:
	@echo "[USAGE] make <target>"
	@echo "Please choose one of the following targets:"
	@echo "    all (default): Builds the code in release mode"
	@echo "    nopt         : Builds the code without any optimizations"
	@echo "    debug        : Builds the code in debug mode"
	@echo "    profile      : Builds the code for VTune profiling"
	@echo "    clean        : Deletes all the build files (binaries)"
	@echo "    format       : Formats the code using clang-format"
	@echo "    help         : Prints out this help message"

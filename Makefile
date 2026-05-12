SRC_DIR       = source
INC_DIR       = include
BASE_OBJ_DIR  = obj
BIN_DIR       = bin
SCRIPTS_DIR   = scripts
REPORTS_DIR   = reports
IMAGES_DIR    = images
DATA_DIR 	  = data
UPROF_DIR  	  = /opt/AMDuProf_5.2-606

CC 			  = g++
CXXFLAGS      = -I$(INC_DIR) -Wall -Wextra -Werror \
				-march=native -g -I$(UPROF_DIR)/include

LDFLAGS       = -L$(UPROF_DIR)/lib/x64/ 
LDLIBS 		  = -lAMDProfileController -lrt -pthread

ASMFLAGS 	  = -masm=intel -march=native
ASM_OFILES    = 

GENFLAGS 	  = -I$(INC_DIR) -O3 

BENCH_TARGET_FILE   := bench
GEN_TARGET_FILE     = gen

ifeq ($(LOGS), OFF)
	CXXFLAGS += -DDISABLE_LOGS
endif 

ifeq ($(DEBUG), OFF)
	CXXFLAGS += -DNDEBUG -DDISABLE_LOGS
endif 

ifeq ($(INDIRECT), ON)
	OPTI = DEFAULT
    CXXFLAGS += -DINDIRECT
    BENCH_TARGET_FILE := $(BENCH_TARGET_FILE)_indirect
endif 

ifdef OPTI
	undefine LIST_TYPE
    ifeq ($(OPTI), DEFAULT)
        undefine HASH
        undefine CMP
		BENCH_TARGET_FILE := $(BENCH_TARGET_FILE)_default
	else ifeq ($(OPTI), LEVEL0)
		CXXFLAGS += -O3
		undefine HASH
		undefine CMP
		BENCH_TARGET_FILE := $(BENCH_TARGET_FILE)_level0
    else ifeq ($(OPTI), LEVEL1)
        CXXFLAGS += -O3
		HASH = hash
		undefine CMP
		undefine LEN
		BENCH_TARGET_FILE := $(BENCH_TARGET_FILE)_level1
    else ifeq ($(OPTI), LEVEL2)
        CXXFLAGS += -O3
        HASH = hash
		CMP = inline
		undefine LEN
		BENCH_TARGET_FILE := $(BENCH_TARGET_FILE)_level2
	else ifeq ($(OPTI), LEVEL3)
        CXXFLAGS += -O3
        HASH = hash
		CMP = inline
		LEN = inline
		BENCH_TARGET_FILE := $(BENCH_TARGET_FILE)_level3
    endif
endif

ifdef HASH
    CXXFLAGS += -DHASH_INTR
	ifndef OPTI
		BENCH_TARGET_FILE := $(BENCH_TARGET_FILE)_intr
	endif
endif 

ifdef LEN
	ifeq ($(LEN), inline)
		CXXFLAGS += -DSTRLEN_INLINE
		ifndef OPTI
			BENCH_TARGET_FILE := $(BENCH_TARGET_FILE)_inlinelen
		endif
	else 
		CXXFLAGS += -DSTRLEN_ASM
		ifndef OPTI
			BENCH_TARGET_FILE := $(BENCH_TARGET_FILE)_$(LEN)
		endif
	endif
endif 

ifdef CMP
    ifeq ($(CMP), inline)
        CXXFLAGS += -DSTRCMP_INLINE
		ifndef OPTI
			BENCH_TARGET_FILE := $(BENCH_TARGET_FILE)_inlinecmp
		endif
    else
        CXXFLAGS += -DSTRCMP_ASM
		ifndef OPTI
			BENCH_TARGET_FILE := $(BENCH_TARGET_FILE)_$(CMP)
		endif
    endif
endif 

OBJ_DIR := $(BASE_OBJ_DIR)/$(BENCH_TARGET_FILE)

ifdef LEN
	ifneq ($(LEN), inline)
		ASM_OFILES += $(OBJ_DIR)/asm/$(LEN).o
	endif
endif 

ifdef CMP
    ifneq ($(CMP), inline)
        ASM_OFILES += $(OBJ_DIR)/asm/$(CMP).o
    endif
endif

ifdef LIST_TYPE
	ifeq ($(LIST_TYPE), CF)
		CXXFLAGS += -DCF_LIST
		CONTAINER_NAME = cf_list
	else ifeq ($(LIST_TYPE), STD)
		CXXFLAGS += -DSTD_LIST
		CONTAINER_NAME = std_list
	else ifeq ($(LIST_TYPE), CLASSIC)
		CXXFLAGS += -DCLASSIC_LIST
		CONTAINER_NAME = classic_list
	endif
endif

CONTAINER_NAME ?= array

# Функция для получения объектных файлов на основе .cpp файлов из поддиректорий $(SRC_DIR)
# В качестве единственного аргумента передается поддиректория в $(SRC_DIR)
get_cxx_objects  = $(patsubst $(SRC_DIR)/$(1)/%.cpp,$(OBJ_DIR)/$(1)/%.o,$(wildcard $(SRC_DIR)/$(1)/*.cpp))

COMMON_OFILES  = $(call get_cxx_objects,common)
TABLE_OFILES   = $(call get_cxx_objects,table)
TESTING_OFILES = $(call get_cxx_objects,testing)
CONTAINERS_OFILES = $(filter %$(CONTAINER_NAME).o, $(call get_cxx_objects,containers))

OFILES         = $(COMMON_OFILES) $(TABLE_OFILES) $(TESTING_OFILES) $(CONTAINERS_OFILES)

GEN_OFILES     = $(call get_cxx_objects,generator)

.PHONY: all run gen clean

all: $(OFILES) $(ASM_OFILES) | $(BIN_DIR)
	@$(CC) $(CXXFLAGS) $^ -o $(BIN_DIR)/$(BENCH_TARGET_FILE) $(LDFLAGS) $(LDLIBS)

run: 
	@./$(BIN_DIR)/$(TARGET_FILE)

gen: $(GEN_OFILES) | $(BIN_DIR)
	@$(CC) $^ -o $(BIN_DIR)/$(GEN_TARGET_FILE)

clean:
	@rm -rf $(BIN_DIR)
	@rm -rf $(BASE_OBJ_DIR)
	@rm -rf $(REPORTS_DIR)
	@rm -rf $(IMAGES_DIR)

$(GEN_OFILES): $(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	@$(CC) $(GENFLAGS) -c $< -o $@

$(OFILES): $(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	@$(CC) $(CXXFLAGS) -c $< -o $@

$(ASM_OFILES): $(OBJ_DIR)/%.o: $(SRC_DIR)/%.s | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	@$(CC) $(ASMFLAGS) -c $< -o $@

$(OBJ_DIR) $(BIN_DIR):
	@mkdir -p $@

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
CFLAGS     	  = -I$(INC_DIR) -Wall -Wextra -Werror \
				-march=native -g -I$(UPROF_DIR)/include

LDFLAGS       = -L$(UPROF_DIR)/lib/x64/ 
LDLIBS 		  = -lAMDProfileController -lrt -pthread

ASMFLAGS 	  = -masm=intel -march=native
ASM_OFILES    = 

GENFLAGS 	  = -I$(INC_DIR) -O3 

BENCH_TARGET_FILE   := bench
GEN_TARGET_FILE     = gen

ifeq ($(INDIRECT), ON)
	OPTI = DEFAULT
    CFLAGS += -DINDIRECT
    BENCH_TARGET_FILE := $(BENCH_TARGET_FILE)_indirect
endif 

ifdef OPTI
    ifeq ($(OPTI), DEFAULT)
        undefine HASH
        undefine CMP
		BENCH_TARGET_FILE := $(BENCH_TARGET_FILE)_default
	else ifeq ($(OPTI), LEVEL0)
		CFLAGS += -O3
		undefine HASH
		undefine CMP
		BENCH_TARGET_FILE := $(BENCH_TARGET_FILE)_level0
    else ifeq ($(OPTI), LEVEL1)
        CFLAGS += -O3
		undefine HASH
		CMP = inline
		undefine LEN
		BENCH_TARGET_FILE := $(BENCH_TARGET_FILE)_level1
    else ifeq ($(OPTI), LEVEL2)
        CFLAGS += -O3
        HASH = hash
		CMP = inline
		undefine LEN
		BENCH_TARGET_FILE := $(BENCH_TARGET_FILE)_level2
	else ifeq ($(OPTI), LEVEL3)
        CFLAGS += -O3
        HASH = hash
		CMP = inline
		LEN = inline
		BENCH_TARGET_FILE := $(BENCH_TARGET_FILE)_level3
    endif
endif

ifdef HASH
    CFLAGS += -DHASH_INTR
	ifndef OPTI
		BENCH_TARGET_FILE := $(BENCH_TARGET_FILE)_intr
	endif
endif 

ifdef LEN
	ifeq ($(LEN), inline)
		CFLAGS += -DSTRLEN_INLINE
		ifndef OPTI
			BENCH_TARGET_FILE := $(BENCH_TARGET_FILE)_inlinelen
		endif
	else 
		CFLAGS += -DSTRLEN_ASM
		ifndef OPTI
			BENCH_TARGET_FILE := $(BENCH_TARGET_FILE)_$(LEN)
		endif
	endif
endif 

ifdef CMP
    ifeq ($(CMP), inline)
        CFLAGS += -DSTRCMP_INLINE
		ifndef OPTI
			BENCH_TARGET_FILE := $(BENCH_TARGET_FILE)_inlinecmp
		endif
    else
        CFLAGS += -DSTRCMP_ASM
		ifndef OPTI
			BENCH_TARGET_FILE := $(BENCH_TARGET_FILE)_$(CMP)
		endif
    endif
endif 

OBJ_DIR := $(BASE_OBJ_DIR)/$(BENCH_TARGET_FILE)

ifdef LEN
	ifneq ($(LEN), inline)
		ASM_OFILES += $(OBJ_DIR)/common/$(LEN).o
	endif
endif 

ifdef CMP
    ifneq ($(CMP), inline)
        ASM_OFILES += $(OBJ_DIR)/common/$(CMP).o
    endif
endif

# Функция для получения объектных файлов на основе .c файлов из поддиректорий $(SRC_DIR)
# В качестве единственного аргумента передается поддиректория в $(SRC_DIR)
get_c_objects  = $(patsubst $(SRC_DIR)/$(1)/%.c,$(OBJ_DIR)/$(1)/%.o,$(wildcard $(SRC_DIR)/$(1)/*.c))

COMMON_OFILES  = $(call get_c_objects,common)
TABLE_OFILES   = $(call get_c_objects,table)
TESTING_OFILES = $(call get_c_objects,testing)
ERROR_OBJ      = $(filter %report.o $(COMMON_OFILES))

OFILES         = $(COMMON_OFILES) $(TABLE_OFILES) $(TESTING_OFILES)

GEN_OFILES     = $(call get_c_objects,generator)

ifeq ($(LOGS), OFF)
	CFLAGS += -DDISABLE_LOGS
endif 

ifeq ($(DEBUG), OFF)
	CFLAGS += -DNDEBUG -DDISABLE_LOGS
endif 

.PHONY: all run clean

all: $(OFILES) $(ASM_OFILES) | $(BIN_DIR)
	@$(CC) $(CFLAGS) $^ -o $(BIN_DIR)/$(BENCH_TARGET_FILE) $(LDFLAGS) $(LDLIBS)

run: 
	@./$(BIN_DIR)/$(TARGET_FILE)

gen: $(GEN_OFILES) | $(BIN_DIR)
	@$(CC) $^ -o $(BIN_DIR)/$(GEN_TARGET_FILE)

clean:
	@rm -rf $(BIN_DIR)
	@rm -rf $(BASE_OBJ_DIR)
	@rm -rf $(REPORTS_DIR)
	@rm -rf $(IMAGES_DIR)

$(GEN_OFILES): $(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	@$(CC) $(GENFLAGS) -c $< -o $@

$(OFILES): $(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

$(ASM_OFILES): $(OBJ_DIR)/%.o: $(SRC_DIR)/%.s | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	@$(CC) $(ASMFLAGS) -c $< -o $@

$(OBJ_DIR) $(BIN_DIR):
	@mkdir -p $@

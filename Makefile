SRC_DIR       = source
INC_DIR       = include
OBJ_DIR       = obj
BIN_DIR       = bin
SCRIPTS_DIR   = scripts
ERRORS_DIR   = reports
IMAGES_DIR    = images
DATA_DIR 	  = data
SUBMODULE_DIR = Array_based_list

CC 			  = gcc
CFLAGS     	  = -I$(INC_DIR) -Wall -Wextra -Werror\
				-O3 -march=native -g
ASMFLAGS 	  = -masm=intel -march=native

GENFLAGS 	  = -I$(INC_DIR) -O3 

BENCH_TARGET_FILE   = bench
GEN_TARGET_FILE     = gen

# DATA_GEN_SCRIPT = data_generator.sh
# PLOT_GEN_SCRIPT = plot_generator.py

# Функция для получения объектных файлов на основе .c файлов из поддиректорий $(SRC_DIR)
# В качестве единственного аргумента передается поддиректория в $(SRC_DIR)
get_c_objects  = $(patsubst $(SRC_DIR)/$(1)/%.c,$(OBJ_DIR)/$(1)/%.o,$(wildcard $(SRC_DIR)/$(1)/*.c))

COMMON_OFILES  = $(call get_c_objects,common)
TABLE_OFILES   = $(call get_c_objects,table)
TESTING_OFILES = $(call get_c_objects,testing)
ERROR_OBJ      = $(filter %report.o $(COMMON_OFILES))

OFILES         = $(COMMON_OFILES) $(TABLE_OFILES) $(TESTING_OFILES)
ASM_OFILES     = 

GEN_OFILES     = $(call get_c_objects,generator)

ifeq ($(LOGS), OFF)
	CFLAGS += -DDISABLE_LOGS
endif 

ifeq ($(FASTCMP), ON)
	ASM_OFILES += $(OBJ_DIR)/common/strcmp.o
	CFLAGS += -DMY_STRCMP
	BENCH_TARGET_FILE = bench_fastcmp
endif 

ifeq ($(DEBUG), OFF)
	CFLAGS += -DNDEBUG -DDISABLE_LOGS
endif 

.PHONY: all run clean

all: $(OFILES) $(ASM_OFILES) | $(BIN_DIR)
	@$(CC) $(CFLAGS) $^ -o $(BIN_DIR)/$(BENCH_TARGET_FILE)

run: 
	@./$(BIN_DIR)/$(TARGET_FILE)

gen: $(GEN_OFILES) | $(BIN_DIR)
	@$(CC) $^ -o $(BIN_DIR)/$(GEN_TARGET_FILE)

clean:
	@rm -rf $(BIN_DIR)
	@rm -rf $(OBJ_DIR)

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

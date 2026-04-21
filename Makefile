SRC_DIR       = source
INC_DIR       = include
OBJ_DIR       = obj
BIN_DIR       = bin
SCRIPTS_DIR   = scripts
REPORTS_DIR   = reports
IMAGES_DIR    = images
DATA_DIR 	  = data
SUBMODULE_DIR = Array_based_list

CC 			  =    gcc
CFLAGS     	  = -I$(INC_DIR) -I$(SUBMODULE_DIR) -Wall -Wextra -Werror

TARGET_FILE 	  = table.out

TARGET_GENERATOR = hash_table
# DATA_GEN_SCRIPT = data_generator.sh
# PLOT_GEN_SCRIPT = plot_generator.py

# Функция для получения объектных файлов на основе .c файлов из поддиректорий $(SRC_DIR)
# В качестве единственного аргумента передается поддиректория в $(SRC_DIR)
get_objects = $(patsubst $(SRC_DIR)/$(1)/*.c,$(OBJ_DIR)/$(1)/*.o,$(wildcard $(SRC_DIR)/$(1)/*.c))

COMMON_FILES = $(call get_objects,common)
TABLE_FILES  = $(call get_objects,table)

REPORT_OBJ = $(filter %report.o $(COMMON_FILES))


.PHONY: all run

all: | $(BIN_DIR)
	@$(CC) $(CFLAGS) $(SRC_DIR)/table/table.c $(SRC_DIR)/testing/main.c $(SRC_DIR)/common/report.c -o $(BIN_DIR)/$(TARGET_FILE)

run: 
	@./$(BIN_DIR)/$(TARGET_FILE)

$(OBJ_DIR) $(BIN_DIR):
	@mkdir -p $@
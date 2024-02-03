
CFLAGS	+= -I$(PROJECT_DIR)/source
VPATH 	+=   $(PROJECT_DIR)/source

-include $(PROJECT_DIR)/source/sdk/Makefile.mk

OBJ += $(OBJ_DIR)/main.o

VERSION=1.0.0

TOOLS_DIR	=
NAME_MODULE	= mtce-dev
NAME_LIB	= 
OPTIMIZE	= -Wall -O2
PROJECT_DIR = $(PWD)

OBJ_DIR=${PROJECT_DIR}/build

-include $(PROJECT_DIR)/Makefile.conf
-include $(PROJECT_DIR)/Makefile.sdk.conf

CFLAGS +=$(FLAG_HAVE)

# linux
CROSS_COMPILE=

CXX	  =$(CROSS_COMPILE)g++
CC	  =$(CROSS_COMPILE)gcc
AR	  =$(CROSS_COMPILE)ar
STRIP =$(CROSS_COMPILE)strip

-include $(PROJECT_DIR)/source/Makefile.mk


CFLAGS += $(OPTIMIZE)
CFLAGS += -ldl -lpthread -lrt -g 

# libs
CFLAGS +=-I$(PROJECT_DIR)/libs/inc


# dependencies
CFLAGS +=-I$(PROJECT_DIR)/deps/inc


.PHONY : all flash create clean

all: create $(OBJ_DIR)/$(NAME_MODULE)

create:
	@echo Create
	@echo ${CFLAGS}
	mkdir -p $(OBJ_DIR)

flash: 
	@echo $(BLUE) "--flash to nfs !"
	@date
	@sudo cp -r $(OBJ_DIR)/$(NAME_MODULE) $(INSTALL_DIR)

clean:
	@echo clean 
	@rm -rf $(OBJ_DIR)


$(OBJ_DIR)/%.o: %.c $(HDRS)
	@echo $(BLUE) CC $< $(HAVE_FLAG) $(NONE)
ifneq ($(MT_TYPE),MT_TYPE_WINDOWS)
	@$(CC) -c -o $@ $< $(CFLAGS) $(LDFLAGS) $(HAVE_FLAG)
endif

$(OBJ_DIR)/$(NAME_MODULE): $(OBJ)
	@echo ---------- BUILD PROJECT ----------

	@$(CC) -o $(OBJ_DIR)/$(NAME_MODULE) $^ $(CFLAGS) $(LDFLAGS) $(LALIBS) 
ifdef CONFIG_BUILD_DEBUG
	@echo $(RED)"--Compiling '$(NAME_MODULE) build debug'..."$(NONE)
else
	@echo $(GREEN)"--Compiling '$(NAME_MODULE) build release'..."$(NONE)
endif

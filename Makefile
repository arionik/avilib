AR = ar
RANLIB = ranlib
CFLAGS = -Wall -Wno-unknown-pragmas

ifeq ($(findstring __x86_64__, $(shell echo | $(CC) -dM -E -x c -)), __x86_64__)
	BITS=64
else
	BITS=
endif

TARGET=$(shell $(CC) -dumpmachine)

INSTALL_DIR=/usr/local/lib
INSTALL_HEADER_DIR=/usr/local/include

ifeq ($(findstring darwin, $(TARGET)), darwin)
	OS=darwin
	CFLAGS += -fPIC -fno-strict-aliasing
	CFLAGS += -std=c++11 -Wno-incompatible-pointer-types-discards-qualifiers
	C1=
	C2=033
else
ifeq ($(findstring linux, $(TARGET)), linux)
	OS=linux
	CFLAGS += -fPIC -fno-strict-aliasing -Wno-multichar
	CFLAGS += -std=gnu++11 
	OPERATING_SYSTEM=$(shell uname -a)
	ifeq ($(findstring Ubuntu, $(OPERATING_SYSTEM)), Ubuntu)
		C1=
		C2=033
	else
		C1=-e
		C2=e
	endif
endif
endif

ARCH = $(OS)_$(BITS)

INC_DIR = ./
SRC_DIR = ./

INC_DIRS = $(addprefix -I, $(INC_DIR))

ifeq ($(findstring debug, $(MAKECMDGOALS)), debug)
	LIB = libavi$(BITS)d.a
	CFLAGS += -ggdb -O0 -D_DEBUG
	OBJ_DIR = ./build/$(ARCH)/debug/
	LIB_DIR = ../bin/
else
	LIB = libavi$(BITS).a
	CFLAGS += -Ofast -DNDEBUG 
	OBJ_DIR = ./build/$(ARCH)/release/
	LIB_DIR = ../bin/
endif

SRC_FILES = $(patsubst %.cpp, %.o, $(filter-out main.cpp, $(wildcard *.cpp)))
OBJ_FILES  = $(addprefix $(OBJ_DIR), $(SRC_FILES:.cpp=.o))


###############################
# Building Library
###############################

$(LIB): init $(OBJ_FILES) $(OBJ_DIR)main.o
	@echo  "Creating $(LIB) ..."
	@$(AR) -rc $(LIB_DIR)$(LIB) $(OBJ_FILES)
	@$(RANLIB) $(LIB_DIR)$(LIB)
	@if test -f $(LIB_DIR)$(LIB); then echo $(C1) "\$(C2)[92mLib <$(LIB_DIR)$(LIB)> built\$(C2)[39m"; else echo $(C1) "\$(C2)[31mLib <$(LIB_DIR)$(LIB)> failed\$(C2)[39m"; fi
	@$(CXX) $(OBJ_DIR)main.o $(CFLAGS) -o $(LIB_DIR)aviapp -L$(LIB_DIR) -lavi$(BITS)
	@if test -f $(LIB_DIR)aviapp; then echo $(C1) "\$(C2)[92mLib <$(LIB_DIR)aviapp> built\$(C2)[39m"; else echo $(C1) "\$(C2)[31mLib <$(LIB_DIR)aviapp> failed\$(C2)[39m"; fi
	

$(OBJ_DIR)%.o:$(SRC_DIR)%.cpp
	@echo " CC $<"
	@$(CXX) -c $(CFLAGS) $(INC_DIRS) $< -o $@

$(OBJ_DIR)main.o:$(SRC_DIR)main.cpp
	@echo " CC $<"
	@$(CXX) -c $(CFLAGS) $(INC_DIRS) $< -o $@

###############################
# Phony stuff
###############################
init:
	@if test ! -d $(OBJ_DIR) ; then mkdir -vp $(OBJ_DIR); fi
	@if test ! -d $(LIB_DIR) ; then mkdir -vp $(LIB_DIR); fi

clean:
	@echo "Cleaning library and object files"
	@$(RM) $(LIB_DIR)lib*	
	@$(RM) $(LIB_DIR)aviapp	
	@rm -rf  $(OBJ_DIR)

all:	init $(LIB)
debug:	init $(LIB)
install:all
	@install $(LIB_DIR)libavi$(BITS).a $(INSTALL_DIR)/libavi.a
	@install avilib.h $(INSTALL_HEADER_DIR)/
	@if test -f $(INSTALL_DIR)/libavi.a; then echo $(C1) "Installed \$(C2)[92m$(INSTALL_DIR)/libavi.a\$(C2)[39m"; else echo $(C1) "Install of \$(C2)[31m$(INSTALL_DIR)/libavi.a failed\$(C2)[39m"; fi


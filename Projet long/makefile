## COMPILER ##
CXX = g++ -g
RM = rm -if
XENO_CONFIG = /usr/xenomai/bin/xeno-config
XENO_CFLAGS = $(shell $(XENO_CONFIG) --alchemy --posix --cflags) #--skin native
XENO_LDFLAGS = $(shell $(XENO_CONFIG) --alchemy --posix --ldflags)

## PROJECT ##
EXEC_NAME = MoCoAgent.out
INSTALL_DIR = .

## DIRECTORIES ##
OBJ_DIR = bin
SRC_DIR = src
HDR_DIR = include
LIB_DIR = include/libs
# INCLUDES =  # to include other makefiles
LIBS = #-lboost_system -lboost_chrono -lpthread -lboost_thread

CXXFLAGS = $(XENO_CFLAGS) -Wall -std=c++11 -I $(HDR_DIR) -I $(LIB_DIR)
LDFLAGS = $(XENO_LDFLAGS) $(LIBS)

## FILES & FOLDERS ##
HEADER_FILES = $(shell find $(HDR_DIR) -name "*.h")
SRC_FILES = $(shell find ./src -name "*.cpp" -print -o -name "*.c" -print)
SRC_DIRS = $(shell find . -name "*.cpp" | dirname {} | sort | uniq | sed 's/\/$(SRC_DIR)//g')

TMP = $(patsubst %.cpp, %.o, $(SRC_FILES))
OBJ_FILES = $(subst $(SRC_DIR),$(OBJ_DIR), $(TMP))
#OBJ_FILES = $(shell find $(OBJ_DIR)/*.o)
OUT_FILES = $(shell find ./bench/output/*)

## TARGETS ##
all : $(EXEC_NAME)

clean :
	$(RM) $(OBJ_FILES)
	#$(RM) $(TMP)

cleanout :
	$(RM) $(OUT_FILES)

$(EXEC_NAME) : $(OBJ_FILES)
	$(CXX) $(CXXFLAGS) -o $(EXEC_NAME) $(OBJ_FILES) $(LDFLAGS)
	find . -name \*.gch -type f -exec rm -f {} +

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ -c $<

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cc
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ -c $<

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	gcc $(CXXFLAGS) $(INCLUDES) -o $@ -c $<

install :
	cp $(OBJ_DIR)/$(EXEC_NAME) $(INSTALL_DIR)

run :
	./$(EXEC_NAME)

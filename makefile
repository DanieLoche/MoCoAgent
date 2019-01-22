## COMPILER ##
CXX = g++
RM = rm -f
CXXFLAGS = -Wall -std=c++11 -I $(HDR_DIR)

## PROJECT ##
EXEC_NAME = SchedulerSimulator
INSTALL_DIR = .

## DIRECTORIES ##
OBJ_DIR = bin
SRC_DIR = src
HDR_DIR = include
# INCLUDES =  # to include other makefiles
LIBS = -lboost_system -lboost_chrono -lpthread -lboost_thread

## FILES & FOLDERS ##
HEADER_FILES = $(shell find $(HDR_DIR) -name "*.h")
SRC_FILES = $(shell find $(SRC_DIR) -name "*.cpp")
SRC_DIRS = $(shell find . -name "*.cpp" | dirname {} | sort | uniq | sed 's/\/$(SRC_DIR)//g')
# OBJ_FILES = $(patsubst %.cpp, %.o, $(SRC_FILES))
OBJ_FILES = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC_FILES))

## TARGETS ##
all : $(EXEC_NAME)

clean :
	$(RM) $(OBJ_FILES)

$(EXEC_NAME) : $(OBJ_FILES)
	mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(EXEC_NAME) $(OBJ_FILES) $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ -c $<

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cc
	$(CXX) $(CFLAGS) $(INCLUDES) -o $@ -c $<

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	gcc $(CFLAGS) $(INCLUDES) -o $@ -c $<

install :
	cp $(OBJ_DIR)/$(EXEC_NAME) $(INSTALL_DIR)

run :
	./$(EXEC_NAME)

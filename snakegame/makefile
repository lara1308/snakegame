CXX      = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -I./include
LDFLAGS  = -lncurses

SRC_DIR = src
BIN_DIR = bin

SRCS = $(SRC_DIR)/main.cpp \
       $(SRC_DIR)/map.cpp  \
       $(SRC_DIR)/snake.cpp \
       $(SRC_DIR)/gate.cpp  \
       $(SRC_DIR)/contents.cpp \
       $(SRC_DIR)/game.cpp

OBJS   = $(SRCS:$(SRC_DIR)/%.cpp=$(BIN_DIR)/%.o)
TARGET = snakegame

all: $(BIN_DIR) $(TARGET)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(BIN_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(BIN_DIR)/*.o $(TARGET)

.PHONY: all clean

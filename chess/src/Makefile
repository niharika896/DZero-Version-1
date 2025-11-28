CXX = g++
CXXFLAGS = -O3 -std=c++17

SRC_DIR = .
CORE_DIR = ./core
TABLE_DIR = ./core/PreComputedTables

SRCS = \
    $(CORE_DIR)/Game.cpp \
    $(CORE_DIR)/Bitboard.cpp \
    $(TABLE_DIR)/PreComputed.cpp

TARGET = chess

all:
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET)

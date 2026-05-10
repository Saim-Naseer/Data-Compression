# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude


ifeq ($(OS),Windows_NT)
    TARGET = app.exe
    RM = del /Q
    EXEC = $(TARGET)
else
    TARGET = app
    RM = rm -f
    EXEC = ./$(TARGET)
endif

SRC = \
	src/test.cpp \
	src/Block.cpp \
	src/BWT.cpp \
	src/RLE_1.cpp \
	src/MTF.cpp \
	src/RLE_2.cpp \
	src/Huffman.cpp

# Object files
OBJ = $(SRC:.cpp=.o)


all: $(TARGET)


$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^


src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@


clean:
ifeq ($(OS),Windows_NT)
	-$(RM) src\*.o $(TARGET) 2>nul
else
	$(RM) src/*.o $(TARGET)
endif


run: $(TARGET)
	$(EXEC)


windows:
	x86_64-w64-mingw32-g++ $(CXXFLAGS) -o app.exe $(SRC)
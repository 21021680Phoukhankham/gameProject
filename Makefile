CXX = g++
CXXFLAGS = -Iinclude/SDL2
LDFLAGS = -Llib
LDLIBS = -lSDL2main -lSDL2 -lSDL2_image

SRC = $(wildcard *.cpp)
OBJ = $(SRC:.cpp=.o)
TARGET = main

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	del /Q *.o *.exe

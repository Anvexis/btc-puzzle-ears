CXX      = g++
CXXFLAGS = -O3 -march=native -std=c++17 -pthread -Wall -Wno-deprecated-declarations
LDFLAGS  = -lsecp256k1 -lssl -lcrypto
TARGET   = puzzle71_solver
SRC      = src/puzzle71_solver.cpp

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGET) FOUNDKEY.TXT checkpoint.bin

run: $(TARGET)
	./$(TARGET)
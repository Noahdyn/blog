CXX = clang++
CXXFLAGS = -std=c++17 -Wall -Wextra
SRC = src/main.cpp src/compiler.cpp src/parser.cpp src/html.cpp
BIN = ssg

$(BIN): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(BIN)

run: $(BIN)
	./$(BIN)

clean:
	rm -f $(BIN)

.PHONY: run clean

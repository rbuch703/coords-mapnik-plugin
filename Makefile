#CXX = clang++

CXXFLAGS = -fPIC -std=c++11 -Wall -Wextra#$(shell mapnik-config --cflags) 

LIBS = -lmapnik#$(shell mapnik-config --libs --ldflags --dep-libs)

SRC = $(wildcard *.cc)

OBJ = $(SRC:.cc=.o)

BIN = hello.input

all : $(SRC) $(BIN)

$(BIN) : $(OBJ)
	@echo [LD] $@
	@$(CXX) -shared $(OBJ) $(LIBS) -o $@

%.o: %.cc 
	@echo [C++] $<
	@g++ $(CXXFLAGS) $< -c -o $@

#.cpp.o :
#	$(CXX) -c $(CXXFLAGS) $< -o $@

.PHONY : clean

clean:
	rm -f $(OBJ)
	rm -f $(BIN)

deploy : all
	@echo [CP] hello.input "->" $(shell mapnik-config --input-plugins)
	@cp hello.input $(shell mapnik-config --input-plugins)

install: clean all deploy

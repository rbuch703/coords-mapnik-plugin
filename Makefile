#CXX = clang++

CXXFLAGS = -g -fPIC -std=c++11 -Wall -Wextra#$(shell mapnik-config --cflags) 

LIBS = -lmapnik#$(shell mapnik-config --libs --ldflags --dep-libs)

SRC = helloDataSource.cc helloFeatureSet.cc mem_map.cc osmMappedTypes.cc osmTypes.cc
OBJ = $(SRC:.cc=.o)

BIN = hello.input tester

all : $(BIN)

hello.input : $(OBJ)
	@echo [LD] $@
	@$(CXX) -shared $(OBJ) $(LIBS) -o $@

tester : $(OBJ) pluginTest.cc
	@echo [LD] $@
	@$(CXX) $(OBJ) -g pluginTest.cc $(LIBS) -o $@

%.o: %.cc 
	@echo [C++] $<
	@g++ $(CXXFLAGS) $< -c -o $@

#.cpp.o :
#	$(CXX) -c $(CXXFLAGS) $< -o $@

.PHONY : clean

clean:
	rm -f $(OBJ) $(BIN) *~

deploy : hello.input
	@echo [CP] hello.input "->" $(shell mapnik-config --input-plugins)
	@cp hello.input $(shell mapnik-config --input-plugins)

install: clean all deploy


PLUGIN_SRC = coordsDataSource.cc coordsFeatureSet.cc mem_map.cc osmMappedTypes.cc osmTypes.cc
TESTER_SRC = coordsDataSource.cc coordsFeatureSet.cc mem_map.cc osmMappedTypes.cc osmTypes.cc pluginTest.cc

PLUGIN_OBJ  = $(patsubst %.cc,build/%.o,$(PLUGIN_SRC))
TESTER_OBJ  = $(patsubst %.cc,build/%.o,$(TESTER_SRC))

CXXFLAGS = -g -fPIC -std=c++11 -Wall -Wextra#$(shell mapnik-config --cflags) 

LD_FLAGS = -lmapnik -licui18n -licuuc #$(shell mapnik-config --libs --ldflags --dep-libs)#-fsanitize=address#-flto -O2 #-fprofile-arcs#--as-needed

.PHONY: all clean

all: build make.dep build/tester build/coords.input  
#build/simplifier build/data_converter
#	 @echo [ALL] $<

build:
	@echo [MKDIR ] $@
	@mkdir -p build

build/coords.input: $(PLUGIN_OBJ) make.dep
	@echo [LD ] $@
	@g++ -shared $(PLUGIN_OBJ) $(LD_FLAGS) -o $@

build/tester: $(TESTER_OBJ) make.dep
	@echo [LD ] $@
	@g++ $(TESTER_OBJ) $(LD_FLAGS) -o $@

build/%.o: %.cc 
	@echo [C++] $<
	@g++ $(CXXFLAGS) $< -c -o $@

clean:
	@echo [CLEAN]
	@rm -rf *~
	@rm -rf *gcda
	@rm -rf *gcno
	@rm -rf coverage.info callgrind.out.*
	@rm -rf build/*

deploy : build/coords.input
	@echo [CP] build/coords.input "->" $(shell mapnik-config --input-plugins)
	@cp build/coords.input $(shell mapnik-config --input-plugins)

make.dep: $(PLUGIN_SRC) $(TESTER_SRC)
	@echo [DEP]
	@g++ -MM -MG $^ | sed "s/\([[:graph:]]*\)\.o/build\/\\1.o/g" > make.dep

include make.dep


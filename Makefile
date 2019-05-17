PYTHON_EXECUTABLE ?= python
PYTHON_CONFIG ?= $(PYTHON_EXECUTABLE)-config
PREFIX ?= /usr/local
PYTHON_INCLUDES = `$(PYTHON_CONFIG) --includes`
PYTHON_LIBS = `$(PYTHON_CONFIG) --libs`
CXX = g++
AR = ar crvs
RANLIB = ranlib
CXXFLAGS = -O2 -fPIC -fno-stack-protector
GEOFUN_OBJ = geofun.o
GEOFUN_LIB = libgeofun.a
GEOFUN_INC = geofun.hpp

all: $(GEOFUN_OBJ) $(GEOFUN_LIB) build
	
$(GEOFUN_OBJ): geofun.cpp geofun.hpp
	@$(CXX) -c $(CXXFLAGS) -o $(GEOFUN_OBJ) geofun.cpp

$(GEOFUN_LIB): $(GEOFUN_OBJ)
	@$(AR) $(GEOFUN_LIB) $(GEOFUN_OBJ) > /dev/null
	@$(RANLIB) $(GEOFUN_LIB)

test: test_geofun.cpp geofun.cpp geofun.hpp
	@$(CXX) -o test_geofun -lcppunit -ldl test_geofun.cpp geofun.cpp
	@./test_geofun

module: $(GEOFUN_OBJ) geofun.i
	@swig -c++ -python geofun.i
	@g++ -c $(CXXFLAGS) ${PYTHON_INCLUDES} geofun_wrap.cxx 
	@g++ -shared geofun_wrap.o $(GEOFUN_OBJ) -o _geofun.so

build: $(GEOFUN_OBJ) geofun.i setup.py
	@swig -c++ -python geofun.i
	@$(PYTHON_EXECUTABLE) setup.py build

install: 
	@mkdir -p $(PREFIX)/include
	@mkdir -p $(PREFIX)/lib
	@cp -a $(GEOFUN_LIB) $(PREFIX)/lib
	@cp -a $(GEOFUN_INC) $(PREFIX)/include


.PHONY: clean all
clean:
	rm -f test_geofun
	rm -f *.o
	rm -f geofun_wrap.*
	rm -rf build
	rm -f *.pyc
	rm -f geofun.py
	rm -f _geofun.so
	rm -rf geofun.egg-info
	rm -f $(GEOFUN_LIB)


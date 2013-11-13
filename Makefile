PYTHON_INCLUDES = `python-config --includes`
PYTHON_LIBS = `python-config --libs`
CXX = g++
AR = ar crvs
RANLIB = ranlib
CXXFLAGS = -O2 -fPIC -fno-stack-protector
GEOFUN_OBJ = geofun.o
GEOFUN_LIB = libgeofun.a

all: $(GEOFUN_OBJ) $(GEOFUN_LIB) module
	
$(GEOFUN_OBJ): geofun.cpp geofun.hpp
	@$(CXX) -c $(CXXFLAGS) -o $(GEOFUN_OBJ) geofun.cpp

$(GEOFUN_LIB): $(GEOFUN_OBJ)
	@$(AR) $(GEOFUN_LIB) $(GEOFUN_OBJ) > /dev/null
	@$(RANLIB) $(GEOFUN_LIB)

test: test_geofun.cpp geofun.cpp geofun.hpp
	@$(CXX) -o test_geofun -lcppunit test_geofun.cpp geofun.cpp
	@./test_geofun

module: $(GEOFUN_OBJ) geofun.i
	@swig -c++ -python geofun.i
	@g++ -c $(CXXFLAGS) ${PYTHON_INCLUDES} geofun_wrap.cxx 
	@g++ -shared geofun_wrap.o $(GEOFUN_OBJ) -o _geofun.so

install: $(GEOFUN_OBJ) geofun.i setup.py
	@swig -c++ -python geofun.i
	@python2.7 setup.py build
	@sudo python2.7 setup.py install


.PHONY: clean all
clean:
	rm -f test_geofun
	rm -f *.o
	rm -f geofun_wrap.*
	rm -rf build
	rm -f *.pyc
	rm -f geofun.py
	rm -f _geofun.so
	rm -f $(GEOFUN_LIB)


PYTHON_INCLUDES = `python-config --includes`
PYTHON_LIBS = `python-config --libs`

all: geofun.cpp geofun.hpp
	@g++ -c -O2 -fPIC -fno-stack-protector -o geofun.o geofun.cpp

test: test_geofun.cpp geofun.cpp geofun.hpp
	@g++ -o test_geofun -lcppunit test_geofun.cpp geofun.cpp
	@./test_geofun

module: all geofun.i
	@swig -c++ -python geofun.i
	@g++ -c -O2 -fPIC -fno-stack-protector ${PYTHON_INCLUDES} geofun_wrap.cxx 
	@g++ -shared geofun_wrap.o geofun.o -o _geofun.so

install: all geofun.i setup.py
	@swig -c++ -python geofun.i
	@python setup.py build
	@sudo python setup.py install


.PHONY: clean
clean:
	rm -f test_geofun
	rm -f *.o
	rm -f geofun_wrap.*
	rm -rf build
	rm -f *.pyc
	rm -f geofun.py
	rm -f _geofun.so


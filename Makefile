PYTHON_INCLUDES = `python-config --includes`
PYTHON_LIBS = `python-config --libs`

all: geofun.cpp geofun.h
	@g++ -c -fPIC -fno-stack-protector -o geofun.o geofun.cpp

test: test_geofun.cpp geofun.cpp geofun.h
	@g++ -o test_geofun -lcppunit test_geofun.cpp geofun.cpp
	@./test_geofun

python: all geofun.i
	@swig -c++ -python geofun.i
	@g++ -c -fPIC -fno-stack-protector ${PYTHON_INCLUDES} geofun_wrap.cxx 
	@g++ -shared geofun_wrap.o geofun.o -o _geofun.so

.PHONY: clean
clean:
	rm -f test_geofun
	rm -f *.o


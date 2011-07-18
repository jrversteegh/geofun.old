all: geofun.cpp geofun.h
	@g++ -c -o geofun.o geofun.cpp

test: test_geofun.cpp geofun.cpp geofun.h
	@g++ -o test_geofun -lcppunit test_geofun.cpp geofun.cpp
	@./test_geofun

.PHONY: clean
clean:
	rm -f test_geofun
	rm -f *.o


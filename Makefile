all: getfilesinfo

proj4: getfilesinfo.o
	g++ -pthread getfilesinfo.cpp  -o getfilesinfo

clean:
	\rm *.o getfilesinfo


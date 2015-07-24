CC            = gcc
CXX           = g++
CFLAGS        = -pipe -O3 -Wall -W -fPIC
CXXFLAGS      = -pipe -std=c++0x -g -fno-omit-frame-pointer -DGIT_DESC=\"`git describe --tags --always`\" -O3 -Wall -W -fPIC $(DEFINES)
INCPATH       = -I/usr/include/python2.7 -I../../
LINK          = g++
#LFLAGS        = -rdynamic -Wl,-O1 -shared -Wl,-soname,pyiploc.so
LIBS          = $(SUBLIBS) -lpython2.7  -L./bin 
AR            = ar cqs
DEL_FILE      = rm -f
OBJECTS_DIR   = .objs/
CHK_DIR_EXISTS = test -d

SOURCES       = pyt_iploc.cpp \
		iploc_database.cpp \
		pyt_extension.cpp  \
		main.cpp

OBJECTS       = .objs/pyt_iploc.o \
		.objs/iploc_database.o \
		.objs/pyt_extension.o \
		.objs/main.o

QMAKE_TARGET  = iploc.exe
DESTDIR       = ./bin/
TARGET        = iploc.exe

first: all

.SUFFIXES: .o .c .cpp .cc .cxx .C

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.cxx.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.C.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.c.o:
	$(CC) -c $(CFLAGS) $(INCPATH) -o "$@" "$<"

####### Build rules

all: ./bin/$(TARGET)

./bin/$(TARGET):  $(OBJECTS) $(SUBLIBS) $(OBJCOMP)  
	@$(CHK_DIR_EXISTS) ./bin/ || $(MKDIR) ./bin/ 
	-$(DEL_FILE) ./bin/$(TARGET)
	$(LINK) $(LFLAGS) -o ./bin/$(TARGET) $(OBJECTS) $(LIBS) $(OBJCOMP)

clean:
	-$(DEL_FILE) $(OBJECTS)
	-$(DEL_FILE) $(TARGET) $(QMAKE_TARGET)
	-$(DEL_FILE) *~ core *.core

.objs/pyt_iploc.o: pyt_iploc.cpp pyt_iploc.h iploc_database.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o .objs/pyt_iploc.o pyt_iploc.cpp

.objs/iploc_database.o: iploc_database.cpp iploc_database.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o .objs/iploc_database.o iploc_database.cpp

.objs/pyt_extension.o: pyt_extension.cpp pyt_iploc.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o .objs/pyt_extension.o pyt_extension.cpp

.objs/main.o: main.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o .objs/main.o main.cpp


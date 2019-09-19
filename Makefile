include /usr/local/quanta04/include/QUANTA/QUANTA_APPLICATION_INCLUDES

CC = g++

CFLAGS = -O  -I/usr/local/include -I/usr/local/quanta04/include/QUANTA -DDEBUG -Wno-deprecated
C++FLAGS = $(CFLAGS)

LFLAGS = -O 

VERSION =P2P03

LIBS = -L/usr/local/lib -L/usr/local/quanta04/lib -lm  -lportaudio -lpthread
OBJS =vocAL.o\
	sample$(VERSION).o\
	 pablio.o\
	 ringbuffer.o\
	HybridP2Ptcp.o

all:
	rm sample$(VERSION); make $(OBJS) sample$(VERSION) 

OS=Linux

.SUFFIXES: .cxx
.cxx.o:
	$(CC) $(C++FLAGS) -D$(OS) -g  -DDEBUG $(QUANTA_CFLAGS) -c $<

OBJ =  vocAL.o pablio.o ringbuffer.o HybridP2Ptcp.o

sample$(VERSION):sample$(VERSION).o
	$(CC) $(LFLAGS) -o sample$(VERSION) sample$(VERSION).o $(OBJ) $(QUANTA_LIB) $(LIBS)

clean:
	rm $(OBJS) core* sample$(VERSION)

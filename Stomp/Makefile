
PROGRAMS := Sender Receiver Receiver2 Socktest

CXX = g++
CXXFLAGS = -g -pthread -m64 -std=c++14 -Wall
INCLUDES =
STOMPOBJS = Stomp.o StompAgent.o

.cpp.o:
	$(CXX) -o $@ $(CXXFLAGS) $(INCLUDES) -c $<

all: $(PROGRAMS)


Sender: Sender.o StrFile.o $(STOMPOBJS)
	$(CXX) -o $@ $^

Receiver: Receiver.o $(STOMPOBJS)
	$(CXX) -o $@ $^

Receiver2: Receiver2.o $(STOMPOBJS)
	$(CXX) -o $@ $^

Socktest: Socktest.o $(STOMPOBJS)
	$(CXX) -o $@ $^

clean:
	rm -f $(PROGRAMS) *.o


.PHONY: all clean


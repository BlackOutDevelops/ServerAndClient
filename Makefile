# Makefile for C/C++ code

# on solaris we need to explicitly include the socket and name server
# libraries, this because sockets are not supported by the O.S. directly,
# but are a library that calls the native (XTI) network API.
# Leave this commented out unless you are on a Sun running Solaris
#LIBS= -lsocket -lnsl

# by default Make will build the first target - so here we define the
# first target "all", which depends on target "server"
all:
	gcc -g -o server server.c ${LIBS}
	gcc -g -o client client.c ${LIBS}

clean:
	rm *~ *.o server client

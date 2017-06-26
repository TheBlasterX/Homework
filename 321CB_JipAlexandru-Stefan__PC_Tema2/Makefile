CC=gcc
LIBSOCKET=-lnsl
CCFLAGS=-Wall -g
SRV=server
CLT=client

all: $(SRV) $(CLT)

$(SRV):$(SRV).c
	$(CC) $(CCFLAGS) -o $(SRV) $(LIBSOCKET) $(SRV).c

$(CLT):	$(CLT).c
	$(CC) $(CCFLAGS) -o $(CLT) $(LIBSOCKET) $(CLT).c

clean:
	rm -f $(SRV) $(CLT)


all: COHERENCE

CC = gcc
LD = gcc

#CFLAGS = -g -Wall -Iinclude
CFLAGS += -std=gnu9x
LDFLAGS = 

HDRS = ./*.h

SRCS = simulator.c

OBJS = $(patsubst %.c,%.o,$(SRCS))

$(OBJS) : %.o : %.c ./*.h
	$(CC) -c $(CFLAGS) $< -o $@

COHERENCE: simulator.o
	$(LD) $(LDFLAGS) simulator.o -o coherence 
	
clean:
	rm -f *.o client local_dns dns

tags: $(SRCS) $(HDRS)
	ctags $(SRCS) $(HDRS)

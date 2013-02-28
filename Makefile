CC	:= gcc
HEAD	:= vgps_device.h vgps.h
SRC	:= main.c vgps_device.c vgps.c
OBJS	:= main.o vgps_device.o vgps.o
TT	:= vgps

INC = .
CFLAGS = -pipe  -Wall -I$(INC)
LDFLAGS = -Wall 

all:$(TT)

$(TT):$(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@ 

main.o: main.c
	$(CC) $(CFLAGS) -c $< -o $@
vgps_device.o: vgps_device.c
	$(CC) $(CFLAGS) -c $< -o $@
vgps.o: vgps.c
	$(CC) $(CFLAGS) -c $< -o $@	

.PHONY	: clean

clean:
	rm -f *.o *~
	rm -f $(TT)


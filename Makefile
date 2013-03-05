CC	:= gcc
HEAD	:= vgps_device.h vgps.h utility.h driver_nmea0813.h
SRC		:= main.c vgps_device.c vgps.c utility.c driver_nmea0813.c
OBJS	:= main.o vgps_device.o vgps.o utility.o driver_nmea0813.o
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
utility.o: utility.c	
	$(CC) $(CFLAGS) -c $< -o $@	
driver_nmea0813.o: driver_nmea0813.c	
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY	: clean

clean:
	rm -f *.o *~
	rm -f $(TT)


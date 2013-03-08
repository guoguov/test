CC	:= gcc
HEAD	:= vgps_device.h utility.h driver_nmea0813.h driver_ublox.h gps.h loc_eng.h
SRC		:= main.c vgps_device.c  utility.c driver_nmea0813.c driver_ublox.c loc_eng.c
OBJS	:= main.o vgps_device.o  utility.o driver_nmea0813.o driver_ublox.o loc_eng.o
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
utility.o: utility.c	
	$(CC) $(CFLAGS) -c $< -o $@	
driver_nmea0813.o: driver_nmea0813.c	
	$(CC) $(CFLAGS) -c $< -o $@
dricer_ublox.o: driver_ublox.c 	
	$(CC) $(CFLAGS) -c $< -o $@
loc_eng.o: loc_eng.c	
	$(CC) $(CFLAGS) -c $< -o $@
	
.PHONY	: clean

clean:
	rm -f *.o *~
	rm -f $(TT)


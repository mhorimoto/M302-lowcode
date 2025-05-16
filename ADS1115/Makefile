ifndef TTY
TTY=ttyACM0
endif
ifndef BOARD
BOARD=nano:cpu=atmega328old
endif
all:
	@echo nothing

main: singleended/singleended.touch

singleended/singleended.touch: singleended/singleended.ino
	arduino-cli compile -b arduino:avr:${BOARD} singleended
	touch singleended/singleended.touch

inst: singleended/singleended.touch
	arduino-cli upload -p /dev/${TTY} -b arduino:avr:${BOARD} singleended

clean:
	-rm singleended/singleended.touch

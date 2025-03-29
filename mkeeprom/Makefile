ifndef TTY
TTY=ttyACM0
endif
ifndef BOARD
BOARD=nano:cpu=atmega328old
endif
ifndef W5500SS
W5500SS=10
endif
all:
	@echo nothing

mklc: mklc/mklc.touch

mklc/mklc.touch: mklc/mklc.ino mklc/listCommand.ino mklc/setCommand.ino mklc/clearpage.ino mklc/help.ino
	arduino-cli compile -b arduino:avr:${BOARD} mklc
	touch mklc/mklc.touch

inst: mklc/mklc.touch 
	arduino-cli upload -p /dev/${TTY} -b arduino:avr:${BOARD} mklc

clean: 
	rm mklc/mklc.touch 


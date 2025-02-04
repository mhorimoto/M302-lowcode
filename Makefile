ifndef TTY
TTY=ttyUSB0
endif
ifndef BOARD
BOARD=uno
endif
ifndef W5500SS
W5500SS=10
endif
all:
	@echo nothing

main: main/main.touch

main/main.touch: main/main.ino main/lcdout.ino
	arduino-cli compile -b arduino:avr:${BOARD} --build-property build.extra_flags=-DW5500SS=${W5500SS} main
	touch main/main.touch

inst: main/main.touch
	arduino-cli upload -p /dev/${TTY} -b arduino:avr:${BOARD} main

clean:
	-rm main/main.touch

eeprom: eeprom/eeprom.touch

eeprom/eeprom.touch: eeprom/eeprom.ino eeprom/bytemap.ino eeprom/cmnd_dump.ino eeprom/cmnd_fill.ino eeprom/cmnd_setbyte.ino
	arduino-cli compile -b arduino:avr:${BOARD} --build-property build.extra_flags=-DW5500SS=${W5500SS} eeprom
	touch eeprom/eeprom.touch

eeprominst: eeprom/eeprom.touch
	arduino-cli upload -p /dev/${TTY} -b arduino:avr:${BOARD} eeprom

eepromclean:
	-rm eeprom/eeprom.touch

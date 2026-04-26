atmega328pbの場合には、

```
arduino-cli compile -b MiniCore:avr:328:variant=modelPB,clock=8MHz_internal test4 --export-binaries
avrdude -c usbasp -p m328pb -U flash:w:test4/build/MiniCore.avr.328/test4.ino.hex:i
```

USBaspを用いて書き込む。

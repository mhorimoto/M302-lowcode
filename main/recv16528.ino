#define  LEN_UECS_BUFFER 8
/////////////////////////////////
// Reset Function goto Address 0
/////////////////////////////////
void(*resetFunc)(void) = 0;


void recv16528port(void) {
  char *xmlDT PROGMEM = CCMFMT;
  char uecsbuf[LEN_UECS_BUFFER];
  int packetSize,i,iaddr,idata;
  char eaddr[4],edata[3];
  const char *EEPROMIMG PROGMEM = "%03X=%02X";
  extern void clrM252(int);
  extern char val[];
  
  wdt_reset();
  packetSize = Udp16528.parsePacket();
  if (packetSize>0) {
    Serial.println(packetSize);
    Udp16528.read(uecsbuf,LEN_UECS_BUFFER-1);
    uecsbuf[packetSize] = NULL;
    for(i=0;i<LEN_UECS_BUFFER;i++) {
      if (uecsbuf[i]<(char)0x20) {
	val[i] = (char)NULL;
      } else {
	val[i] = uecsbuf[i];
      }
    }
    if (!strcmp(val,"R77")) {
      resetFunc();
    }
    if (!strcmp(val,"R71")) {
      clrM252(1);
    }
    if (!strcmp(val,"R72")) {
      digitalWrite(9,LOW);
      delay(50);
      digitalWrite(9,HIGH);
    }
    if (val[0]=='S') {
      for(i=0;i<3;i++) {
	eaddr[i]=val[i+1];
      }
      eaddr[i] = (char)NULL;
      for(i=0;i<2;i++) {
	edata[i]=val[i+4];
      }
      edata[i]=(char)NULL;
      iaddr = strtol(eaddr, NULL, 16);
      idata = strtol(edata, NULL, 16);
      EEPROM.write(iaddr,idata);
      sprintf(val,EEPROMIMG,iaddr,idata);
      uecsSendData(0,xmlDT,val,0);
    }
    if (val[0]=='D') {
      for(i=0;i<3;i++) {
	eaddr[i]=val[i+1];
      }
      eaddr[i] = (char)NULL;
      iaddr = strtol(eaddr, NULL, 16);
      idata = EEPROM.read(iaddr);
      sprintf(val,EEPROMIMG,iaddr,idata);
      uecsSendData(0,xmlDT,val,0);
    }
  }
}


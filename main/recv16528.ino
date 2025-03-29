#define  LEN_UECS_BUFFER 8

void recv16528port(void) {
  char uecsbuf[LEN_UECS_BUFFER];
  int packetSize ;
  wdt_reset();
  packetSize = Udp16528.parsePacket();
  if (packetSize>0) {
    Serial.println(packetSize);
    Udp16528.read(uecsbuf,LEN_UECS_BUFFER-1);
    uecsbuf[packetSize] = NULL;
    Serial.println(uecsbuf);
  }
}


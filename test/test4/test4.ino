uint16_t  FSTLB[15];

void setup() {
    int i;
    unsigned int v, l, f;
    Serial.begin(57600);
    for (i=0; i<15; i++) {
        v = i & 0x0001;
        l = i;
        f = i * 2;
        FSTLB[i] = 0;
        FSTLB[i] = (f<<8) | (l<<4) | v;
    }
    Serial.println();
    Serial.println("START v0.05");
}

void loop() {
    int i;
    int v,l,f;
    for (i=0; i<15; i++) {
        v = FSTLB[i] & 0x0001;
        l = (FSTLB[i] & 0x00f0) >> 4;
        f = (FSTLB[i] & 0xff00) >> 8;
        Serial.print("FSTLB[");
        Serial.print(i);
        Serial.print("] = 0x");
        Serial.print(FSTLB[i], HEX);
        Serial.print("  v=");
        Serial.print(v);
        Serial.print("  l=");
        Serial.print(l);
        Serial.print("  f=");
        Serial.println(f);
    }
    while(1) {
        __asm__ __volatile__ ("nop");
    }
  // Your main code here
}
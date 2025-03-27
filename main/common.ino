void copyVersionCode(unsigned int addr,char *pgn) {
    int i;
    uint8_t c;
    
    for(i=0;i<16;i++) {
        c = (uint8_t)(*pgn);
        eeprom_update(addr+i,c);
        if (c==0) break;
        pgn++;
    }
}
  
  void eeprom_update(unsigned int a,uint8_t v) {
    uint8_t rv;
    rv = EEPROM.read(a);
    if (rv!=v) {
      EEPROM.write(a,v);
    }
  }
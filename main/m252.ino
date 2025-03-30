void getM252(int ccmid,bool f) {
  char *xmlDT PROGMEM = CCMFMT;
  extern char val[],lcdtext[2][17];
  
  if (f==false) {
    digitalWrite(LED2,HIGH);
    if (digitalRead(4)==HIGH) {
      Serial.write('a');
      delay(100);
      if (Serial.available() > 0) { // 受信データを読み込む
        String receivedData = Serial.readStringUntil(0x0a); // 改行コードが来るまで読み込む
        receivedData.trim(); // 前後の空白を削除
        receivedData.replace("\r", ""); // 改行コードを削除
        receivedData.replace("\n", ""); // 改行コードを削除
        receivedData.toCharArray(val,16);
      }
    } else {
      val[0] = 'N';
      val[1] = 'a';
      val[2] = 'N';
      val[3] = 0;
    }
    digitalWrite(LED2,LOW);
  } else {
    uecsSendData(ccmid,xmlDT,val,0);
  }
  sprintf(lcdtext[1],"%d/%d/%d  %sL",
          EEPROM.read(LC_SEND_START+LC_SEND_ROOM),EEPROM.read(LC_SEND_START+LC_SEND_REGION),
	  EEPROM.read(LC_SEND_START+LC_SEND_ORDER),val);
  lcd.setCursor(0,1);
  lcd.print(lcdtext[1]);
}

void clrM252(int ccmid) {
 char *xmlDT PROGMEM = CCMFMT;
  extern char val[],lcdtext[2][17];
  
  digitalWrite(LED2,HIGH);
  if (digitalRead(4)==HIGH) {
    Serial.write('b');
    delay(20);
    if (Serial.available() > 0) { // 受信データを読み込む
      String receivedData = Serial.readStringUntil(0x0a); // 改行コードが来るまで読み込む
      receivedData.trim(); // 前後の空白を削除
      receivedData.replace("\r", ""); // 改行コードを削除
      receivedData.replace("\n", ""); // 改行コードを削除
      receivedData.toCharArray(val,16);
    }
  } else {
    val[0] = 'C';
    val[1] = 'L';
    val[2] = 'R';
    val[3] = 0;
  }
  digitalWrite(LED2,LOW);
  uecsSendData(ccmid,xmlDT,val,0);
  sprintf(lcdtext[1]," CLEAR          ",val);
  lcd.setCursor(0,1);
  lcd.print(lcdtext[1]);
}

#include <Arduino.h>
#include <SoftwareSerial.h> // SoftwareSerialライブラリをインクルード

// SoftwareSerialのピン設定
// RXピン (Arduinoがデータを受信するピン) をデジタルピン2に
// TXピン (Arduinoがデータを送信するピン) をデジタルピン3に
SoftwareSerial MySerial(A5, A4); // RX, TX

// Data
SLT5006DATA  sltdata;

void dataConvDebugMsg(byte dth,byte dtl,int idt) {
  Serial.print(" <<");
  Serial.print(dth);
  Serial.print(",");
  Serial.print(dtl);
  Serial.print(",");
  Serial.print(idt);
  Serial.println(">> ");
}

float _dataConvEle(char *rdt,int h,int l,float f) {
  byte dth,dtl;
  int idt;
  dth = (*(rdt+h))&0xff;
  dtl = (*(rdt+l))&0xff;
  idt = dth*0x100+dtl;
  //  dataConvDebugMsg(dth,dtl,idt);
  return( (float)(idt * f) );
}

void dataConv(char *rdt) {
  float _dataConvEle(char *,int,int,float);
  byte dth,dtl;
  int idt;
  sltdata.temp = _dataConvEle(rdt,4,3,0.0625);
  sltdata.ec_bulk = _dataConvEle(rdt,6,5,0.001);
  sltdata.vwc_rock = _dataConvEle(rdt,8,7,0.1);
  sltdata.vwc = _dataConvEle(rdt,10,9,0.1);
  sltdata.vwc_coco = _dataConvEle(rdt,12,11,0.1);
  sltdata.ec_pore = _dataConvEle(rdt,16,15,0.001);
}

int rx_data(int dataConv_Flag,int CompleteCheck) {
  extern unsigned long cndVal;
  extern void byteArrayToHexString(const byte*,int);
// SoftwareSerialからの応答を受信
  unsigned long startTime = millis();
  const long timeout = 500; // タイムアウト1秒
  byte receiveData[64];
  int receivedBytes = 0;
  int ret;
  
  //delay(100);
  // SoftwareSerialからデータが来るのを待つ
  while (millis() - startTime < timeout && receivedBytes < sizeof(receiveData)) {
    if (MySerial.available()) {
      receiveData[receivedBytes++] = MySerial.read();
    }
  }
  if (CompleteCheck==1) {
    if (receiveData[3]==1) {  // Measuring completed の確認を行う。
      ret = 0;
    } else {
      ret = 2;
    }
  } else {
    ret = 0;
  }
  if (receivedBytes > 0) {
    Serial.print(F("Receive from SoftwareSerial: "));
    byteArrayToHexString(receiveData, receivedBytes);
    if (dataConv_Flag!=0) {
      dataConv(receiveData);
    }
    //Serial.println(sltdata.temp);
  } else {
    Serial.println(F("No data from SoftwareSerial"));
    cndVal = 0x20000900;
  }
  return(ret);
}

//
//  初期化ルーティン
//
void slt5006_setup() {
  // 標準シリアルポート（USB経由でPCと通信）
  // SoftwareSerialポート（別のデバイスと通信）

  int r;
  byte check_ver[]    = {0x01,0x00,0x07,0xc2,0x61};
  MySerial.begin(9600); // 接続するデバイスのボーレートに合わせて設定
  delay(100);
  MySerial.write(check_ver,5);
  r = rx_data(0,0);
}

//
//  10秒毎に呼ばれる
//
void slt5006_loop() {
  byte start_mesure[] = {0x02,0x07,0x01,0x01,0x0D,0x70};
  byte check_mesure[] = {0x01,0x08,0x01,0x00,0xe6};
  byte read_result[]  = {0x01,0x13,0x10,0xfc,0x2c};
  char *xmlDT PROGMEM = CCMFMT;
  extern void uecsSendData(int,char *,char *,int);
  extern SLT5006DATA sltdata;
  int r;
  
  MySerial.write(start_mesure,6);
  r = rx_data(0,0);
  //  delay(20);
  do {
    MySerial.write(check_mesure,5);
    r = rx_data(0,1);
    wdt_reset();
    //    delay(20);
  } while(r==2);
  MySerial.write(read_result,5);
  r = rx_data(1,0);
  dtostrf(sltdata.temp,-6,3,val);
  uecsSendData(1,xmlDT,val,0);
  dtostrf(sltdata.ec_bulk,-6,3,val);
  uecsSendData(2,xmlDT,val,0);
  dtostrf(sltdata.vwc_rock,-5,1,val);
  uecsSendData(3,xmlDT,val,0);
  dtostrf(sltdata.vwc,-5,1,val);
  uecsSendData(4,xmlDT,val,0);
  dtostrf(sltdata.vwc_coco,-5,1,val);
  uecsSendData(5,xmlDT,val,0);
  dtostrf(sltdata.ec_pore,-6,3,val);
  uecsSendData(6,xmlDT,val,0);
  wdt_reset();
}

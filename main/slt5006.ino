#include <Arduino.h>
#include <SoftwareSerial.h> // SoftwareSerialライブラリをインクルード

// SoftwareSerialのピン設定
// RXピン (Arduinoがデータを受信するピン) をデジタルピン2に
// TXピン (Arduinoがデータを送信するピン) をデジタルピン3に
SoftwareSerial MySerial(A5, A4); // RX, TX

// Data
SLT5006DATA  sltdata;

void dataConvDebugMsg(char *s,byte dth,byte dtl,int idt) {
  Serial.print(" <<");
  Serial.print(s);
  Serial.print(":");
  Serial.print(dth);
  Serial.print(",");
  Serial.print(dtl);
  Serial.print(",");
  Serial.print(idt);
  Serial.println(">> ");
}

void dataConv(char *rdt) {
  //extern SLT5006DATA sltdata;
  byte dth,dtl;
  int idt;
  dth = (*(rdt+4))&0xff;
  dtl = (*(rdt+3))&0xff;
  idt = dth*0x100+dtl;
  dataConvDebugMsg("temp",dth,dtl,idt);
  sltdata.temp = (float)(idt * 0.0625);
  dth = (*(rdt+6))&0xff;
  dtl = (*(rdt+5))&0xff;
  idt = dth*0x100+dtl;
  dataConvDebugMsg("ec_bulk",dth,dtl,idt);
  sltdata.ec_bulk = (float)(idt * 0.001);
  dth = (*(rdt+8))&0xff;
  dtl = (*(rdt+7))&0xff;
  idt = dth*0x100+dtl;
  dataConvDebugMsg("vwc_rock",dth,dtl,idt);
  sltdata.vwc_rock = (float)(idt * 0.1);
  dth = (*(rdt+10))&0xff;
  dtl = (*(rdt+9))&0xff;
  idt = dth*0x100+dtl;
  dataConvDebugMsg("vwc",dth,dtl,idt);
  sltdata.vwc = (float)(idt * 0.1);
  dth = (*(rdt+12))&0xff;
  dtl = (*(rdt+11))&0xff;
  idt = dth*0x100+dtl;
  dataConvDebugMsg("vwc_cocobulk",dth,dtl,idt);
  sltdata.vwc_coco = (float)(idt * 0.1);
  dth = (*(rdt+16))&0xff;
  dtl = (*(rdt+15))&0xff;
  idt = dth*0x100+dtl;
  dataConvDebugMsg("ec_pore",dth,dtl,idt);
  sltdata.ec_pore = (float)(idt * 0.001);
}

/**
 * @brief 16進数の文字を数値に変換するヘルパー関数
 * @param hexChar 16進数の文字 (例: '0' - '9', 'A' - 'F', 'a' - 'f')
 * @return 変換された数値 (0 - 15)、またはエラーの場合は-1
 */
int hexCharToByte(char hexChar) {
  if (hexChar >= '0' && hexChar <= '9') {
    return hexChar - '0';
  } else if (hexChar >= 'A' && hexChar <= 'F') {
    return hexChar - 'A' + 10;
  } else if (hexChar >= 'a' && hexChar <= 'f') {
    return hexChar - 'a' + 10;
  }
  return -1; // 無効な文字
}

/**
 * @brief 16進数の文字列をバイト配列に変換する関数
 * @param hexString 16進数の文字列 (例: "01 2A FF")
 * @param outputByteArray 変換結果を格納するバイト配列
 * @param outputByteArraySize outputByteArrayの最大サイズ
 * @return 変換されたバイト数、またはエラーの場合は-1
 */
int hexStringToByteArray(const char* hexString, byte* outputByteArray, int outputByteArraySize) {
  int len = strlen(hexString);
  int byteIndex = 0;

  for (int i = 0; i < len; ++i) {
    // スペースはスキップ
    if (hexString[i] == ' ') {
      continue;
    }

    // 偶数長のチェックとバイト配列のオーバーフローチェック
    if (i + 1 >= len || byteIndex >= outputByteArraySize) {
      Serial.println(F("error: The hex string must be an even length or the buffer is insufficient."));
      return -1;
    }

    int highNibble = hexCharToByte(hexString[i]);
    int lowNibble = hexCharToByte(hexString[i+1]);

    if (highNibble == -1 || lowNibble == -1) {
      Serial.println(F("error: It contains invalid hexadecimal characters."));
      return -1;
    }

    outputByteArray[byteIndex] = (byte)((highNibble << 4) | lowNibble);
    byteIndex++;
    i++; // 2文字処理したのでインクリメント
  }
  return byteIndex;
}

/**
 * @brief バイト配列を16進数の文字列に変換してシリアル出力する関数 (標準Serial用)
 * @param byteArray 変換するバイト配列
 * @param length バイト配列の長さ
 */
void byteArrayToHexString(const byte* byteArray, int length) {
  for (int i = 0; i < length; ++i) {
    if (byteArray[i] < 0x10) { // 1桁の場合は先頭に0を追加
      Serial.print("0");
    }
    Serial.print(byteArray[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}

/**
 * @brief バイト配列を16進数の文字列に変換してシリアル出力する関数 (SoftwareSerial用)
 * @param byteArray 変換するバイト配列
 * @param length バイト配列の長さ
 */
void byteArrayToHexStringSoft(const byte* byteArray, int length) {
  for (int i = 0; i < length; ++i) {
    if (byteArray[i] < 0x10) { // 1桁の場合は先頭に0を追加
      MySerial.print("0");
    }
    MySerial.print(byteArray[i], HEX);
    MySerial.print(" ");
  }
  MySerial.println();
}

int rx_data(int dataConv_Flag,int CompleteCheck) {
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
     if (receiveData[3]==1) {
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
  }
  return(ret);
}

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

void slt5006_loop() {
  byte start_mesure[] = {0x02,0x07,0x01,0x01,0x0D,0x70};
  byte check_mesure[] = {0x01,0x08,0x01,0x00,0xe6};
  byte read_result[]  = {0x01,0x13,0x10,0xfc,0x2c};
  extern SLT5006DATA sltdata;
  int r;
  
  MySerial.write(start_mesure,6);
  //  byteArrayToHexString(start_mesure,6);
  r = rx_data(0,0);
  delay(20);
  do {
    MySerial.write(check_mesure,5);
    //  byteArrayToHexString(check_mesure,5);
    r = rx_data(0,1);
    delay(20);
  } while(r==2);
  MySerial.write(read_result,5);
  //  byteArrayToHexString(read_result,5);
  r = rx_data(1,0);
}

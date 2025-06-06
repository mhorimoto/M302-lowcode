#include <Arduino.h>
#include <SoftwareSerial.h> // SoftwareSerialライブラリをインクルード

const char *ver = "2.14";

// SoftwareSerialのピン設定
// RXピン (Arduinoがデータを受信するピン) をデジタルピン2に
// TXピン (Arduinoがデータを送信するピン) をデジタルピン3に
SoftwareSerial MySerial(A5, A4); // RX, TX

// シリアルポートからの入力バッファサイズ
const int MAX_INPUT_LENGTH = 64; 
char serialInputBuffer[MAX_INPUT_LENGTH];
int inputBufferIndex = 0;

// Data
struct SLT5006DATA {
  float temp;
  float ec_bulk;
  float vwc_rock;
  float vwc;
  float vwc_coco;
  float ec_pore;
} sltdata;

byte start_mesure[] = {0x02,0x07,0x01,0x01,0x0D,0x70};
byte check_mesure[] = {0x01,0x08,0x01,0x00,0xe6};
byte read_result[]  = {0x01,0x13,0x10,0xfc,0x2c};
byte check_ver[]    = {0x01,0x00,0x07,0xc2,0x61};

void dataConv(char *rdt) {
  extern SLT5006DATA sltdata;
  byte dth,dtl;
  int idt;
  dth = (*(rdt+4))&0xff;
  dtl = (*(rdt+3))&0xff;
  idt = dth*0x100+dtl;
  Serial.print(" <<");
  Serial.print(dth);
  Serial.print(",");
  Serial.print(dtl);
  Serial.print(",");
  Serial.print(idt);
  Serial.print(">> ");
  sltdata.temp = (float)(idt * 0.0625);
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

void rx_data(void) {
// SoftwareSerialからの応答を受信
  long startTime = millis();
  const long timeout = 1000; // タイムアウト1秒
  byte receiveData[64];
  int receivedBytes = 0;

  delay(100);
  // SoftwareSerialからデータが来るのを待つ
  while (millis() - startTime < timeout && receivedBytes < sizeof(receiveData)) {
    if (MySerial.available()) {
      receiveData[receivedBytes++] = MySerial.read();
    }
  }

  if (receivedBytes > 0) {
    Serial.print(F("Receive from SoftwareSerial: "));
    byteArrayToHexString(receiveData, receivedBytes);
    Serial.print(F("DATA="));
    dataConv(receiveData);
    Serial.println(sltdata.temp);
  } else {
    Serial.println(F("No data from SoftwareSerial"));
  }
}

void setup() {
  // 標準シリアルポート（USB経由でPCと通信）
  Serial.begin(19200); 
  while (!Serial) {
    ; // シリアルポートが接続されるまで待機 (Leonardo, Micro など用)
  }
  Serial.print(F("SLT5006 TEST v"));
  Serial.println(ver);
  Serial.println(F("Enter the hexadecimal number you want to send into the serial monitor."));

  // SoftwareSerialポート（別のデバイスと通信）
  MySerial.begin(9600); // 接続するデバイスのボーレートに合わせて設定
  MySerial.println(F("Initialize SoftwareSerial"));
  delay(500);
  MySerial.write(check_ver,5);
  rx_data();
}

void loop() {
  extern SLT5006DATA sltdata;
  delay(500);
  MySerial.write(start_mesure,6);
  rx_data();
  delay(100);
  MySerial.write(check_mesure,5);
  rx_data();
  delay(400);
  MySerial.write(read_result,5);
  rx_data();
  delay(1000);
  while (Serial.available()) {
    char inChar = Serial.read();
    
    // 改行文字が来たら入力完了
    if (inChar == '\n' || inChar == '\r') {
      serialInputBuffer[inputBufferIndex] = '\0'; // ヌル終端
      
      // 入力された文字列を処理
      if (inputBufferIndex > 0) {
        byte sendData[MAX_INPUT_LENGTH / 2]; // 最大バイト数
        int bytesToSend = hexStringToByteArray(serialInputBuffer, sendData, sizeof(sendData));

        if (bytesToSend > 0) {
          Serial.print(F("Send to stdout: "));
          byteArrayToHexString(sendData, bytesToSend);
          
          // SoftwareSerialへ送信
          MySerial.write(sendData, bytesToSend);
          Serial.println(F("Send to SoftwareSerial"));

          // SoftwareSerialからの応答を受信
          long startTime = millis();
          const long timeout = 1000; // タイムアウト1秒
          byte receiveData[64];
          int receivedBytes = 0;

          // SoftwareSerialからデータが来るのを待つ
          while (millis() - startTime < timeout && receivedBytes < sizeof(receiveData)) {
            if (MySerial.available()) {
              receiveData[receivedBytes++] = MySerial.read();
            }
          }

          if (receivedBytes > 0) {
            Serial.print(F("Receive from SoftwareSerial: "));
            byteArrayToHexString(receiveData, receivedBytes);
	    Serial.print(F("DATA="));
	    dataConv(receiveData);
	    Serial.println(sltdata.temp);
          } else {
            Serial.println(F("No data from SoftwareSerial"));
          }
        }
      }
      
      // 入力バッファをリセット
      inputBufferIndex = 0;
      Serial.println(F("\nEnter for send")); // 次の入力を促す
    } else {
      // バッファに文字を追加
      if (inputBufferIndex < MAX_INPUT_LENGTH - 1) {
        serialInputBuffer[inputBufferIndex++] = inChar;
      } else {
        Serial.println(F("error: Overflow input buffer"));
        inputBufferIndex = 0; // バッファをリセット
      }
    }
  }

  // もしMySerialからの入力に応答したい場合（オプション）
  /*
  while (MySerial.available()) {
    char inChar = MySerial.read();
    Serial.print("MySerialから受信: ");
    Serial.println(inChar); // 受信した文字を標準シリアルに出力
    // ここで受信したデータに対する処理を記述できます
  }
  */
}

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
  int hexCharToByte(char);
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
//void byteArrayToHexStringSoft(const byte* byteArray, int length) {
//  for (int i = 0; i < length; ++i) {
//   if (byteArray[i] < 0x10) { // 1桁の場合は先頭に0を追加
//      MySerial.print("0");
//    }
//    MySerial.print(byteArray[i], HEX);
//    MySerial.print(" ");
//  }
//  MySerial.println();
//}


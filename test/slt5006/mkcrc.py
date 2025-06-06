#! /usr/bin/env python3
def crc16(data: bytes, size: int) -> int:
    cr = 0xFFFF
    for i in range(size):
        cr ^= data[i]
        for _ in range(8):
            if (cr & 0x0001):
                cr >>= 1
                cr ^= 0xA001
            else:
                cr >>= 1
    return cr

# 使用例
if __name__ == "__main__":
    hex_str = input("16進データをスペースなしで入力してください（例: 0100030a）: ").strip()
    try:
        # 2文字ずつ分割してバイト列に変換
        data = bytes.fromhex(hex_str)
    except ValueError:
        print("入力が不正です。16進数文字列を正しく入力してください。")
        exit(1)
    size = len(data)
    crc = crc16(data, size)
    print(f"CRC16: {crc:04X}")
    print(f"SEND DATA: {hex_str}{crc:04X}")
    

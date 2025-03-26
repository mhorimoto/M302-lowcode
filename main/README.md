# Main Program

## Version

* 0.01: 2023/08/08 初版
* 0.02: 2025/02/03 Makefileの改版

## UECS IDなどを記録するメモリマップ (IDmap)

0x000〜0x07f

アドレス
0x000〜0x005: UECS-ID
0x006〜0x00b: MACアドレス
0x00c: 0xff=DHCP
       0x00=固定IP
0x00d〜0x00f: 予約(0xff)
0x010〜0x013: IPアドレス (固定IPの場合)
0x014〜0x017: Subnet mask アドレス (固定IPの場合)
0x018〜0x01b: Default Gateway アドレス (固定IPの場合)
0x01c〜0x01f: DNSサーバアドレス (固定IPの場合)
0x040〜0x04f: VENDER名
0x050〜0x05f: NODE名

## CCM typeを記録するメモリマップ (CCMmap1)

0x080〜0x17f
32bytes/1record
00=cnd

byte位置 
00: enable=1/disable!=1
01: room
02: region
03-04: order (low-highの順)
05: priority
06-25: ccm type名
26: itvl インターバル
    itvl[0:1]=00=受信専用CCMのため送信しない
              01=A
              10=B
	      11=S
    itvl[2:6]=00001=1S
              00010=10S
	      00100=1M
    itvl[7]  =0 値変化時に送信しない
              1 値変化時に送信する
27-31: 予約

## CCMの精度と単位を記録するメモリマップ (CCMmap2)

0x180〜0x1ff
16bytes/1record
0x180=cnd
CCMmap1のレコード番号に応じて

CCMmap1 - CCMmap2
 0x080  -  0x180
 0x0a0  -  0x190
 0x0c0  -  0x1a0
 0x0e0  -  0x1b0
 0x100  -  0x1c0
 0x120  -  0x1d0
 0x140  -  0x1e0
 0x160  -  0x1f0
  
byte位置 
00: 精度小数点下◯桁
01-15: 単位(ASCII文字限定:15文字まで)

# Sample LOWCORE

## M252
'''
% dump                                                                          
Arduino EEPROM EEPROM DUMP                                                      
 ADDR | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F |0123456789ABCDEF       
------+-------------------------------------------------+----------------       
 0000 | 10 10 0C 00 00 04 02 A2 73 04 00 01 FF FF FF FF |........s.......       
 0010 | FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF |................       
 0020 | FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF |................       
 0030 | FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF |................       
 0040 | 48 4F 4C 4C 59 00 FF FF FF FF FF FF FF FF FF FF |HOLLY...........       
 0050 | 54 42 32 4E 5F 4D 32 35 32 00 FF FF FF FF FF FF |TB2N_M252.......       
 0060 | FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF |................       
 0070 | FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF |................       
 0080 | 01 07 01 10 00 1D 63 6E 64 2E 6D 4E 42 00 FF FF |......cnd.mNB...       
 0090 | FF FF FF FF FF FF FF FF FF FF 01 FF FF FF FF FF |................       
 00A0 | 01 07 01 10 00 0F 46 4C 4F 57 2E 6D 4E 42 00 FF |......FLOW.mNB..       
 00B0 | FF FF FF FF FF FF FF FF FF FF 05 FF FF FF FF FF |................       
 00C0 | FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF |................       
 00D0 | FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF |................       
 00E0 | FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF |................       
 00F0 | FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF |................       
'''

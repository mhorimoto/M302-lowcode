# mkeeprom

出荷時に EEPROM に初期値を登録するプログラム

* UECS ID
* MAC address
* DHCP 利用の有無
* DHCP不要のときの IP Address
* DHCP不要のときの Netmask
* DHCP不要のときの Default gatewy
* DHCP不要のときの DNS Address
* ベンダー名
* ノード名

これらをシリアルインタフェース経由で設定するプログラム。

## 設定例サンプル

'''
M302 mklc V1.20 
# list
UECS ID: 10 10 0C 00 00 0F
MAC ADDR: 02:A2:73:11:00:13
Address allocation method is DHCP.
The data below are for reference only.
IP Address/Netmask: 255.255.255.255/255.255.255.255
Default Gateway: 255.255.255.255
DNS: 255.255.255.255
Vender Name: YS-Lab
Node Name: m302n-eva01
CCM 0: E,7,19,190,29,1,0,cnd,,255,FFFFFFFFFFFF
CCM 1: E,7,19,190,15,3,3,SoilTemp,C,9,FFFFFFFFFFFF
CCM 2: E,7,19,190,15,3,3,EC_BULK,dS/m,9,FFFFFFFFFFFF
CCM 3: E,7,19,190,15,3,1,VWC_ROCK,%,9,FFFFFFFFFFFF
CCM 4: E,7,19,190,15,3,1,VWC,%,9,FFFFFFFFFFFF
CCM 5: E,7,19,190,15,3,1,VWC_COCO,%,9,FFFFFFFFFFFF
CCM 6: E,7,19,190,15,3,3,EC_PORE,dS/m,9,FFFFFFFFFFFF
CCM 7: D,255,255,-1,255,255,255,,,255,FFFFFFFFFFFF
CCM 8: D,255,255,-1,255,255,255,,,255,FFFFFFFFFFFF
CCM 9: D,255,255,-1,255,255,255,,,255,FFFFFFFFFFFF
# 
'''

## 設定コマンド

'''
set uecsid 10100C00000F
set mac 02A27311000X
set dhcp disable
set ip 192.168.120.1XX
set netmask 255.255.254.0
set vender YS-Lab
set nodename m302n12

set ccm 0 valid enable
set ccm 0 room 7
set ccm 0 region 1
set ccm 0 order 3
set ccm 0 priority 29
set ccm 0 lv 1
set ccm 0 cast 0
set ccm 0 ccmtype cnd

set ccm 1 valid enable
set ccm 1 room 7
set ccm 1 region 1
set ccm 1 order 3
set ccm 1 priority 15
set ccm 1 lv 3
set ccm 1 cast 3
set ccm 1 ccmtype SoilTemp
set ccm 1 unit C
set ccm 1 func 9

set ccm 2 valid enable
set ccm 2 room 7
set ccm 2 region 1
set ccm 2 order 3
set ccm 2 priority 15
set ccm 2 lv 3
set ccm 2 cast 3
set ccm 2 ccmtype EC_BULK
set ccm 2 unit dS/m
set ccm 2 func 9

set ccm 3 valid enable
set ccm 3 room 7
set ccm 3 region 1
set ccm 3 order 3
set ccm 3 priority 15
set ccm 3 lv 3
set ccm 3 cast 1
set ccm 3 ccmtype VWC_ROCK
set ccm 3 unit %
set ccm 3 func 9

set ccm 4 valid enable
set ccm 4 room 7
set ccm 4 region 1
set ccm 4 order 3
set ccm 4 priority 15
set ccm 4 lv 3
set ccm 4 cast 1
set ccm 4 ccmtype VWC
set ccm 4 unit %
set ccm 4 func 9

set ccm 5 valid enable
set ccm 5 room 7
set ccm 5 region 1
set ccm 5 order 3
set ccm 5 priority 15
set ccm 5 lv 3
set ccm 5 cast 1
set ccm 5 ccmtype VWC_COCO
set ccm 5 unit %
set ccm 5 func 9

set ccm 6 valid enable
set ccm 6 room 7
set ccm 6 region 1
set ccm 6 order 3
set ccm 6 priority 15
set ccm 6 lv 3
set ccm 6 cast 3
set ccm 6 ccmtype EC_PORE
set ccm 6 unit dS/m
set ccm 6 func 9

'''

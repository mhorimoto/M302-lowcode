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
# list                                                                                                                               
UECS ID: 10 10 0C 00 00 0F                                                                                                           
MAC ADDR: 02:A2:73:11:00:05                                                                                                          
Address allocation method is STATIC.                                                                                                 
The data below is important.                                                                                                         
IP Address/Netmask: 192.168.120.178/255.255.254.0                                                                                    
Default Gateway: 192.168.120.1                                                                                                       
DNS: 192.168.120.1                                                                                                                   
Vender Name: YS-LAB                                                                                                                  
Node Name: m302n05                                                                                                                   
CCM 0: E,7,1,2,29,1,0,cnd,,0,FFFFFFFFFFFF                                                                                            
CCM 1: E,7,1,2,15,3,2,InAirTemp,C,4,FFFFFFFFFFFF                                                                                     
CCM 2: E,7,1,2,15,3,2,InAirHumid,%RH,4,FFFFFFFFFFFF                                                                                  
CCM 3: D,255,255,-1,255,255,255,��������������������,����������,255,FFFFFFFFFFFF                                                     
CCM 4: D,255,255,-1,255,255,255,��������������������,����������,255,FFFFFFFFFFFF                                                     
'''


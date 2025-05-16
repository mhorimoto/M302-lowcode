#include <stdio.h>
#include <SPI.h>
#include <Ethernet2.h>
#include <EthernetUdp2.h> // UDP library from: bjoern@cs.stanford.edu 12/30/2008
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <EEPROM.h>
#include "LiquidCrystal_I2C.h"
#include <Wire.h>

#ifndef _M302_H_
#define _M302_H_
#define _M302_H_V  100

#define UECS_PORT  16520
#define CCMFMT "<?xml version=\"1.0\"?><UECS ver=\"1.00-E10\"><DATA type=\"%s\" room=\"%d\" region=\"%d\" order=\"%d\" priority=\"%d\">%s</DATA><IP>%s</IP></UECS>";

/*** EEPROM LOWCORE ASSIGN ***/
#define LC_UECS_ID        0x00
#define LC_MAC            0x06
#define FIX_DHCP_FLAG     0x0c
#define FIXED_IPADDRESS   0x10
#define FIXED_NETMASK     0x14
#define FIXED_DEFGW       0x18
#define FIXED_DNS         0x1c
#define VENDER_NAME       0x40
#define NODE_NAME         0x50
#define LC_DBGMSG         0x60  /* bit pos 0x80: Serial out, 0x40: LCD out, 0x20: Info Serial out */
#define   SO_MSG    0x80
#define   LCD_MSG   0x40
#define   SO_INFO   0x02
#define LC_CMODE          0x62  /* Force CMODE Change */

typedef struct stM302 {
  byte mac[6];
  bool dhcpflag=true;
  byte set_ip[4];
  IPAddress ip;
  IPAddress gw;
  IPAddress dns;
  IPAddress subnet;
  int cidr;
} stM302_t ;

#define LC_SEND_START       0x80  // CCM for data sending (for example cnd.aMC)
#define LC_SEND_REC_SIZE    0x30  // reserve to 0x2f step by 0x30
#define   LC_SEND_VALID     0x00  // (00) Valid Flag (0x01:valid, 0xff:invalid)
#define   LC_SEND_ROOM      0x01  // (01) Room Number (0x00:all, 1-127:room number)
#define   LC_SEND_REGION    0x02  // (02) Region Number (0x00:all, 1-127:region number)
#define   LC_SEND_ORDER     0x03  // (03-04) 2byte Low+High  (0x00:all, 1-30000:order number)
#define   LC_SEND_PRIORITY  0x05  // (05) Priority Number (0-30:priority number)
#define   LC_SEND_LV        0x06  // (06) reference LV define
#define   LC_SEND_CAST      0x07  // (07) Cast Number (0:integer, 1-255:float)
#define   LC_SEND_CCMTYPE   0x08  // (08-27) char[20] CCM Type Name
#define   LC_SEND_UNIT      0x1c  // (28-37) char[10] Unit Name
#define   LC_SEND_FUNC      0x28  // (38)  Function Number (1-254)
#define   LC_SEND_PARAM     0x29  // (39-3f) byte[6] Parameter Name

typedef struct uecsM302Send {
    byte valid;        // 0x00
    byte room;         // 0x01
    byte region;       // 0x02
    int  order;        // 0x03
    byte priority;     // 0x05
    byte lv;           // 0x06
    byte cast;         // 0x07
    char ccmtype[20];  // 0x08
    char unit[10];     // 0x1c
    byte func;         // 0x28
    char param[6];     // 0x29
} uecsM302Send_t;

/*** LV define ***/
#define LV_A1S0    1      // A-1S-0
#define LV_A1S1    2      // A-1S-1
#define LV_A10S0   3      // A-10S-0
#define LV_A10S1   4      // A-10S-1
#define LV_A1M0    5      // A-1M-0
#define LV_A1M1    6      // A-1M-1
#define LV_B0      7      // B-0
#define LV_B1      8      // B-1
#define LV_S1S0    9      // S-1S-0
#define LV_S1M0   10      // S-1M-0

/*** FUNC LIST define ***/
#define FUNC_NULL       0
#define FUNC_ANA        1
#define FUNC_DIG        2
#define FUNC_SHT3       3
#define FUNC_SHT4       4
#define FUNC_SHT5       5
#define FUNC_HWINT2     6
#define FUNC_HWINT3     7
#define FUNC_IMG_CA0012 8
#define FUNC_SLT5006    9
#define FUNC_M252      10
#define FUNC_ADC1115   11

/*** Mode via httpd ***/

#define MD_HT_IGNORE  0
#define MD_HT_STORE   1
#define MD_HT_FETCH   2
#define MD_HT_END     3
#define MD_HT_REMOCON 4

#endif

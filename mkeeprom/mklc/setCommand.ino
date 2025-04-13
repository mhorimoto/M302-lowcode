#include "M302.h"

void setCommand(char *p,int c,int a) {
  byte r,d[4];
  int l,i,ccmid;
  char sc[3],tc[17],*ptc;
  while( *p == 0x20 ) {
    p++;
  }
  
  l = strlen(p);
  switch(a) {
  case FIX_DHCP_FLAG:
    if ( !strcmp(p,"static")|| !strcmp(p,"disable")|| !strcmp(p,"no") ) {
      EEPROM.write(FIX_DHCP_FLAG,0);
    } else if ( !strcmp(p,"dhcp")|| !strcmp(p,"enable")|| !strcmp(p,"yes") ) {
      EEPROM.write(FIX_DHCP_FLAG,0xff);
    } else {
      Serial.println("Error: argument is disable or enable.");
    }
    break;
  case FIXED_IPADDRESS:
  case FIXED_NETMASK:
  case FIXED_DEFGW:
  case FIXED_DNS:
    i = 0;
    strncpy(tc,p,15);
    ptc = strtok(tc,".");
    d[i] = (byte)(atoi(ptc)&0xff);
    while(ptc!=NULL) {
      ptc = strtok(NULL,".");
      if (ptc!=NULL) {
        i++;
        d[i] = (byte)(atoi(ptc)&0xff);
      }
    }
    for (i=0;i<4;i++) {
      Serial.print(d[i]);
      Serial.print(".");
      EEPROM.write(a+i,d[i]);
    }
    Serial.println();
    break;
  case VENDER_NAME:
  case NODE_NAME:
    for (i=0;i<16;i++) {
      tc[i] = (char)NULL;
    }
    strncpy(tc,p,15);
    for (i=0;i<16;i++) {
      EEPROM.write(a+i,tc[i]);
    }
    break;
  case LC_SEND_VALID:
  case LC_SEND_ROOM:
  case LC_SEND_REGION:
  case LC_SEND_ORDER:
  case LC_SEND_PRIORITY:
  case LC_SEND_LV:
  case LC_SEND_CAST:
  case LC_SEND_CCMTYPE:
  case LC_SEND_UNIT:
  case LC_SEND_FUNC:
  case LC_SEND_PARAM:
    if (c==2) {
      strncpy(sc,p,2);
      sc[2]=(char)NULL;
      r = (byte)(strtol(sc,0,16)&0xff);
      EEPROM.write(a,r);
      Serial.print(r,HEX);
    } else if (c==4) {
      strncpy(sc,p,4);
      sc[4]=(char)NULL;
      r = (byte)(strtol(sc,0,16)&0xffff);
      EEPROM.put(a,r);
      Serial.print(r,HEX);
    } else if (c==6) {
      strncpy(sc,p,6);
      sc[6]=(char)NULL;
      r = (byte)(strtol(sc,0,16)&0xffffff);
      EEPROM.put(a,r);
      Serial.print(r,HEX);
    }
    Serial.println();
    break;
  default:
    if (l==c) {
      for(i=0;i<l;i+=2) {
        strncpy(sc,p+i,2);
        sc[2]=(char)NULL;
        r = (byte)(strtol(sc,0,16)&0xff);
        EEPROM.write(a,r);
        a++;
        Serial.print(r,HEX);
      }
      Serial.println();
    }
    break;
  }
}


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
                Serial.println(F("Error: argument is disable or enable."));
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

void setCCMCommand(char *p,int c,int a,int ccmid) {
    byte r,d[4];
    int l,i,ccmoffset;
    char sc[3],tc[21],*ptc;

    ccmoffset = LC_SEND_START + (ccmid * LC_SEND_REC_SIZE);
    if (ccmid>0x3f) {
        Serial.println(F("OUT of RANGE"));
        return;
    }

    while( *p == 0x20 ) {
        p++;
    }
    
    l = strlen(p);
    switch(a) {
        case LC_SEND_VALID:
            if ( !strcmp(p,"disable")|| !strcmp(p,"no") ) {
                EEPROM.write(LC_SEND_VALID+ccmoffset,0xff);
            } else if ( !strcmp(p,"enable")|| !strcmp(p,"yes") ) {
                EEPROM.write(LC_SEND_VALID+ccmoffset,0x01);
            } else {
                Serial.println(F("Error: argument is disable or enable."));
            }
            break;
        case LC_SEND_ROOM:
            if ( !strcmp(p,"all") ) {
                EEPROM.write(LC_SEND_ROOM+ccmoffset,0x00);
            } else if (atoi(p)>0 && atoi(p)<128) {
                EEPROM.write(LC_SEND_ROOM+ccmoffset,(byte)(atoi(p)&0x7f));
            } else {
                Serial.println(F("Error: argument is 1-127 or all."));
            }
            break;
        case LC_SEND_REGION:
            if ( !strcmp(p,"all") ) {
                EEPROM.write(LC_SEND_REGION+ccmoffset,0x00);
            } else if (atoi(p)>0 && atoi(p)<128) {
                EEPROM.write(LC_SEND_REGION+ccmoffset,(byte)(atoi(p)&0x7f));
            } else {
                Serial.println(F("Error: argument is 1-127 or all."));
            }
            break;
        case LC_SEND_ORDER:
            if ( !strcmp(p,"all") ) {
                EEPROM.put(LC_SEND_ORDER+ccmoffset,(int)0x0000);
            } else if (atoi(p)>0 && atoi(p)<30001) {
                EEPROM.put(LC_SEND_ORDER+ccmoffset,(int)(atoi(p)));
            } else {
                Serial.println(F("Error: argument is 1-30000 or all."));
            }
            break;
        case LC_SEND_PRIORITY:
            if (atoi(p)>=0 && atoi(p)<=30) {
                EEPROM.write(LC_SEND_PRIORITY+ccmoffset,(byte)(atoi(p)));
            } else {
                Serial.println(F("Error: argument is 0-30."));
            }
            break;
        case LC_SEND_LV:
            if (atoi(p)>0 && atoi(p)<=10) {
                EEPROM.write(LC_SEND_LV+ccmoffset,(byte)(atoi(p)));
            } else {
                Serial.println(F("Error: argument is 1-10."));
            }
            break;
        case LC_SEND_CAST:
            if (atoi(p)>=0 && atoi(p)<=255) {
                EEPROM.write(LC_SEND_CAST+ccmoffset,(byte)(atoi(p)));
            } else {
                Serial.println(F("Error: argument is 0-255."));
            }
            break;
        case LC_SEND_CCMTYPE:
            for (i=0;i<21;i++) {
                if (*p==(char)NULL) break;
                EEPROM.write(LC_SEND_CCMTYPE+i+ccmoffset,(char)(*p));
                p++;
            }
            for (;i<21;i++) {
                EEPROM.write(LC_SEND_CCMTYPE+i+ccmoffset,(char)NULL);
            }
            break;
        case LC_SEND_UNIT:
            for (i=0;i<11;i++) {
                if (*p==(char)NULL) break;
                EEPROM.write(LC_SEND_UNIT+i+ccmoffset,(char)(*p));
                p++;
            }
            for (;i<11;i++) {
                EEPROM.write(LC_SEND_UNIT+i+ccmoffset,(char)NULL);
            }
            break;
        case LC_SEND_FUNC:
            if (atoi(p)>=0 && atoi(p)<=250) {
                EEPROM.write(LC_SEND_FUNC+ccmoffset,(byte)(atoi(p)));
            } else {
                Serial.println(F("Error: argument is 0-250."));
            }
            break;
        case LC_SEND_PARAM:
            strncpy(sc,p,6);
            sc[6]=(char)NULL;
            r = (byte)(strtol(sc,0,16)&0xffffff);
            EEPROM.put(a,r);
            Serial.print(r,HEX);
            break;
        default:
            Serial.println();
            break;
    }
}


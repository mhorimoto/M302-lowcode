#include "M302.h"

void listCommand(void) {
    int i,lc_order,ccmoffset;
    char lbf[80];
    char sbf[21];
    char lc_valid;
    byte lc_room,lc_region,lc_priority,lc_lv,lc_cast,lc_func;

    // UECS ID
    sprintf(lbf,"UECS ID: %02X %02X %02X %02X %02X %02X",
        EEPROM.read(LC_UECS_ID),EEPROM.read(LC_UECS_ID+1),EEPROM.read(LC_UECS_ID+2),
        EEPROM.read(LC_UECS_ID+3),EEPROM.read(LC_UECS_ID+4),EEPROM.read(LC_UECS_ID+5));
    Serial.println(lbf);
    // MAC Address
    sprintf(lbf,"MAC ADDR: %02X:%02X:%02X:%02X:%02X:%02X",
        EEPROM.read(LC_MAC),EEPROM.read(LC_MAC+1),EEPROM.read(LC_MAC+2),
        EEPROM.read(LC_MAC+3),EEPROM.read(LC_MAC+4),EEPROM.read(LC_MAC+5));
    Serial.println(lbf);
    // FIXED or DHCP
    Serial.print(F("Address allocation method is "));
    if (EEPROM.read(FIX_DHCP_FLAG)==0) {
        Serial.println(F("STATIC."));
        Serial.println(F("The data below is important."));
    } else {
        Serial.println(F("DHCP."));
        Serial.println(F("The data below are for reference only."));
    }
    // IP Address
    sprintf(lbf,"IP Address/Netmask: %d.%d.%d.%d/%d.%d.%d.%d",
        EEPROM.read(FIXED_IPADDRESS),EEPROM.read(FIXED_IPADDRESS+1),
        EEPROM.read(FIXED_IPADDRESS+2),EEPROM.read(FIXED_IPADDRESS+3),
        EEPROM.read(FIXED_NETMASK),EEPROM.read(FIXED_NETMASK+1),
        EEPROM.read(FIXED_NETMASK+2),EEPROM.read(FIXED_NETMASK+3));
    Serial.println(lbf);
    // Default gateway
    sprintf(lbf,"Default Gateway: %d.%d.%d.%d",
        EEPROM.read(FIXED_DEFGW),EEPROM.read(FIXED_DEFGW+1),
        EEPROM.read(FIXED_DEFGW+2),EEPROM.read(FIXED_DEFGW+3));
    Serial.println(lbf);
    // DNS
    sprintf(lbf,"DNS: %d.%d.%d.%d",
        EEPROM.read(FIXED_DNS),EEPROM.read(FIXED_DNS+1),
        EEPROM.read(FIXED_DNS+2),EEPROM.read(FIXED_DNS+3));
    Serial.println(lbf);
    // Vendor name
    for (i=0;i<16;i++) {
    sbf[i] = EEPROM.read(VENDER_NAME+i);
        if (sbf[i]==(char)NULL) break;
    }
    sprintf(lbf,"Vender Name: %s",sbf);
    Serial.println(lbf);
    // Node name
    for (i=0;i<16;i++) {
        sbf[i] = EEPROM.read(NODE_NAME+i);
        if (sbf[i]==(char)NULL) break;
    }
    sprintf(lbf,"Node Name: %s",sbf);
    Serial.println(lbf);
    // CCM Send
    for (i=0;i<10;i++) {
        ccmoffset = LC_SEND_START + LC_SEND_VALID + (i * LC_SEND_REC_SIZE);
        if (EEPROM.read(ccmoffset)==0x01) {
            lc_valid = 'E';
        } else {
            lc_valid = 'D';
        }
        ccmoffset = LC_SEND_START + LC_SEND_ROOM + (i * LC_SEND_REC_SIZE);
        lc_room = EEPROM.read(ccmoffset);
        ccmoffset = LC_SEND_START + LC_SEND_REGION + (i * LC_SEND_REC_SIZE);
        lc_region = EEPROM.read(ccmoffset);
        ccmoffset = LC_SEND_START + LC_SEND_ORDER + (i * LC_SEND_REC_SIZE);
        lc_order = EEPROM.read(ccmoffset) + (EEPROM.read(ccmoffset+1) << 8);
        ccmoffset = LC_SEND_START + LC_SEND_PRIORITY + (i * LC_SEND_REC_SIZE);
        lc_priority = EEPROM.read(ccmoffset);
        ccmoffset = LC_SEND_START + LC_SEND_LV + (i * LC_SEND_REC_SIZE);
        lc_lv = EEPROM.read(ccmoffset);
        ccmoffset = LC_SEND_START + LC_SEND_CAST + (i * LC_SEND_REC_SIZE);
        lc_cast = EEPROM.read(ccmoffset);
        ccmoffset = LC_SEND_START + LC_SEND_FUNC + (i * LC_SEND_REC_SIZE);
        lc_func = EEPROM.read(ccmoffset);
        ccmoffset = LC_SEND_START + LC_SEND_CCMTYPE + (i * LC_SEND_REC_SIZE);
        for (int j=0;j<20;j++) {
            sbf[j] = EEPROM.read(ccmoffset+j);
            if (sbf[j]==(char)NULL) break;
        }
        sbf[20] = (char)NULL;
        sprintf(lbf,"CCM %d: %c,%d,%d,%d,%d,%d,%d,%s",
            i,lc_valid,lc_room,lc_region,lc_order,lc_priority,
            lc_lv,lc_cast,sbf);
        Serial.print(lbf);
        ccmoffset = LC_SEND_START + LC_SEND_UNIT + (i * LC_SEND_REC_SIZE);
        for (int j=0;j<10;j++) {
            sbf[j] = EEPROM.read(ccmoffset+j);
            if (sbf[j]==(char)NULL) break;
        }
        sbf[10] = (char)NULL;
        sprintf(lbf,",%s,%d",sbf,lc_func);
        Serial.print(lbf);
        ccmoffset = LC_SEND_START + LC_SEND_PARAM + (i * LC_SEND_REC_SIZE);
        sprintf(lbf,",%02X%02X%02X%02X%02X%02X",
            EEPROM.read(ccmoffset),EEPROM.read(ccmoffset+1),
            EEPROM.read(ccmoffset+2),EEPROM.read(ccmoffset+3),
            EEPROM.read(ccmoffset+4),EEPROM.read(ccmoffset+5));
        Serial.println(lbf);
    }
}

#include "M302.h"

void listCommand(void) {
  int i;
  char lbf[80];
  char sbf[21];
  
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
}

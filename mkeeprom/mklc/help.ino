void help(void) {
  Serial.println(pgname);
  Serial.println(F("*** Help ***"));
  Serial.println(F("list: Displays the current settings."));
  Serial.println(F("set uecsid [UECSID]: Set the UECSID, which is a 12-digit hex number."));
  Serial.println(F("                For example, 10100C000004"));
  Serial.println(F("set mac [MAC]: Set the MAC Address, which is a 12-digit hex number."));
  Serial.println(F("              For example, 02A27304002F (No colon is needed)"));
  Serial.println(F("set dhcp [enable/disable]: Set DHCP mode, enable or disable"));
  Serial.println(F("set ip [IP]: Set IP address, which is like as 192.168.31.10"));
  Serial.println(F("set netmask [NETMASK]: Set Netmask, which is like as 255.255.255.0"));
  Serial.println(F("set gw [IP]: Set gateway address, which is like as 192.168.31.254"));
  Serial.println(F("set dns [IP]: Set DNS address, which is like as 192.168.31.254"));
  Serial.println(F("set vender [name]: Set vendor name, which is ascii code 15 characters or less"));
  Serial.println(F("set nodename [name]: Set node name, which is ascii code 15 characters or less"));
  Serial.println(F("set ccm [ccmid] [subcommand]: "));
  Serial.println(F("  Set CCM command, ccmid is 0-4, "));
  Serial.println(F("     subcommand is 0:valid, 1:room, 2:region, 3:order, 4:priority"));
  Serial.println(F("                   5:lv, 6:cast, 7:ccmtype, 8:unit, 9:func, a:param"));
}

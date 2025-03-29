void help(void) {
  Serial.println(pgname);
  Serial.println(F("*** Help ***"));
  Serial.println(F("list: Displays the current settings."));
  Serial.println(F("setid [UECSID]: Set the UECSID, which is a 12-digit hex number."));
  Serial.println(F("                For example, 10100C000004"));
  Serial.println(F("setmac [MAC]: Set the MAC Address, which is a 12-digit hex number."));
  Serial.println(F("              For example, 02A27304002F (No colon is needed)"));
  Serial.println(F("setdhcp [enable/disable]: Set DHCP mode, enable or disable"));
  Serial.println(F("setip [IP]: Set IP address, which is like as 192.168.31.10"));
  Serial.println(F("setmask [NETMASK]: Set Netmask, which is like as 255.255.255.0"));
  Serial.println(F("setgw [IP]: Set gateway address, which is like as 192.168.31.254"));
  Serial.println(F("setdns [IP]: Set DNS address, which is like as 192.168.31.254"));
  Serial.println(F("setvender [name]: Set vendor name, which is ascii code 15 characters or less"));
  Serial.println(F("setnodename [name]: Set node name, which is ascii code 15 characters or less"));
  Serial.println(F("clearpage [page]: Initialize with 0xff in 256-byte page units."));
}

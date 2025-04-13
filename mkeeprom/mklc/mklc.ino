#include "M302.h"
#if _M302_H_V < 100
#pragma message("Library M302 is old. Version 1.00 or higher is required.")
#else

#define INPUT_LINE_SIZE  30

char *pgname = "M302 mklc V1.10 ";
char inputbuf[INPUT_LINE_SIZE],*ptr_inputbuf;
int  cnt;

//LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

void setup(void) {
  String linebuf,inputbuf;

  Serial.begin(115200);
//  lcd.init();
//  lcd.backlight();
//  lcd.clear();
  linebuf = String(pgname);
//  lcd.print(linebuf[1]);
//  lcd.setCursor(0,1);
//  lcd.print(F("SERIAL 115200bps"));
  Serial.println(pgname);
  ptr_inputbuf = &inputbuf[0];
  cnt = 0;
}


void loop(void) {
  char ch,sc[3];
  byte r;
  int  sl,i;
  static bool cr = true;
  extern void listCommand(void);
  extern void setCommand(char *,int,int);
  extern void clearpage(char *);
  extern void help(void);
  
  if ((cnt==0) && cr) {
    Serial.print(F("$ "));
    cr = false;
  }
  if (Serial.available() > 0) {
    ch = Serial.read();
    if ((ch=='\r')||(ch=='\n')) {
      cr = true;
      Serial.println();
    } else if ((ch==(char)0x08)||(ch==(char)0x7f)) {
      if (cnt>0) {
	cnt--;
	Serial.print(ch);
	Serial.print(" ");
	Serial.print(ch);
      }
    } else if ((ch>=(char)0x20)&&(ch<(char)0x7f)) {
      Serial.print(ch);
      inputbuf[cnt] = ch;
      cnt++;
    }
    inputbuf[cnt] = (char)NULL;
    if (cnt>=INPUT_LINE_SIZE) {
      Serial.println(F("OV"));
      cnt=0;
    }
  }

  // Command operation
  if (cr) {
    ptr_inputbuf = &inputbuf[0];
    if ( !strncmp(ptr_inputbuf,"list",INPUT_LINE_SIZE-1) ) {
      Serial.println(F("LIST COMMAND"));
      listCommand();
    }
    if ( !strncmp(ptr_inputbuf,"setid ",6) ) {
      ptr_inputbuf += 6;
      setCommand(ptr_inputbuf,12,LC_UECS_ID);
    }
    if ( !strncmp(ptr_inputbuf,"setmac ",7) ) {
      ptr_inputbuf += 7;
      setCommand(ptr_inputbuf,12,LC_MAC);
    }
    if ( !strncmp(ptr_inputbuf,"setdhcp ",8) ) {
      ptr_inputbuf += 8;
      setCommand(ptr_inputbuf,1,FIX_DHCP_FLAG);
    }
    if ( !strncmp(ptr_inputbuf,"setip ",6) ) {
      ptr_inputbuf += 6;
      setCommand(ptr_inputbuf,15,FIXED_IPADDRESS);
    }
    if ( !strncmp(ptr_inputbuf,"setmask ",8) ) {
      ptr_inputbuf += 8;
      setCommand(ptr_inputbuf,15,FIXED_NETMASK);
    }
    if ( !strncmp(ptr_inputbuf,"setgw ",6) ) {
      ptr_inputbuf += 6;
      setCommand(ptr_inputbuf,15,FIXED_DEFGW);
    }
    if ( !strncmp(ptr_inputbuf,"setdns ",7) ) {
      ptr_inputbuf += 7;
      setCommand(ptr_inputbuf,15,FIXED_DNS);
    }
    if ( !strncmp(ptr_inputbuf,"setvender ",10) ) {
      ptr_inputbuf += 10;
      setCommand(ptr_inputbuf,16,VENDER_NAME);
    }
    if ( !strncmp(ptr_inputbuf,"setnodename ",12) ) {
      ptr_inputbuf += 12;
      setCommand(ptr_inputbuf,16,NODE_NAME);
    }
    if ( !strncmp(ptr_inputbuf,"clearpage ",10) ) {
      ptr_inputbuf += 10;
      clearpage(ptr_inputbuf);
    }
    if ( !strncmp(ptr_inputbuf,"setvalid ",9) ) {
      ptr_inputbuf += 9;
      setCommand(ptr_inputbuf,2,LC_SEND_VALID);
    }
    if ( !strncmp(ptr_inputbuf,"setroom ",8) ) {
      ptr_inputbuf += 8;
      setCommand(ptr_inputbuf,2,LC_SEND_ROOM);
    }
    if ( !strncmp(ptr_inputbuf,"setregion ",10) ) {
      ptr_inputbuf += 10;
      setCommand(ptr_inputbuf,2,LC_SEND_REGION);
    }
    if ( !strncmp(ptr_inputbuf,"setorder ",9) ) {
      ptr_inputbuf += 9;
      setCommand(ptr_inputbuf,4,LC_SEND_ORDER);
    }
    if ( !strncmp(ptr_inputbuf,"setpriority ",12) ) {
      ptr_inputbuf += 12;
      setCommand(ptr_inputbuf,2,LC_SEND_PRIORITY);
    }
    if ( !strncmp(ptr_inputbuf,"setlv ",6) ) {
      ptr_inputbuf += 6;
      setCommand(ptr_inputbuf,2,LC_SEND_LV);
    }
    if ( !strncmp(ptr_inputbuf,"setcast ",8) ) {
      ptr_inputbuf += 8;
      setCommand(ptr_inputbuf,2,LC_SEND_CAST);
    }
    if ( !strncmp(ptr_inputbuf,"setccmtype ",11) ) {
      ptr_inputbuf += 11;
      setCommand(ptr_inputbuf,20,LC_SEND_CCMTYPE);
    }
    if ( !strncmp(ptr_inputbuf,"setccmcast ",11) ) {
      ptr_inputbuf += 11;
      setCommand(ptr_inputbuf,20,LC_SEND_CAST);
    }
    if ( !strncmp(ptr_inputbuf,"setfunc ",8) ) {
      ptr_inputbuf += 8;
      setCommand(ptr_inputbuf,2,LC_SEND_FUNC);
    }
    if ( !strncmp(ptr_inputbuf,"setparam ",9) ) {
      ptr_inputbuf += 9;
      setCommand(ptr_inputbuf,12,LC_SEND_PARAM);
    }
    if ( !strncmp(ptr_inputbuf,"setunit ",8) ) {
      ptr_inputbuf += 8;
      setCommand(ptr_inputbuf,12,LC_SEND_UNIT);
    }
    if ( !strncmp(ptr_inputbuf,"help",4)) {
      help();
    }
    Serial.println(F("OK"));
    cnt = 0;
  }
}



#endif

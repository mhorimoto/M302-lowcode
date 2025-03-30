///////////////////////////////////////////////////////////////////
// M302-lowcode for TB2N
//  MIT License
//  Copyright (c) 2025 Masafumi Horimoto
//  Release on 
//  
///////////////////////////////////////////////////////////////////

#include "M302.h"

#ifndef W5500SS
#define W5500SS 10
#endif

uint8_t mcusr_mirror __attribute__ ((section (".noinit")));
void get_mcusr(void)     \
  __attribute__((naked)) \
  __attribute__((section(".init3")));
void get_mcusr(void) {
  mcusr_mirror = MCUSR;
  MCUSR = 0;
  wdt_disable();
}


#define  pCND        0x80
#define  pRADIATION  0xa0
#define  delayMillis 5000UL // 5sec
#define  LED2        3


const char VERSION[16] PROGMEM = "TB2N 0.13";

char          uecsid[6], uecstext[180];
unsigned long cndVal;   // CCM cnd Value
char     val[16];


/////////////////////////////////////
// Hardware Define
/////////////////////////////////////

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display
char              lcdtext[2][17];
stM302_t          st_m302;

IPAddress   broadcastIP,networkADDR;
EthernetUDP Udp16520,Udp16521,Udp16528,Udp16529;

volatile int period1sec = 0;
volatile int period10sec = 0;
volatile int period60sec = 0;

//int packetSize;

void setup(void) {
  int i,er;
  const char *ids PROGMEM = "%s:%02X%02X%02X%02X%02X%02X";
  extern void lcdout(int,int,int);

  pinMode(LED2,OUTPUT);
  digitalWrite(LED2,LOW);
  pinMode(4,INPUT_PULLUP);
  pinMode(5,INPUT_PULLUP);
  pinMode(6,INPUT_PULLUP);
  pinMode(7,OUTPUT);
  pinMode(8,OUTPUT);
  pinMode(9,OUTPUT);

  cndVal = 0L;    // Reset cnd value
  lcd.init();
  lcd.backlight();
  configure_wdt();
  EEPROM.get(LC_UECS_ID,uecsid);
  EEPROM.get(LC_MAC,st_m302.mac);
  if (EEPROM.read(FIX_DHCP_FLAG)==0) {
    st_m302.dhcpflag = false;
    EEPROM.get(FIXED_IPADDRESS,st_m302.set_ip);
    for(i=0;i<4;i++) {
      st_m302.subnet[i] = EEPROM.read(FIXED_NETMASK+i);
      st_m302.gw[i]     = EEPROM.read(FIXED_DEFGW+i);
      st_m302.dns[i]    = EEPROM.read(FIXED_DNS+i);
    }
  }

  for(i=0;i<16;i++) {
    lcdtext[0][i] = pgm_read_byte(&(VERSION[i]));
  }
  wdt_reset();
  lcdtext[0][i] = 0;
  sprintf(lcdtext[1],ids,"ID",
          uecsid[0],uecsid[1],uecsid[2],uecsid[3],uecsid[4],uecsid[5]);
  lcdout(0,1,1);
  if (digitalRead(4)==HIGH) { // 通常運転
    Serial.begin(19200);
  } else {
    Serial.begin(115200);
    Serial.println(lcdtext[0]);
    delay(50);
  }
  Ethernet.init(W5500SS);
  wdt_reset();
  if (st_m302.dhcpflag) {
    er = Ethernet.begin(st_m302.mac);
    st_m302.subnet = Ethernet.subnetMask();
  } else {
    Ethernet.begin(st_m302.mac,st_m302.set_ip,st_m302.dns,st_m302.gw,st_m302.subnet);
    er = 1;
  }
  if (er==0) {
    sprintf(lcdtext[1],"NFL");
  } else {
    st_m302.ip = Ethernet.localIP();
    for(i=0;i<4;i++) {
      networkADDR[i] = st_m302.subnet[i] & st_m302.ip[i];
      broadcastIP[i] = ~st_m302.subnet[i]|networkADDR[i];
    }

    wdt_reset();
    delay(100);
    Udp16520.begin(16520);
    Udp16528.begin(16528);
  }

  //**********************************
  //
  //  Initialize of Sensor devices
  //
  //**********************************

  wdt_reset();
  cndVal |= 0x00000001;  // Setup completed
  delay(100);
  //
  // Setup Timer1 Interrupt
  //
  TCCR1A  = 0;
  TCCR1B  = 0;
  TCCR1B |= (1 << WGM12) | (1 << CS12) | (1 << CS10);  //CTCmode //prescaler to 1024
  OCR1A   = 15625-1;
  TIMSK1 |= (1 << OCIE1A);
}

/////////////////////////////////
// Reset Function goto Address 0
/////////////////////////////////
void(*resetFunc)(void) = 0;

extern void lcdout(int,int,int);
extern int setParam(char *);
extern void dumpLowCore(void);
  
#define CCMFMT "<?xml version=\"1.0\"?><UECS ver=\"1.00-E10\"><DATA type=\"%s\" room=\"%d\" region=\"%d\" order=\"%d\" priority=\"%d\">%s</DATA><IP>%s</IP></UECS>";

int dk=0;

void lcd_display_loop(void) {
  dk++;
  switch(dk) {
  case 3:
    lcdout(0,2,1);
    break;
  case 4:
    lcdout(0,3,1);
    break;
  case 5:
    dk = 0;
    lcdout(0,4,1);
    break;
  default:
    lcdout(dk,4,1);
  }
}

float sens_ana(int aport,int map_low,int map_high,float slope) {
  int sval,vol;
  float r;
  sval = analogRead(aport);
  vol  = map(sval,0,1023,map_low,map_high);
  r    = vol * slope;
  return r;
}

void getM252(int ccmid,bool f) {
  char *xmlDT PROGMEM = CCMFMT;
  extern char val[];
  if (f==false) {
    digitalWrite(LED2,HIGH);
    if (digitalRead(4)==HIGH) {
      Serial.write('a');
      delay(100);
      if (Serial.available() > 0) { // 受信データを読み込む
        String receivedData = Serial.readStringUntil(0x0a); // 改行コードが来るまで読み込む
        receivedData.trim(); // 前後の空白を削除
        receivedData.replace("\r", ""); // 改行コードを削除
        receivedData.replace("\n", ""); // 改行コードを削除
        receivedData.toCharArray(val,16);
      }
    } else {
      val[0] = 'N';
      val[1] = 'a';
      val[2] = 'N';
      val[3] = 0;
    }
    digitalWrite(LED2,LOW);
  } else {
    uecsSendData(ccmid,xmlDT,val,0);
  }
}

/////////////////////////////////
void loop() {
  static int k=0;
  int i,ia,ta,tb,cdsv;
  byte room,region,priority,interval;
  int  order;
  int  inchar ;
  float ther,humi;
  char name[10],dname[11];
  extern char val[];
  extern void recv16528port(void);
  
  char *xmlDT PROGMEM = CCMFMT;
  const char *ids PROGMEM = "%s:%02X%02X%02X%02X%02X%02X";
  
  recv16528port();
  wdt_reset();
  
  // 10 sec interval
  if (period10sec==1) {
    UserEvery10Seconds();
    period10sec=0;
    wdt_reset();
  }
   // 1 min interval
   if (period60sec==1) {
     UserEveryMinute();
     period60sec = 0;
     wdt_reset();
   }
   //1 sec interval
   if (period1sec==1) {
      period1sec = 0;
      ia = 0; // cnd
      sprintf(val,"%u",cndVal);
      uecsSendData(0,xmlDT,val,0);     // cnd
      cndVal &= 0xfffffffe;            // Clear setup completed flag
      UserEverySecond();
   }
   wdt_reset();
}

ISR(TIMER1_COMPA_vect) {
  static byte cnt10,cnt60;
  cnt10++;
  cnt60++;
  period1sec = 1;
  if (cnt10 >= 10) {
    cnt10 = 0;
    period10sec = 1;
  }
  if (cnt60 >= 60) {
    cnt60 = 0;
    period60sec = 1;
  }
}


void configure_wdt(void) {
  cli();                           // disable interrupts for changing the registers
  MCUSR = 0;                       // reset status register flags
                                   // Put timer in interrupt-only mode:
  WDTCSR |= 0b00011000;            // Set WDCE (5th from left) and WDE (4th from left) to enter config mode,
                                   // using bitwise OR assignment (leaves other bits unchanged).
  WDTCSR =  0b00001000 | 0b100001; // clr WDIE: interrupt enabled
                                   // set WDE: reset disabled
                                   // and set delay interval (right side of bar) to 8 seconds
  sei();                           // re-enable interrupts
                                   // reminder of the definitions for the time before firing
                                   // delay interval patterns:
                                   //  16 ms:     0b000000
                                   //  500 ms:    0b000101
                                   //  1 second:  0b000110
                                   //  2 seconds: 0b000111
                                   //  4 seconds: 0b100000
                                   //  8 seconds: 0b100001
}

void uecsSendData(int id,char *xmlDT,char *tval,int z) {
  byte room,region,priority,interval;
  int  order,i,a;
  char name[20],dname[21],strIP[17];
  extern stM302_t st_m302;

  a = id * LC_SEND_REC_SIZE + LC_SEND_START;
  EEPROM.get(a+LC_SEND_ROOM,room);
  EEPROM.get(a+LC_SEND_REGION,region);
  EEPROM.get(a+LC_SEND_ORDER,order);
  EEPROM.get(a+LC_SEND_PRIORITY,priority);
  EEPROM.get(a+LC_SEND_LV,interval);
  EEPROM.get(a+LC_SEND_CCMTYPE,name);
  //  for(i=0;i<10;i++) {
  //    dname[i] = name[i];
  //    if (name[i]==NULL) break;
  //  }
  //  dname[i] = NULL;
  sprintf(strIP,"%d.%d.%d.%d",st_m302.ip[0],st_m302.ip[1],st_m302.ip[2],st_m302.ip[3]);
  sprintf(uecstext,xmlDT,name,room,region,order,priority+z,tval,strIP);
  Udp16520.beginPacket(broadcastIP,16520);
  Udp16520.write(uecstext);
  Udp16520.endPacket();
}

void UserEverySecond(void) {
  volatile bool aaa;
  volatile byte a=0 ;
  char val[6];
  int ia,l;
  char *xmlDT PROGMEM = CCMFMT;

  cndVal &= 0xfffffffe;            // Clear setup completed flag
  if (aaa) {
    lcd.setCursor(15,0);
    aaa=false;
    lcd.print(">");
  } else {
    lcd.setCursor(15,0);
    aaa=true;
    lcd.print("<");
  }
  wdt_reset();
}

void UserEvery10Seconds(void) {
  char *xmlDT PROGMEM = CCMFMT;
  extern void lcdout(int,int,int);
  extern void getM252(int,bool);

  getM252(1,false);
  wdt_reset();
}

void UserEveryMinute(void) {
  char *xmlDT PROGMEM = CCMFMT;
  extern void lcdout(int,int,int);
  extern void getM252(int,bool);

  getM252(1,true);
  wdt_reset();
}


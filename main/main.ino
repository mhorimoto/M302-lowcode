///////////////////////////////////////////////////////////////////
// M302-lowcode for SLT5006
//  MIT License
//  Copyright (c) 2025 Masafumi Horimoto
//  Release on 
//  
///////////////////////////////////////////////////////////////////
#define VERSION "M302 V3.00"

#include "M302.h"
#include <SLT5006.h>
#include <avr/wdt.h>

#ifndef W5500SS
#define W5500SS 10
#endif

#define  delayMillis 5000UL // 5sec
#define  LED2        3

char          uecsid[6], uecstext[180];
unsigned long cndVal;   // CCM cnd Value
char          val[16];
bool          useSerial = false;

/////////////////////////////////////
// Hardware Define
/////////////////////////////////////

stM302_t    st_m302;
SLT5006     slt(A5,A4);
IPAddress   broadcastIP,networkADDR;
EthernetUDP Udp16520,Udp16521,Udp16528,Udp16529;

volatile int period1sec = 0;
volatile int period10sec = 0;
volatile int period60sec = 0;

unsigned long previousMillis = 0; // 前回の時刻を記録
const unsigned long interval = 1000; // 1秒（1000ms）間隔

void setup(void) {
  char *xmlDT PROGMEM = CCMFMT;
  int i,er;
  const char *ids PROGMEM = "%s:%02X%02X%02X%02X%02X%02X";
  extern unsigned short crc16(int,byte *);
    
  pinMode(LED2,OUTPUT);
  digitalWrite(LED2,LOW);
  pinMode(4,INPUT_PULLUP);
  pinMode(5,INPUT_PULLUP);
  pinMode(6,INPUT_PULLUP);
  pinMode(7,OUTPUT);
  pinMode(8,OUTPUT);
  pinMode(9,OUTPUT);
    
  cndVal = 0L;    // Reset cnd value
  wdt_enable(WDTO_8S);
  //  configure_wdt();
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
  wdt_reset();
  if (digitalRead(4)==HIGH) { // 通常運転
    useSerial = false;
    Serial.begin(9600);
    Serial.println(F(VERSION));
  } else {
    useSerial = true;
    Serial.begin(115200);
    Serial.println(F(VERSION));
  }
  delay(50);
  Ethernet.init(W5500SS);
  delay(300);
  wdt_reset();
  if (st_m302.dhcpflag) {
    er = Ethernet.begin(st_m302.mac);
    st_m302.subnet = Ethernet.subnetMask();
  } else {
    Ethernet.begin(st_m302.mac,st_m302.set_ip,st_m302.dns,st_m302.gw,st_m302.subnet);
    er = 1;
  }
  if (er==0) {
    if (useSerial) {
      Serial.println(F("DHCP Failed"));
    }
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
    delay(500);
  }
    
  //**********************************
  //
  //  Initialize of Sensor devices
  //
  //**********************************

  slt.begin();  // SLT5006初期化
  wdt_reset();
  uecsSendData(0,xmlDT,"395264",0);     // start cnd
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

float sens_ana(int aport,int map_low,int map_high,float slope) {
    int sval,vol;
    float r;
    sval = analogRead(aport);
    vol  = map(sval,0,1023,map_low,map_high);
    r    = vol * slope;
    return r;
}

void ope_slt5006(void) {
  char *xmlDT PROGMEM = CCMFMT;
  if (slt.readSensor()) {
    // 温度送信
    dtostrf(slt.getTemp(), -6, 3, val);
    uecsSendData(1, xmlDT, val, 0);
    
    // EC Bulk送信
    dtostrf(slt.getECBulk(), -6, 3, val);
    uecsSendData(2, xmlDT, val, 0);
    
    // VWC Rock送信
    dtostrf(slt.getVWCRock(), -5, 1, val);
    uecsSendData(3, xmlDT, val, 0);
    
    // VWC送信
    dtostrf(slt.getVWC(), -5, 1, val);
    uecsSendData(4, xmlDT, val, 0);
    
    // VWC Coco送信
    dtostrf(slt.getVWCCoco(), -5, 1, val);
    uecsSendData(5, xmlDT, val, 0);
    
    // EC Pore送信
    dtostrf(slt.getECPore(), -6, 3, val);
    uecsSendData(6, xmlDT, val, 0);
    cndVal = 0;
  } else {
    cndVal = 0x20000900;
  }
}

/////////////////////////////////
void loop() {
  static int k=0;
  int i,ia,ta,tb,cdsv;
  extern void recv16528port(void);
    
  recv16528port();
  wdt_reset();
    
  //1 sec interval
  if (period1sec==1) {
    period1sec = 0;
    UserEverySecond();
  }
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

void replaceSpaceWithNull(char *t) {
  if (!t) return;  // NULLポインタ保護
  while (*t) {
    if (*t == 0x20) {
      *t = 0x00;
      break;
    }
    t++;
  }
}

void uecsSendData(int id,char *xmlDT,char *tval,int z) {
    byte room,region,priority;
    int  order,i,a;
    char name[20],dname[21],strIP[17];
    void replaceSpaceWithNull(char *);
    extern stM302_t st_m302;
    
    a = id * LC_SEND_REC_SIZE + LC_SEND_START;
    if (EEPROM.read(a+LC_SEND_VALID)!=0x01) return;

    replaceSpaceWithNull(tval);
    EEPROM.get(a+LC_SEND_ROOM,room);
    EEPROM.get(a+LC_SEND_REGION,region);
    EEPROM.get(a+LC_SEND_ORDER,order);
    EEPROM.get(a+LC_SEND_PRIORITY,priority);
    EEPROM.get(a+LC_SEND_CCMTYPE,name);
    sprintf(strIP,"%d.%d.%d.%d",st_m302.ip[0],st_m302.ip[1],st_m302.ip[2],st_m302.ip[3]);
    sprintf(uecstext,xmlDT,name,room,region,order,priority+z,tval,strIP);
    Udp16520.beginPacket(broadcastIP,16520);
    Udp16520.write(uecstext);
    Udp16520.endPacket();
}

void UserEverySecond(void) {
  volatile bool aaa;
  char lval[12];
  char *xmlDT PROGMEM = CCMFMT;
  cndVal &= 0xfffffffe;            // Clear setup completed flag
  if (aaa) {
    digitalWrite(LED2,HIGH);
    aaa=false;
  } else {
    digitalWrite(LED2,LOW);
    aaa=true;
  }
  sprintf(lval,"%lu",cndVal);
  uecsSendData(0,xmlDT,lval,0);     // cnd
  wdt_reset();
}

void UserEvery10Seconds(void) {
  void ope_slt5006(void);
  ope_slt5006();
  wdt_reset();
}

void UserEveryMinute(void) {
  char *xmlDT PROGMEM = CCMFMT;
  wdt_reset();
}


///////////////////////////////////////////////////////////////////
// M302-lowcode for SLT5006
//  MIT License
//  Copyright (c) 2025 Masafumi Horimoto
//  Release on 
//  
///////////////////////////////////////////////////////////////////

const char VERSION[16] PROGMEM = "M302 V2.22";

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


#define  delayMillis 5000UL // 5sec
#define  LED2        3

char          uecsid[6], uecstext[180];
unsigned long cndVal;   // CCM cnd Value
char          val[16];
bool          useSerial = false;

//extern void lcdout(int,int,int);

/////////////////////////////////////
// Hardware Define
/////////////////////////////////////

stM302_t          st_m302;

IPAddress   broadcastIP,networkADDR;
EthernetUDP Udp16520,Udp16521,Udp16528,Udp16529;

// volatile bool period1sec =  false;
// volatile bool period10sec = false;
// volatile bool period60sec = false;
// volatile int  count10sec = 0;
// volatile int  count60sec = 0;

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
    
    wdt_reset();
    if (digitalRead(4)==HIGH) { // 通常運転
        useSerial = false;
        Serial.begin(9600);
        Serial.println(VERSION);
    } else {
        useSerial = true;
        Serial.begin(19200);
        Serial.println(VERSION);
        delay(50);
    }
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
            Serial.println(F("Set Static IP"));
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

    slt5006_setup();
    
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

unsigned short crc16(int size,byte* data) {
    unsigned short crc = 0xFFFF;
    int i,j;
    for (i=0;i<size;i++) {
        crc ^= data[i];
        for (j=0;j<8;j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

float sens_ana(int aport,int map_low,int map_high,float slope) {
    int sval,vol;
    float r;
    sval = analogRead(aport);
    vol  = map(sval,0,1023,map_low,map_high);
    r    = vol * slope;
    return r;
}

/////////////////////////////////
void loop() {
  static int k=0;
  int i,ia,ta,tb,cdsv;
  extern void recv16528port(void);
    
  recv16528port();
  wdt_reset();
    
  // unsigned long currentMillis = millis();
  // if (currentMillis - previousMillis >= interval) { // 1秒カウント
  //   previousMillis = currentMillis;
  //   period1sec = true; // 1秒ごとにフラグを立てる
  //   count10sec++;
  //   if (count10sec > 9) {
  //     period10sec = true;
  //     count10sec = 0;
  //   } else {
  //     period10sec = false;
  //   }
  //   count60sec++;
  //   if (count60sec > 59) {
  //     period60sec = true;
  //     count60sec = 0;
  //   } else {
  //     period60sec = false;
  //   }
  // }

  // // 10 sec interval
  // if (period10sec) {
  //   UserEvery10Seconds();
  //   period10sec=false;
  //   wdt_reset();
  // }
  // // 1 min interval
  // if (period60sec) {
  //       UserEveryMinute();
  //       period60sec = false;
  //       wdt_reset();
  //   }
  //   //1 sec interval
  //   if (period1sec) {
  //       period1sec = false;
  //       UserEverySecond();
  //   }
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
    byte room,region,priority;
    int  order,i,a;
    char name[20],dname[21],strIP[17];
    extern stM302_t st_m302;
    
    a = id * LC_SEND_REC_SIZE + LC_SEND_START;
    if (EEPROM.read(a+LC_SEND_VALID)!=0x01) return;
    
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
    volatile byte a=0 ;
    char val[7];
    char *xmlDT PROGMEM = CCMFMT;
    Serial.println("UserEverySecond");
    cndVal &= 0xfffffffe;            // Clear setup completed flag
    if (aaa) {
        digitalWrite(LED2,HIGH);
        aaa=false;
    } else {
        digitalWrite(LED2,LOW);
        aaa=true;
    }
    sprintf(val,"%u",cndVal);
    uecsSendData(0,xmlDT,val,0);     // cnd
    wdt_reset();
}

void UserEvery10Seconds(void) {
    char *xmlDT PROGMEM = CCMFMT;

    slt5006_loop();
    Serial.print(F("Temp="));
    Serial.println(sltdata.temp);
    dtostrf(sltdata.temp,-6,3,val);
    uecsSendData(1,xmlDT,val,0);
    dtostrf(sltdata.ec_bulk,-6,3,val);
    uecsSendData(2,xmlDT,val,0);
    dtostrf(sltdata.vwc_rock,-5,1,val);
    uecsSendData(3,xmlDT,val,0);
    dtostrf(sltdata.vwc,-5,1,val);
    uecsSendData(4,xmlDT,val,0);
    dtostrf(sltdata.vwc_coco,-5,1,val);
    uecsSendData(5,xmlDT,val,0);
    dtostrf(sltdata.ec_pore,-6,3,val);
    uecsSendData(6,xmlDT,val,0);
    wdt_reset();
}

void UserEveryMinute(void) {
    char *xmlDT PROGMEM = CCMFMT;
    Serial.println("UserEveryMinute");

    //  extern void lcdout(int,int,int);
    //  extern void getM252(int,bool);
    
    //  getM252(1,true);
    wdt_reset();
}


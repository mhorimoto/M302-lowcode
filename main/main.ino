///////////////////////////////////////////////////////////////////
// M302-lowcode for SHT4x
//  MIT License
//  Copyright (c) 2025 Masafumi Horimoto
//  Release on 
//  
///////////////////////////////////////////////////////////////////

const char VERSION[16] PROGMEM = "M302 V1.00";

#include "M302.h"
//#include "Adafruit_SHT4x.h"

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

//LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display
//char              lcdtext[2][17];
stM302_t          st_m302;

//Adafruit_SHT4x sht4 = Adafruit_SHT4x();

IPAddress   broadcastIP,networkADDR;
EthernetUDP Udp16520,Udp16521,Udp16528,Udp16529;

volatile int period1sec = 0;
volatile int period10sec = 0;
volatile int period60sec = 0;

//int packetSize;

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
    
    //  digitalWrite(9,LOW);
    //  delay(50);
    //  digitalWrite(9,HIGH);
    
    cndVal = 0L;    // Reset cnd value
    //  lcd.init();
    //  lcd.backlight();
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
    
    //  for(i=0;i<16;i++) {
    //    lcdtext[0][i] = pgm_read_byte(&(VERSION[i]));
    //  }
    wdt_reset();
    //  lcdtext[0][i] = 0;
    //  sprintf(lcdtext[1],"%d/%d/%d  %sL",
    //          EEPROM.read(LC_SEND_START+LC_SEND_ROOM),EEPROM.read(LC_SEND_START+LC_SEND_REGION),
    //	  EEPROM.read(LC_SEND_START+LC_SEND_ORDER),val);
    //  lcdout(0,1,1);
    if (digitalRead(4)==HIGH) { // 通常運転
        useSerial = false;
        Serial.begin(9600);
    } else {
        useSerial = true;
        Serial.begin(19200);
        Serial.println(VERSION);
        delay(50);
    }
    Ethernet.init(W5500SS);
    delay(500);
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
//    if (! sht4.begin()) {
//        Serial.println(F("NO SHT4x"));
//        while (1) {
//            uecsSendData(0,xmlDT,"67108865",0);     // NO SHT cnd
//            digitalWrite(LED2,HIGH);
//            delay(500);
//            digitalWrite(LED2,LOW);
//            delay(500);
//        }
//    }
//    sht4.setPrecision(SHT4X_HIGH_PRECISION);
//    sht4.setHeater(SHT4X_NO_HEATER);
    wdt_reset();
    uecsSendData(0,xmlDT,"395264",0);     // start cnd
    delay(100);

    byte data[] = {0x02,0x07,0x01,0x01,0x00,0x00};
    unsigned short crc = crc16(4,data);
    data[4] = (crc >> 8) & 0x00FF;
    data[5] = crc & 0x00FF;
    Serial.write(data,6);
    Udp16520.beginPacket(broadcastIP,16520);
    Udp16520.write(data,6);
    Udp16520.endPacket();
    delay(100);
    data[0] = 0x01;
    data[1] = 0x08;
    data[2] = 0x01;
    data[3] = 0x00;
    data[4] = 0xe6;
    Serial.write(data,5);
    Udp16520.beginPacket(broadcastIP,16520);
    Udp16520.write(data,5);
    Udp16520.endPacket();
    while(1) {
        if (Serial.available() >= 6) {
            for(i=0;i<6;i++) {
                data[i] = Serial.read();
            }
            Udp16520.beginPacket(broadcastIP,16520);
            Udp16520.write(data,6);
            Udp16520.endPacket();
        }
        delay(100);
    }
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
    int ia,l;
    char *xmlDT PROGMEM = CCMFMT;
    
    cndVal &= 0xfffffffe;            // Clear setup completed flag
    if (aaa) {
        digitalWrite(LED2,HIGH);
        aaa=false;
        //    lcd.setCursor(15,0);
        //    lcd.print(">");
    } else {
        digitalWrite(LED2,LOW);
        aaa=true;
        //    lcd.setCursor(15,0);
        //    lcd.print("<");
    }
    wdt_reset();
}

void UserEvery10Seconds(void) {
    char *xmlDT PROGMEM = CCMFMT;
    int i;
//    sensors_event_t ther,humi;
    //  extern void lcdout(int,int,int);
    //  extern void getM252(int,bool);
    
    //  getM252(1,false);
//    sht4.getEvent(&humi,&ther);
//    dtostrf(ther.temperature,-6,2,val);
//    for(i=0;i<7;i++) {
//        if (val[i]==' ') {
//            val[i] = 0;
//            break;
//        }
//    }
//    uecsSendData(1,xmlDT,val,0);   // cnd
//    dtostrf(humi.relative_humidity,-6,2,val);
//    for(i=0;i<7;i++) {
//        if (val[i]==' ') {
//            val[i] = 0;
//            break;
//        }
//    }
//    uecsSendData(2,xmlDT,val,0);     // cnd
//    sprintf(val,"%d",int(sht4.readSerial()));
//    uecsSendData(3,xmlDT,val,0);     // cnd
//    sprintf(val,"%d",int(sht4.readSerial()));
//    uecsSendData(3,xmlDT,val,0);     // cnd
    wdt_reset();
}

void UserEveryMinute(void) {
    char *xmlDT PROGMEM = CCMFMT;
    //  extern void lcdout(int,int,int);
    //  extern void getM252(int,bool);
    
    //  getM252(1,true);
    wdt_reset();
}


///////////////////////////////////////////////////////////////////
// M302-lowcode
//  MIT License
//  Copyright (c) 2026 Masafumi Horimoto
//  Release on 2026/04/17
///////////////////////////////////////////////////////////////////
//
//  This is a program for outdoor weather observation at T-House
//  in Miyazaki Prefecture.
//  The configuration is as follows:
//  - SHT-40 (Akizuki) for temperature and humidity observation
//  - Solar radiation measurement
//  - ROS2-U2JP (Sensiphia) for soil moisture measurement (SDI-12)
//  - CO2 measurement using an analog sensor
//  - PPFD measurement using an ADS1115 and a photodiode
//  The SHT-40 is connected via I2C.
//  The solar radiation sensor is connected via ADC.
//  The solar radiation sensor outputs an analog voltage between 0 and 1V,
//  representing solar radiation between 0 and 1kW/m^2.
//
//  The main board uses the M302N2.
//
//////////////////////////////////////////////////////////////////

//const char VERSION[16] PROGMEM = "M302N2 V3.10";
const char VERSION[16] PROGMEM = "M302N2 V4.10";

#include "M302.h"

#ifndef W5500SS
#define W5500SS SS
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

char          uecsid[6], uecstext[180];
unsigned long cndVal;   // CCM cnd Value
char          val[16];
bool          useSerial = false;

//extern void lcdout(int,int,int);

/////////////////////////////////////
// Hardware Define
/////////////////////////////////////

stM302_t          st_m302;

SensirionI2cSht4x sht4x;
Adafruit_ADS1115  ads;

IPAddress   broadcastIP,networkADDR;
EthernetUDP Udp16520,Udp16521,Udp16528,Udp16529;

volatile int period1sec = 0;
volatile int period10sec = 0;
volatile int period60sec = 0;

unsigned long previousMillis = 0; // 前回の時刻を記録
const unsigned long interval = 1000; // 1秒（1000ms）間隔

void ope_SHT4(int TempId, int HumidId);
void ope_Radiation(int ccmid);
void ope_CO2(int ccmid);
void ope_PPFD(int ccmid);
void ope_ROS2U2JP(int vwc0id, int vwc1id, int pdid, int tempid, int humidid);
void ros2_poll(void);
float sens_ana(int aport, int map_low, int map_high, float slope);
void truncate_at_first_space(int n, char v[]);
void configure_wdt(void);
void uecsSendData(int id, char *xmlDT, char *tval, int z);
void UserEverySecond(void);
void UserEvery10Seconds(void);
void UserEveryMinute(void);

void setup(void) {
    char *xmlDT PROGMEM = CCMFMT;
    int i,er;
    char version_info[17];

    //const char *ids PROGMEM = "%s:%02X%02X%02X%02X%02X%02X";
    extern void recv16528port(void);
    writeVersionToEEPROM();
    pinMode(LED1,OUTPUT);
    digitalWrite(LED1,LOW);
    pinMode(PORT_D2,INPUT_PULLUP);
    pinMode(PORT_D3,INPUT_PULLUP);
    pinMode(PORT_D4,INPUT_PULLUP);
    pinMode(PORT_D5,INPUT_PULLUP);
    pinMode(PORT_D6,INPUT_PULLUP);
    pinMode(PORT_D7,INPUT_PULLUP);
    pinMode(ADC_IN1,INPUT);
    pinMode(ADC_IN2,INPUT);
    Serial.begin(115200);  // for Debug
    Wire.begin();
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
    Ethernet.init(W5500SS);
    delay(300);
    wdt_reset();
    if (st_m302.dhcpflag) {
        er = Ethernet.begin(st_m302.mac);
        if (er==0) {
            st_m302.dhcpflag = false;
            while(1) {
                blinkLED(LED1,850,150,1);
            }
        }
    } else {
        Ethernet.begin(st_m302.mac,st_m302.set_ip,st_m302.dns,st_m302.gw,st_m302.subnet);
        er = 1;
    }
    if (er==0) {
        while (1) {
            blinkLED(LED1,1500,500,1);
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
    //sht4x.begin(Wire,SHT40_I2C_ADDR_44);
    //if ( sht4x.softReset() != 0 ) {
    //    while (1) {
    //        uecsSendData(0,xmlDT,"0x20000400",0);     // NO SHT cnd
    //        recv16528port();
    //        blinkLED(LED1,100,150,4);
    //    }
    // }
    //delay(10);
    //ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.0078125mV
    //delay(10);
    //if (!ads.begin()) {
    //    while(1) {
    //        uecsSendData(0,xmlDT,"0x20000B00",0);     // NO ADS1115 cnd
    //        recv16528port();
    //        blinkLED(LED1,50,200,4);
    //    }
    // }
    //delay(10);
    //
    uecsSendData(0,xmlDT,"0x60800",0);     // start cnd
    delay(100);    
    wdt_reset();
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
    vol  = map(constrain(sval,0,1023),0,1023,map_low,map_high);
    r    = vol * slope;
    return r;
}

/////////////////////////////////
void loop() {
    extern void recv16528port(void);
    extern void ros2_poll(void);
    
    recv16528port();
    ros2_poll();          // ROS2-U2JP 計測を非ブロッキングで進める
    wdt_reset();
    
    //1 sec interval
    if (period1sec==1) {
        period1sec = 0;
        UserEverySecond();
    }
    // 10 sec interval
    if (period10sec==1) {
        UserEvery10Seconds();
        period10sec=0;
    }
    // 1 min interval
    if (period60sec==1) {
        UserEveryMinute();
        period60sec = 0;
    }
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
    cli();                          // disable interrupts for changing the registers
    MCUSR = 0;                      // reset status register flags
                                    // Put timer in interrupt-only mode:
    WDTCSR |= 0b00011000;           // Set WDCE (5th from left) and WDE (4th from left) to enter config mode,
                                    // using bitwise OR assignment (leaves other bits unchanged).
    WDTCSR = 0b00001000 | 0b100001; // clr WDIE: interrupt enabled
                                    // set WDE: reset disabled
                                    // and set delay interval (right side of bar) to 8 seconds
    sei();                          // re-enable interrupts
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
    cndVal &= 0xfffffffe;            // Clear setup completed flag
    if (aaa) {
        digitalWrite(LED1,HIGH);
        aaa=false;
    } else {
        digitalWrite(LED1,LOW);
        aaa=true;
    }
    sprintf(val,"%lu",cndVal);
    uecsSendData(0,xmlDT,val,0);     // cnd
    wdt_reset();
}

void UserEvery10Seconds(void) {
//    void ope_SHT4(int,int);
//    void ope_Radiation(int);
//    void ope_CO2(int);
//    void ope_PPFD(int);
    void ope_ROS2U2JP(int,int,int,int,int);
    char *xmlDT PROGMEM = CCMFMT;
//    ope_SHT4(1,2);
//    delay(50);
//    ope_CO2(3);
//    delay(50);
    // 引数順: (vwc0id, vwc1id, pdid, tempid, humidid)  -1は送信しない
    ope_ROS2U2JP(6, 7, 8, 4, 9);   // VWC_0=6(VWC), VWC_1=7(VWC), PD=8(VWC_RAW), TEMP=4(SoilTemp), HUMID=9(Humid)
    delay(10);
//    ope_PPFD(10);
//    wdt_reset();
}

void UserEveryMinute(void) {
    char *xmlDT PROGMEM = CCMFMT;
    //  Serial.println("UserEveryMinute");
    wdt_reset();
}

void ope_SHT4(int TempId,int HumidId) {
    char *xmlDT PROGMEM = CCMFMT;
    float aTemperature = 0.0;
    float aHumidity = 0.0;
    sht4x.measureLowestPrecision(aTemperature, aHumidity);
    dtostrf(aTemperature,-6,2,val); 
    truncate_at_first_space(7,val) ;
    uecsSendData(TempId,xmlDT,val,0);   // Temp
    dtostrf(aHumidity,-6,2,val);
    truncate_at_first_space(7,val) ;
    uecsSendData(HumidId,xmlDT,val,0);   // Humid
    wdt_reset();
}

void ope_Radiation(int ccmid) {
    char *xmlDT PROGMEM = CCMFMT;
    float wrad;
    wrad = sens_ana(ADC_IN1,0,5000,1.2);
    sprintf(val,"%d",int(wrad));
    uecsSendData(ccmid,xmlDT,val,0);
    wdt_reset();
}

void ope_CO2(int ccmid) {
    char *xmlDT PROGMEM = CCMFMT;
    float co2;
    co2 = sens_ana(ADC_IN1,0,5000,0.6);
    sprintf(val,"%d",int(co2));
    uecsSendData(ccmid,xmlDT,val,0);   // co2
    wdt_reset();
}


void ope_PPFD(int ccmid) {
    char *xmlDT PROGMEM = CCMFMT;
    int16_t adc0;
    char tval[10];
    int   i_radiation;
    float radvolts,radiation;
    adc0 = ads.readADC_SingleEnded(0);
    radvolts = ads.computeVolts(adc0) * 1000.0;
    radiation= radvolts * 10.0;
    i_radiation = int(radiation);
    if (i_radiation < 0) {
        i_radiation = 0;
    }
    sprintf(tval,"%d",i_radiation);  // Radiation μmol/m^2*sec
    uecsSendData(ccmid,xmlDT,tval,0);
}
void truncate_at_first_space(int n,char v[]) {
    int i;
    for (i = 0; i < n; i++) {
        // 文字列の終端に達した場合も処理を終了
        if (v[i] == '\0') {
            break;
        }
        // スペースが見つかったら、それをヌル終端文字に置き換えてループを終了
        if (v[i] == ' ') {
            v[i] = '\0';
            break;
        }
    }
}
void blinkLED(int led,int on_time,int off_time,int n) {
    for (int i=0;i<n;i++) {
        digitalWrite(led,HIGH);
        delay(on_time);
        digitalWrite(led,LOW);
        delay(off_time);
    }
}

void writeVersionToEEPROM() {
    const int base = VERSION_INFO;  // 開始アドレス
    // 最大15文字 + 終端(0) を想定（VERSION[16]）
    for (int i = 0; i < 16; i++) {
        uint8_t c = pgm_read_byte(&VERSION[i]);      // PROGMEM から1バイト読む
        EEPROM.update(base + i, c);                  // 変更があるときだけ書く
        if (c == 0) break;                           // 終端に達したら終了
    }
}

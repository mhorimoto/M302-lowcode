#include "M302.h"
#if _M302_H_V < 100
#pragma message("Library M302 is old. Version 1.00 or higher is required.")
#else

#define INPUT_LINE_SIZE  30

char *pgname = "M302 mklc V1.10 ";
char inputbuf[INPUT_LINE_SIZE],*ptr_inputbuf;
int  cnt;

const int BUFFER_SIZE = 64;
char inputBuffer[BUFFER_SIZE];
int bufferIndex = 0;


//LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

void setup(void) {
    String linebuf,inputbuf;
    
    Serial.begin(19200);
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
    Serial.print(F("# "));
}


void loop(void) {
    while (Serial.available() > 0) {
        char ch = Serial.read();
        if (ch == '\b' || ch == 0x7F) {
            if (bufferIndex > 0) {
                bufferIndex--;
                Serial.print("\b \b");
            }
            continue;
        }
        Serial.print(ch);
        if (ch == '\n' || ch == '\r') {
            Serial.println();
            if (bufferIndex > 0) {
                inputBuffer[bufferIndex] = '\0';
                parseCommand(inputBuffer);
                bufferIndex = 0;
            }
            Serial.print(F("# "));
        } else if (bufferIndex < BUFFER_SIZE - 1) {
            inputBuffer[bufferIndex++] = ch;
        } else {
            Serial.println(F("\nInput buffer overflow!"));
        }
    }
}

void parseCommand(char* input) {
    extern void listCommand(void);
    extern void setCommand(char *,int,int);
    extern void clearpage(char *);
    extern void help(void);
    
    // トークン分解
    char* token = strtok(input, " ");
    if (token == NULL) return;
    
    // コマンド名
    if (strcmp(token, "set") == 0) {
        handleSetCommand();
    } else if (strcmp(token, "get") == 0) {
        handleGetCommand();
    } else if (strcmp(token, "list") == 0) {
        listCommand();
    } else if (strcmp(token, "clearpage") == 0) {
    //    clearpage(page);
    } else if (strcmp(token, "help") == 0) {
        help();
    } else {
        Serial.print("Unknown command: ");
        Serial.println(token);
    }
}

void handleSetCommand() {
    extern void setCommand(char *,int,int);
    extern void setCCMCommand(char *,int,int,int);
    extern void help(void);
    char* target = strtok(NULL, " ");  // 例: "led"
    char* value  = strtok(NULL, " ");  // 例: "on"

    if ( !strcmp(target,"uecsid") ) {
        setCommand(value,12,LC_UECS_ID);
    } else if ( !strcmp(target,"mac") ) {
        setCommand(value,12,LC_MAC);
    } else if ( !strcmp(target,"dhcp") ) {
        setCommand(value,1,FIX_DHCP_FLAG);
    } else if ( !strcmp(target,"ip") ) {
        setCommand(value,15,FIXED_IPADDRESS);
    } else if ( !strcmp(target,"netmask") ) {
        setCommand(value,15,FIXED_NETMASK);
    } else if ( !strcmp(target ,"gw") ) {
        setCommand(value,15,FIXED_DEFGW);
    } else if ( !strcmp(target,"dns") ) {
        setCommand(value,15,FIXED_DNS);
    } else if ( !strcmp(target,"vender") ) {
        setCommand(value,16,VENDER_NAME);
    } else if ( !strcmp(target,"nodename") ) {
        setCommand(value,16,NODE_NAME);
    } else if ( !strcmp(target,"ccm") ) {
        int ccmid = atoi(value);  // set ccm ccmid ccmsubcom 
        char *ccmsubcom = strtok(NULL, " ");
        char *ccmValue = strtok(NULL, " ");
        if ( !strcmp(ccmsubcom,"valid")) {
            setCCMCommand(ccmValue,2,LC_SEND_VALID,ccmid); // value = ccmid
        } else if ( !strcmp(ccmsubcom,"room")) {
            setCCMCommand(ccmValue,2,LC_SEND_ROOM,ccmid);
        } else if ( !strcmp(ccmsubcom,"region")) {
            setCCMCommand(ccmValue,2,LC_SEND_REGION,ccmid);
        } else if ( !strcmp(ccmsubcom,"order")) {
            setCCMCommand(ccmValue,4,LC_SEND_ORDER,ccmid);
        } else if ( !strcmp(ccmsubcom,"priority")) {
            setCCMCommand(ccmValue,2,LC_SEND_PRIORITY,ccmid);
        } else if ( !strcmp(ccmsubcom,"lv")) {
            setCCMCommand(ccmValue,2,LC_SEND_LV,ccmid);
        } else if ( !strcmp(ccmsubcom,"cast")) {
            setCCMCommand(ccmValue,2,LC_SEND_CAST,ccmid);
        } else if ( !strcmp(ccmsubcom,"ccmtype")) {
            setCCMCommand(ccmValue,20,LC_SEND_CCMTYPE,ccmid);
        } else if ( !strcmp(ccmsubcom,"cast")) {
            setCCMCommand(ccmValue,20,LC_SEND_CAST,ccmid);
        } else if ( !strcmp(ccmsubcom,"unit")) {
            setCCMCommand(ccmValue,12,LC_SEND_UNIT,ccmid);
        } else if ( !strcmp(ccmsubcom,"func")) {
            setCCMCommand(ccmValue,2,LC_SEND_FUNC,ccmid);
        } else if ( !strcmp(ccmsubcom,"param")) {
            setCCMCommand(ccmValue,12,LC_SEND_PARAM,ccmid);
        }
    } else if ( !strcmp(target,"help") ) {
        help();
    } else {
        Serial.print("Unknown target: ");
        Serial.println(target);
        Serial.println("Usage: set <target> <value>");
    }
}

void handleGetCommand() {
    char* target = strtok(NULL, " ");  // 例: "temp"
    
    if (target) {
        Serial.print("Getting value of ");
        Serial.println(target);
        // 例: センサー読み取りや状態報告
    } else {
        Serial.println("Usage: get <target>");
    }
}



#endif

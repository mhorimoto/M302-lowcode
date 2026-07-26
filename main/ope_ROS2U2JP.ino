///////////////////////////////////////////////////////////////////
// ope_ROS2U2JP.ino
//  ROS2-U2JP 土壌水分センサー SDI-12 計測モジュール（非ブロッキング版）
//  M302-lowcode ULTRA ブランチへの統合用
//
//  接続:
//    SDI-12 データ線 → D6（M302.h の D6 = PORT_D6）
//    SDI-12 電源     → 5V〜16V DC
//    SDI-12 GND      → GND
//
//  ★ M2! コマンド使用（実機ファーム v1.1 §3.4 準拠）:
//    aD0! 応答（5値）: a<PD><VWC_0><VWC_1><TEMP><HUMIDITY><CR><LF>
//    実機生電文の例: 1+146.8+0.0+0.0+30.4+54.6
//      [0] PD       : 伝播遅延        [ps]
//      [1] VWC_0    : 体積含水率(汎用)  [%]
//      [2] VWC_1    : 体積含水率(土壌固有)[%]
//      [3] TEMP     : 土壌温度        [℃]
//      [4] HUMIDITY : プローブ内湿度   [%RH]
//    → 5値すべてを個別の UECS ID へ送信する。
//
//  UECS送信ID（引数の順に対応。不要なら -1 で送信スキップ）:
//    ope_ROS2U2JP(vwc0id, vwc1id, pdid, tempid, humidid)
//
//  ★非ブロッキング動作:
//    ステートマシンにより、計測待ちの約3秒間も loop() が回り続ける。
//    使い方:
//      - UserEvery10Seconds() から ope_ROS2U2JP(...) を呼ぶ
//        → 計測を「開始」するだけ。すぐ戻る。
//      - loop() から毎回 ros2_poll() を呼ぶ
//        → 計測の続きを少しずつ進める。
//
//  割り込みについて:
//    SoftwareSerial(SLT5006)を使わない構成のため、SDI-12ライブラリが
//    自前でPCINT ISRを定義する通常モードで動作する。
///////////////////////////////////////////////////////////////////

// SDI12.h は M302.h でインクルード済み

// SDI-12 データピン（M302.h PORT_D6 = 6）
#define ROS2_SDI12_PIN   PORT_D6

// センサアドレス（センサのアドレスに合わせる）
#define ROS2_ADDR        '1'

// 各種タイムアウト [ms]
#define ROS2_ATTTN_TIMEOUT 1500   // Mコマンドの atttn 応答待ち
#define ROS2_D0_TIMEOUT    3000   // aD0! 応答待ち
#define ROS2_SR_GUARD_MS   100    // サービスリクエスト後の半二重切替待ち

// 計測コマンド。M2! を使用（PD/VWC_0/VWC_1/TEMP/HUMIDITY の5値）
#define ROS2_MEAS_CMD    "M2"

// 応答に含まれる値の数（M2! は5値）
#define ROS2_MAX_VALS    5

// 値の並び順インデックス（v1.1 §3.4  M2!: PD,VWC_0,VWC_1,TEMP,HUMIDITY）
#define ROS2_IDX_PD    0   // 伝播遅延
#define ROS2_IDX_VWC0  1   // 体積含水率（汎用キャリブレーション）
#define ROS2_IDX_VWC1  2   // 体積含水率（土壌固有キャリブレーション）
#define ROS2_IDX_TEMP  3   // 土壌温度
#define ROS2_IDX_HUMID 4   // プローブ内湿度

// SDI-12 インスタンス
static SDI12 ros2sdi12(ROS2_SDI12_PIN);
static bool  ros2_initialized = false;

// ---- ステートマシン定義 -------------------------------------------------
enum ros2State {
    ROS2_IDLE = 0,   // 待機
    ROS2_WAIT_SR,    // サービスリクエスト待ち
    ROS2_GUARD,      // SR受信後の半二重切替待ち
    ROS2_SEND_D0,    // aD0! 送信
    ROS2_READ_D0     // aD0! 応答受信中
};

static ros2State ros2_state    = ROS2_IDLE;
// 5値それぞれの UECS送信ID。ros2_id[ROS2_IDX_xxx] で参照。-1 は送信しない。
static int       ros2_id[ROS2_MAX_VALS] = { -1, -1, -1, -1, -1 };
static uint8_t   ros2_nVals    = 0;      // atttn で返ってきた値数
static uint32_t  ros2_deadline = 0;      // 現ステートの締切時刻
static uint32_t  ros2_guardUntil = 0;    // GUARD の終了時刻

// 受信用バッファ
static char     ros2_rxBuf[64];
static uint8_t  ros2_rxLen = 0;
static uint32_t ros2_rxLastChar = 0;

// -----------------------------------------------------------------------
// 内部: 非ブロッキングで来ている分だけ処理して行を組み立てる
// -----------------------------------------------------------------------
static bool ros2_pollLine(char* line, uint8_t lineSize) {
    while (ros2sdi12.available()) {
        char c = ros2sdi12.read();
        ros2_rxLastChar = millis();
        if (c == '\r' || c == '\n') {
            if (ros2_rxLen > 0) {
                uint8_t n = ros2_rxLen < lineSize - 1 ? ros2_rxLen : lineSize - 1;
                memcpy(line, ros2_rxBuf, n);
                line[n] = '\0';
                ros2_rxLen = 0;
                return true;
            }
        } else if (c >= 32 && c <= 126) {
            if (ros2_rxLen < sizeof(ros2_rxBuf) - 1) ros2_rxBuf[ros2_rxLen++] = c;
        }
    }
    if (ros2_rxLen > 0 && (millis() - ros2_rxLastChar) > 150) {
        uint8_t n = ros2_rxLen < lineSize - 1 ? ros2_rxLen : lineSize - 1;
        memcpy(line, ros2_rxBuf, n);
        line[n] = '\0';
        ros2_rxLen = 0;
        return true;
    }
    return false;
}

// -----------------------------------------------------------------------
// 内部: 1つの計測値を UECS 送信するヘルパ
//   id < 0 なら送信しない。
//   valid=false またはエラー特殊値(-100/-200)なら小数1桁でそのまま送る。
// -----------------------------------------------------------------------
static void ros2_sendOne(int id, float v, bool valid) {
    if (id < 0) return;
    extern char val[];
    extern void uecsSendData(int, char*, char*, int);
    extern void truncate_at_first_space(int, char[]);
    char *xmlDT PROGMEM = CCMFMT;

    dtostrf(v, -6, (valid && v > -100.0f) ? 2 : 1, val);
    truncate_at_first_space(7, val);
    uecsSendData(id, xmlDT, val, 0);
}

// -----------------------------------------------------------------------
// 内部: 生電文をそのまま UECS 送信する（デバッグ用・通常は未使用）
// -----------------------------------------------------------------------
static void ros2_sendRaw(int id, const char* raw) {
    if (id < 0) return;
    extern void uecsSendData(int, char*, char*, int);
    char *xmlDT PROGMEM = CCMFMT;
    static char rawbuf[48];
    if (raw == 0 || raw[0] == '\0') {
        strcpy(rawbuf, "NORESP");
    } else {
        uint8_t n = strlen(raw);
        if (n >= sizeof(rawbuf)) n = sizeof(rawbuf) - 1;
        memcpy(rawbuf, raw, n);
        rawbuf[n] = '\0';
    }
    uecsSendData(id, xmlDT, rawbuf, 0);
}

// -----------------------------------------------------------------------
// 内部: aD0! 応答を解析して 5値すべてを UECS 送信する
//   dResp が空文字列("")の場合は全値エラー(-200)として送信
// -----------------------------------------------------------------------
static void ros2_parseAndSend(const char* dResp) {
    // M2! 応答: [0]=PD, [1]=VWC_0, [2]=VWC_1, [3]=TEMP, [4]=HUMIDITY
    float vals[ROS2_MAX_VALS];
    for (uint8_t i = 0; i < ROS2_MAX_VALS; i++) vals[i] = -200.0f;
    uint8_t got = 0;

    if (strlen(dResp) >= 2 && dResp[0] == ROS2_ADDR) {
        const char* p = dResp + 1;
        while (*p != '\0' && got < ros2_nVals && got < ROS2_MAX_VALS) {
            if (*p == '+' || *p == '-') {
                const char* start = p++;
                while (*p != '\0' && *p != '+' && *p != '-') p++;
                char tmp[16];
                uint8_t tlen = (uint8_t)(p - start);
                if (tlen >= sizeof(tmp)) tlen = sizeof(tmp) - 1;
                memcpy(tmp, start, tlen);
                tmp[tlen] = '\0';
                vals[got++] = (float)atof(tmp);
            } else {
                p++;
            }
        }
    }

    // 5値それぞれを対応する UECS ID へ送信（id=-1 はスキップ）
    for (uint8_t i = 0; i < ROS2_MAX_VALS; i++) {
        ros2_sendOne(ros2_id[i], vals[i], got > i);
    }

    // --- デバッグ: 生電文を見たい場合は、任意の枠を使って次を呼ぶ ---
    // ros2_sendRaw(ros2_id[ROS2_IDX_PD], dResp);
    (void)ros2_sendRaw;   // 未使用警告の抑止
}

// -----------------------------------------------------------------------
// 公開関数: ope_ROS2U2JP(vwc0id, vwc1id, pdid, tempid, humidid)
//   計測を「開始」するだけ。実際の受信・UECS送信は ros2_poll() が進める。
//   各引数に UECS の CCM id を渡す。送信したくない値は -1 を渡す。
//   前回の計測がまだ進行中なら何もせず戻る。
// -----------------------------------------------------------------------
void ope_ROS2U2JP(int vwc0id, int vwc1id, int pdid, int tempid, int humidid) {
    if (!ros2_initialized) {
        ros2sdi12.begin();
        delay(500);
        ros2sdi12.forceHold();
        ros2_initialized = true;
    }

    if (ros2_state != ROS2_IDLE) return;

    // 引数を応答の並び順（PD,VWC_0,VWC_1,TEMP,HUMIDITY）に対応させて格納
    ros2_id[ROS2_IDX_PD]    = pdid;
    ros2_id[ROS2_IDX_VWC0]  = vwc0id;
    ros2_id[ROS2_IDX_VWC1]  = vwc1id;
    ros2_id[ROS2_IDX_TEMP]  = tempid;
    ros2_id[ROS2_IDX_HUMID] = humidid;

    // --- Mコマンド送信 ---
    char buf[8];
    snprintf(buf, sizeof(buf), "%c%s!", ROS2_ADDR, ROS2_MEAS_CMD);
    while (ros2sdi12.available()) ros2sdi12.read();
    ros2sdi12.clearBuffer();
    ros2_rxLen = 0;
    ros2sdi12.sendCommand(buf);

    // atttn 応答（数十msで返る）だけは短時間待つ
    char resp[16];
    uint32_t atttnDeadline = millis() + ROS2_ATTTN_TIMEOUT;
    bool gotAtttn = false;
    while (millis() < atttnDeadline) {
        if (ros2_pollLine(resp, sizeof(resp))) { gotAtttn = true; break; }
        wdt_reset();
    }

    if (!gotAtttn || strlen(resp) < 5 || resp[0] != ROS2_ADDR) {
        ros2_nVals = 0;
        ros2_parseAndSend("");
        ros2_state = ROS2_IDLE;
        return;
    }

    // atttn 解析（例: 10035 → ttt=003秒, n=5値）
    char tttBuf[4] = { resp[1], resp[2], resp[3], '\0' };
    uint32_t waitSec = (uint32_t)atoi(tttBuf);
    ros2_nVals = resp[4] - '0';

    if (ros2_nVals == 0) {
        ros2_parseAndSend("");
        ros2_state = ROS2_IDLE;
        return;
    }

    ros2_deadline = millis() + (waitSec + 1) * 1000UL;
    ros2_rxLen = 0;
    ros2_state = ROS2_WAIT_SR;
}

// -----------------------------------------------------------------------
// 公開関数: ros2_poll()
//   loop() から毎回呼ぶ。計測の続きを非ブロッキングで進める。
// -----------------------------------------------------------------------
void ros2_poll(void) {
    if (ros2_state == ROS2_IDLE) return;

    char line[64];

    switch (ros2_state) {

    case ROS2_WAIT_SR:
        if (ros2_pollLine(line, sizeof(line))) {
            if (strlen(line) == 1 && line[0] == ROS2_ADDR) {
                ros2_guardUntil = millis() + ROS2_SR_GUARD_MS;
                ros2_state = ROS2_GUARD;
                return;
            }
        }
        if ((int32_t)(millis() - ros2_deadline) >= 0) {
            ros2_guardUntil = millis();
            ros2_state = ROS2_GUARD;
        }
        return;

    case ROS2_GUARD:
        if ((int32_t)(millis() - ros2_guardUntil) >= 0) {
            ros2_state = ROS2_SEND_D0;
        }
        return;

    case ROS2_SEND_D0: {
        char buf[8];
        snprintf(buf, sizeof(buf), "%cD0!", ROS2_ADDR);
        while (ros2sdi12.available()) ros2sdi12.read();
        ros2sdi12.clearBuffer();
        ros2_rxLen = 0;
        ros2sdi12.sendCommand(buf);
        ros2_deadline = millis() + ROS2_D0_TIMEOUT;
        ros2_state = ROS2_READ_D0;
        return;
    }

    case ROS2_READ_D0:
        if (ros2_pollLine(line, sizeof(line))) {
            ros2_parseAndSend(line);
            ros2_state = ROS2_IDLE;
            return;
        }
        if ((int32_t)(millis() - ros2_deadline) >= 0) {
            ros2_parseAndSend("");
            ros2_state = ROS2_IDLE;
        }
        return;

    default:
        ros2_state = ROS2_IDLE;
        return;
    }
}

/* irfake_v2r: Fake IR remote controll from Viera to Regza
 *  Send Regza TV command from IR
 *   when received Viera TV command with DIGA BD controller
 *  refs:
 *    https://qiita.com/dokkozo/items/25c6e17fcc2e655d5d42
 *    IRsendDemo on https://github.com/z3t0/Arduino-IRremote
 *    https://gist.github.com/sukedai/1759605
 *  Version 1.0 by pado3 2020/09/18
 *  Version 2.0 by pado3 2020/11/15 change INT timing fm CHANGE to FALLING
 *  Version 2.1 by pado3 2020/11/16 add powerdown mode
 */

#include <stdio.h>
#include <IRremote.h>   // 送信のみで使用
#include <avr/sleep.h>  // v2.1でパワーダウン対応
#define maxLen 200      // 信号の最大長, Vieraは50なので長ければ捨てる
#define IR_VCC 10       // IR受信モジュールのVCC 47Ω 10uFのLPFを通す
#define IR_GND 9        // IR受信モジュールのGND
// IR_RECEIVE_PIN 2     // attachInterrupt(0, ...)を使うのでD2使用
// IR_SEND_PIN 3        // どこで定義されているか不明だがIRremote制約
//                         D3から47Ωを通して赤外線LEDを光らせる
// 次の2つはISR割り込みルーチンで変化するためレジスタでなくRAMからロード
volatile unsigned int irBuffer[maxLen]; // IRを受信した時間
volatile unsigned int x = 0; // 信号の長さ Pointer thru irBuffer
int splash = 0;
unsigned int prevx = 0; // 前回ループ時のx値、これが増えていなければゴミ
unsigned long last_ms;  // 最後に割込があった時点のmillis() 
IRsend irsend;

// 関数の型定義
void setup(void);
void loop(void);
unsigned long v2r(unsigned long);
unsigned long recvViera();
unsigned long Str2Hex(const char*);
void powerdown(void);
void rxIR_Interrupt_Handler(void);

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(IR_GND, OUTPUT);
  pinMode(IR_VCC, OUTPUT);
  digitalWrite(IR_GND, LOW);
  Serial.begin(115200);
  delay(200);
  digitalWrite(IR_VCC, HIGH);
  Serial.println();
  // Just to know which program is running on my Arduino
  Serial.println(F("START " __FILE__ " from " __DATE__));
  // Receiver setup
  Serial.println(F("Please send me Panasonic remocon with PIN2"));
  //  attachInterrupt(0, rxIR_Interrupt_Handler, CHANGE); // v2.0:BOTH=>FALL for stability
  attachInterrupt(0, rxIR_Interrupt_Handler, FALLING);
  // Transmitter setup
  Serial.print(F("Ready to send IR signals at pin "));
  Serial.println(IR_SEND_PIN);
}

void loop() {
  unsigned long recvdata, senddata;
  // splash
  while(splash < 5) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    splash++;
  }
  recvdata = recvViera();
  if (recvdata) {
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.print(F("Viera received: 0x"));
    Serial.print(recvdata, HEX);
    Serial.print(F(", command: "));
    senddata = v2r(recvdata);
    if (senddata) {
      irsend.sendNEC(senddata, 32); // RegzaはNECフォーマット
      Serial.print(F("Send: 0x"));
      Serial.print(senddata, HEX);
    } else {
      Serial.print(F("Send data: PASS"));
    }
    digitalWrite(LED_BUILTIN, LOW);
    last_ms = millis();
    Serial.println(F(", wait next"));
  }
  if (10000 < (millis() - last_ms)) { // 10秒以上データが来なかったらパワーダウン
    Serial.println(F("Zzz.."));
    delay(10);
    powerdown();
    Serial.println(F("wake up!"));
  }
}

unsigned long v2r(unsigned long viera) {
  switch(viera) {
    case 0x3DBD:  // 電源
      Serial.print(F("power, "));
      return 0x2FD48B7;
    case 0x585:  // 入力切換
      Serial.print(F("chg, "));
      return 0x2FDF00F;
    case 0x39B9:  // 画面表示
      Serial.print(F("disp, "));
      return 0x2FD38C7;
    case 0x99B12:  // ｄ
      Serial.print(F("data, "));
      return 0xC23D28D7;
    case 0x32B2:  // MUTE
      Serial.print(F("mute, "));
      return 0x2FD08F7;
    case 0x34B4:  // CHUP
      Serial.print(F("chup, "));
      return 0x2FDD827;
    case 0x35B5: // CHDOWN
      Serial.print(F("chdn, "));
      return 0x2FDF807;
    case 0x20A0: // 音量UP
      Serial.print(F("volup, "));
      return 0x2FD58A7;
    case 0x21A1:  // 音量DOWN
      Serial.print(F("voldn, "));
      return 0x2FD7887;
    case 0x274F6:  // 地上
      Serial.print(F("dttv, "));
      return 0x2FD5EA1;
    case 0x270F2:  // BS
      Serial.print(F("bs, "));
      return 0x2FD3EC1;
    case 0x275F7:  // CS
      Serial.print(F("cs, "));
      return 0x2FDBE41;
    case 0x9F27B:  // NETFLIX => YOUTUBE
      Serial.print(F("youtube, "));
      return 0x2FDF40B;
    // DTTV Channel
    case 0x940C9:  // 1
      Serial.print(F("D1, "));
      return 0x2FD807F;
    case 0x941C8:  // 2
      Serial.print(F("D2, "));
      return 0x2FD40BF;
    case 0x942CB:  // 3
      Serial.print(F("D3, "));
      return 0x2FDC03F;
    case 0x943CA:  // 4
      Serial.print(F("D4, "));
      return 0x2FD20DF;
    case 0x944CD:  // 5
      Serial.print(F("D5, "));
      return 0x2FDA05F;
    case 0x945CC:  // 6
      Serial.print(F("D6, "));
      return 0x2FD609F;
    case 0x946CF:  // 7
      Serial.print(F("D7, "));
      return 0x2FDE01F;
    case 0x947CE:  // 8
      Serial.print(F("D8, "));
      return 0x2FD10EF;
    case 0x948C1:  // 9
      Serial.print(F("D9, "));
      return 0x2FD906F;
    case 0x949C0:  // 10
      Serial.print(F("D10, "));
      return 0x2FD50AF;
    case 0x94AC3:  // 11
      Serial.print(F("D11, "));
      return 0x2FDD02F;
    case 0x94BC2:  // 12
      Serial.print(F("D12, "));
      return 0x2FD30CF;
    // BS Channel
    case 0x950D9:  // 1
      Serial.print(F("B1, "));
      return 0x2FD807F;
    case 0x951D8:  // 2
      Serial.print(F("B2, "));
      return 0x2FD40BF;
    case 0x952DB:  // 3
      Serial.print(F("B3, "));
      return 0x2FDC03F;
    case 0x953DA:  // 4
      Serial.print(F("B4, "));
      return 0x2FD20DF;
    case 0x954DD:  // 5
      Serial.print(F("B5, "));
      return 0x2FDA05F;
    case 0x955DC:  // 6
      Serial.print(F("B6, "));
      return 0x2FD609F;
    case 0x956DF:  // 7
      Serial.print(F("B7, "));
      return 0x2FDE01F;
    case 0x957DE:  // 8
      Serial.print(F("B8, "));
      return 0x2FD10EF;
    case 0x958D1:  // 9
      Serial.print(F("B9, "));
      return 0x2FD906F;
    case 0x959D0:  // 10
      Serial.print(F("B10, "));
      return 0x2FD50AF;
    case 0x95AD3:  // 11
      Serial.print(F("B11, "));
      return 0x2FDD02F;
    case 0x95BD2:  // 12
      Serial.print(F("B12, "));
      return 0x2FD30CF;
    // CS Channel
    case 0x960E9:  // 1
      Serial.print(F("C1, "));
      return 0x2FD807F;
    case 0x961E8:  // 2
      Serial.print(F("C2, "));
      return 0x2FD40BF;
    case 0x962EB:  // 3
      Serial.print(F("C3, "));
      return 0x2FDC03F;
    case 0x963EA:  // 4
      Serial.print(F("C4, "));
      return 0x2FD20DF;
    case 0x964ED:  // 5
      Serial.print(F("C5, "));
      return 0x2FDA05F;
    case 0x965EC:  // 6
      Serial.print(F("C6, "));
      return 0x2FD609F;
    case 0x966EF:  // 7
      Serial.print(F("C7, "));
      return 0x2FDE01F;
    case 0x967EE:  // 8
      Serial.print(F("C8, "));
      return 0x2FD10EF;
    case 0x968E1:  // 9
      Serial.print(F("C9, "));
      return 0x2FD906F;
    case 0x969E0:  // 10
      Serial.print(F("C10, "));
      return 0x2FD50AF;
    case 0x96AE3:  // 11
      Serial.print(F("C11, "));
      return 0x2FDD02F;
    case 0x96BE2:  // 12
      Serial.print(F("C12, "));
      return 0x2FD30CF;
    case 0x4ECE:  // 左
      Serial.print(F("left, "));
      return 0x2FDFA05;
    case 0x4ACA:  // 上
      Serial.print(F("up, "));
      return 0x2FD7C83;
    case 0x4FCF:  // 右
      Serial.print(F("right, "));
      return 0x2FDDA25;
    case 0x4BCB:  // 下
      Serial.print(F("down, "));
      return 0x2FDFC03;
    case 0x49C9:  // 決定
      Serial.print(F("enter, "));
      return 0x2FDBC43;
    case 0xD454:  // 戻る
      Serial.print(F("return, "));
      return 0x2FDDC23;
    case 0x9A72E:  //  サブメニュー
      Serial.print(F("sub, "));
      return 0x2FDE41B;
    case 0x9C841:  // 録画一覧
      Serial.print(F("recorded, "));
      return 0x27D14EB;
    case 0x9951C:  // ホーム => 終了
      Serial.print(F("end, "));
      return 0x2FD3CC3;
    case 0x140C1:  // 番組表
      Serial.print(F("timetable, "));
      return 0x2FD7689;
    case 0x73F3:  // 青
      Serial.print(F("blue, "));
      return 0x2FDCE31;
    case 0x70F0:  // 赤
      Serial.print(F("red, "));
      return 0x2FD2ED1;
    case 0x71F1:  // 緑
      Serial.print(F("green, "));
      return 0x2FDAE51;
    case 0x72F2:  // 黄色
      Serial.print(F("yellow, "));
      return 0x2FD6E91;
    default:  // 空振り
      Serial.print(F("UNKNOWN, "));
      return 0x0;
//    case 0x:  // 予備
//      Serial.print(F(", "));
//      return 0x;
  }
}

unsigned long recvViera() {
  String recvs = "";        // ゴミが紛れ込むことがあるので、まずStringで受けて、
  unsigned long recvL = 0;  // データを確認したら数値に変換して返す
  unsigned int llen;        // リーダ部の長さ
  int i, cnt, hexval;       // ループカウンタ, デコードカウンタ, hex値の元
  delay(130); // バッファに家電協の最大信号長130msは溜めてから処理
  if (0 < x) {  // データがあれば、先頭がリーダ部か確認する
    llen = irBuffer[1] - irBuffer[0];
    // 家電協の最小リーダ長4200usを下回っているときはゴミ。このあたりは使うリモコンの規格次第
    // cf. http://elm-chan.org/docs/ir_format.html
    if (llen < 4000) {
      Serial.println(F("1st data is too short, clear buffer"));
      x = 0;
    } else {
      /* DEBUG
      Serial.print(F("leader length: "));
      Serial.print(llen);
      Serial.println(F("us"));
      */
      if (x == prevx) { // 前のループからデータが増えていなければ、ゴミとみなして消す。これ有効
        Serial.println(F("buffer is not incremented, clear buffer"));
        x = 0;
      } else {
        prevx = x;
      }
    }
  }
  if (50 <= x ) { // 50以上バッファに溜まればデコードする(Vieraの有効信号はL+dat48+Tで50)
    Serial.print(F("count: "));
    Serial.print(x);
    Serial.print(F(", "));
    // detachInterrupt(0); // いったん割り込みを止める // v2.0でごみ処理できたので止めない
    // 絶対時間を相対時間すなわちパルス長に変換
    // for (int i = 1; i < x; i++) {  // 以前は全データをデコードしていた
    for (i = 1; i <= 50; i++) { // Vieraなら50組だけ読めばいい。後ろにゴミがついても無視
      irBuffer[i-1] = irBuffer[i] - irBuffer[i-1];
    }
    // ここからLSBファーストのbit-byte変換。念のためのパラメータ初期化
    cnt = 0;    // digit
    hexval = 0; // sum of 8digits C0 C1 ... C7 (LSB first)
    for (i = 1; i <= 48; i++) { // Panasonic Viera 48bit
      if (1200 < irBuffer[i]){  // 2Tと4Tの閾値、2T:max1000us 4T:min1400us
        hexval += 1 << cnt;
      }
      cnt++;
      if (cnt == 8) { // 8bitまとまったらHEXの文字列に変換する
        if(hexval < 16) { // padding
          recvs += "0";
        }
        recvs += String(hexval, HEX);
        cnt = 0;
        hexval = 0;
      }
    }
    if (!recvs.startsWith("022080")) { // Vieraはヘッダが0x022080でなければおかしい
      Serial.print(recvs);
      Serial.println(F(" is not Viera data, return NULL"));
      recvs = "";
      recvL = 0;
    } else {
      // Vieraのヘッダ 0x022080 を切り詰める
      recvs = recvs.substring(6);
      // 数値のHEXに変換
      recvL = Str2Hex(recvs.c_str());
    }
    delay(200); // リピート対策、これがないとsendでコケる
    x = 0;  // デコード完了したところで初期化
    // attachInterrupt(0, rxIR_Interrupt_Handler, FALLING); // 割込は止めない
  }
  return recvL;
}

unsigned long Str2Hex(const char* str)
{
     return strtol(str, 0, 16); // strtoulだと最上位ビットが反転してしまった
}

void powerdown(void) {
  // IR受信modの消費電流はtyp.300uA max.600uA
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  // ADC停止、使うときは0x80（たぶんpinModeで設定）だが、このスケッチでは使わない
  ADCSRA = 0x00; 
  // 低電圧検出器(BOD:brown-out detector)停止
  // MCUCRのBODSとBODSEに1をセットし、4クロック内にBODSを1, BODSSEを0に設定
  MCUCR |= (1 << BODSE) | (1 << BODS);
  MCUCR = (MCUCR & ~(1 << BODSE)) | (1 << BODS);
  sleep_mode(); // sleep_(enable + cpu + disable), 割り込みで戻るときに使いやすい
}

void rxIR_Interrupt_Handler() { // 割り込みはFALLINGに変えた2020/11/15
  if (x > maxLen) return; //ignore if irBuffer is already full
  irBuffer[x++] = micros(); //just continually record the time-stamp of signal transitions
}

// end of program

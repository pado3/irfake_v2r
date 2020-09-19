/* irfake_v2r: Fake IR remote controll from Viera to Regza
 *  Send Regza TV command from IR
 *   when received Viera TV command with DIGA BD controller
 *  refs:
 *    https://qiita.com/dokkozo/items/25c6e17fcc2e655d5d42
 *    IRsendDemo on https://github.com/z3t0/Arduino-IRremote
 *    https://gist.github.com/sukedai/1759605
 *  Version 1.0 by pado3 2020/09/18
 */

#include <stdio.h>
#include <IRremote.h>
#define maxLen 500 // 信号の最大長, Vieraは基本的に100
#define IR_VCC 10  // IRICのVCC
#define IR_GND 9   // IRICのGND
// IR_RECEIVE_PIN = 2 // attachInterrupt(0, ...)を使う
// IR_SEND_PIN 3      // どこで定義されているか不明だがIRremote制限
// 次の2つはISR割り込みルーチンで変化するためレジスタでなくRAMからロード
volatile unsigned int irBuffer[maxLen]; // IRを受信した時間
volatile unsigned int x = 0; // 信号の長さ Pointer thru irBuffer
int splash = 0;
IRsend irsend;

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
  attachInterrupt(0, rxIR_Interrupt_Handler, CHANGE);
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
    // 消すまでの待ち時間はrecvViera側で設定した
    digitalWrite(LED_BUILTIN, LOW);
    Serial.println(F(", wait next"));
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
  String recvs; // ゴミが紛れ込むことがあるので、まずStringで受けて、
  unsigned long recvL;  // データを確認したら数値に変換して返す
  int i;  // ありきたりなループカウンタ
  if (maxLen < x) { // 派手なノイズが入ったとき
    Serial.print(F("TOO NOISY! count:"));
    Serial.println(x);
    x = 0;
    recvL = 0;
  }
  if (100 <= x ) { // for Panasonic Viera
    int cnt = 0;    // digit
    int hexval = 0; // sum of 8digits C_0 C_1 ... C_7
    // int slen = (x - 3)/2; // length of signal [bit]
    recvs = "";      // 受信データ文字列, String
    recvL = 0;       // 受信データ数値, unsigned long
    detachInterrupt(0); // いったん割り込みを止める
    // 絶対時間を相対時間すなわちパルス長に変換
    for (int i = 1; i < x; i++) {
      irBuffer[i-1] = irBuffer[i] - irBuffer[i-1];
    }
    // hex array. 8T 4T { } T
    for (i = 2; i < 99; i++) {  // Panasonic Viera 48bit
      if (i%2 == 1){
        if (irBuffer[i]> 900){ // T=450usならおよそ2Tが閾値
          hexval += 1 << cnt;
        }
        cnt++;
        if (cnt == 8) { // 8bitまとまったらHEX文字列を得る
          if(hexval < 16) { // padding
            recvs += "0";
          }
          recvs += String(hexval, HEX);
          cnt = 0;
          hexval = 0;
        }
      }
    }
    if (!recvs.startsWith("022080")) { // ヘッダが0x022080ならほぼ正しいデータ
      Serial.print(recvs);
      Serial.println(F(" received but maybe noise (or DIGA), return NULL"));
      recvs = "";
      recvL = 0;
    } else {
      // Vieraのヘッダと思しき 0x022080 を切り詰める
      recvs = recvs.substring(6);
      // 数値のHEXに変換
      recvL = Str2Hex(recvs.c_str());
    }
    // Serial.println();
    x = 0;  // 割り込み再開前に初期化する
    attachInterrupt(0, rxIR_Interrupt_Handler, CHANGE);
  }
  delay(300); // 待ちを入れないとノイズを拾いやすい。このぐらいが適当そう
  return recvL;
}

unsigned long Str2Hex(const char* str)
{
     return strtol(str, 0, 16); // strtoulだと最上位ビットが反転してしまった
}

void rxIR_Interrupt_Handler() { // 割り込みは常にCHANGEで呼ぶ
  if (x > maxLen) return; //ignore if irBuffer is already full
  irBuffer[x++] = micros(); //just continually record the time-stamp of signal transitions
}

// end of program

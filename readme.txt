irfake_v2r
REGZA TV contorol with Panasonic VIERA controller, Arduino + IR （IR FAKE program _ from Viera to Regza)

パナソニックVIERAへの赤外線リモコン信号を、東芝REGZAのリモコン信号に変換するArduinoスケッチです。 送信にIRremoteを使用しています： https://github.com/z3t0/Arduino-IRremote

Arduino UNOと、Arduinoブートローダー書き込み済みATmega328P-PUで動作確認しています。 赤外リモコン受信モジュールの出力はD2ピンに直接接続します。 赤外線出力は、D3ピンから47Ω程度の抵抗を通して赤外線LEDのアノードに接続し、赤外線LEDのカソードはGNDに接続します。 「お約束」の13番ピンに接続したLEDで動作確認ができます。また、動作の詳細はシリアルコンソールで読み込めます。

元々、IRremoteのライブラリだけで受信して出力するつもりだったのですが、受信後の送信が行われず、送信コマンドを打つと受信割り込みが復帰しないという問題に直面したため、受信側を自前で書いた次第です。

ご利用は自己責任でお願いします。
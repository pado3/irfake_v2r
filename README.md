# irfake_v2r
REGZA TV contorol with Panasonic VIERA controller, Arduino + IR

（IR FAKE program _ from Viera to Regza)

パナソニックVIERA用の赤外線リモコン信号を、東芝REGZA TV用のリモコン信号に変換するArduinoスケッチです。

送信にIRremoteを使用しています： https://github.com/z3t0/Arduino-IRremote

Arduino UNOと、Arduinoブートローダー書き込み済みATmega328P-PUで動作確認しています。
赤外リモコン受信モジュールの出力はD2ピンに直接接続します。
赤外線出力は、D3ピンから47Ω程度の抵抗を通して赤外線LEDのアノードに接続し、赤外線LEDのカソードはGNDに接続します。
「お約束」の13番ピンに接続したLEDで動作確認ができます。また、動作の詳細はシリアルコンソールで読み込めます。

元々、IRremoteのライブラリだけで受信して出力するつもりだったのですが、受信後の送信が行われず、送信コマンドを打つと受信割り込みが復帰しないという問題に直面したため、受信側を自前で書いた次第です。

実際に組み立てた内容をブログにまとめています： http://pado.tea-nifty.com/top/2020/09/post-2d412d.html

ご利用は自己責任でお願いします。

2020/11/16追記<br />
・ノイズ対策・動作安定化のため、リーダー部判定・ゴミデータ判定・割り込み条件変更を行いました<br />
・入力が10秒ないとパワーダウンモードに移行するようにしました。<br />

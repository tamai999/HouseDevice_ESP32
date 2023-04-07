# HouseDevice_ESP32
[ハウス環境モニタサービス](https://vhousem.web.app)の計測器サンプルコードです。<br>
ハウス内気温・湿度、地温、外気温、Wi-Fi電波強度をハウス環境モニタサービスに定期的に送信します。

## ■セットアップ

### 開発環境
- Arduino IDE

### マイコン/センサ
サンプルプログラムは次のハードウェア上で動作します。

| パーツ | 製品名 |
|:---|:---|
|マイコン| [M5 ATOM Lite](http://docs.m5stack.com/en/core/atom_lite) |
|温湿度センサ|[RHT03](https://learn.sparkfun.com/tutorials/rht03-dht22-humidity-and-temperature-sensor-hookup-guide/all) |
|地温|[DS18B20](https://www.analog.com/jp/products/ds18b20.html#product-overview)|
|外気温|※地温と同様|

### センサが利用するPIN

| センサー | PIN | 備考 |
|:---:|:---:|:---|
|RHT03|32|Groveコネクタに接続|
|DS18B20|25|地温と外気温は同一PIN。4.7k外部抵抗でプルアップ接続|

### ライブラリのインストール
Arduino IDEで次のライブラリをインストールしてください。

1. OneWire/Arduino-Temperature-Control-Library
    1. [OneWireのコード](https://github.com/PaulStoffregen/OneWire) をZip形式でダウンロードしてローカルPCに展開する。
    1. Arduino IDEの　スケッチ ＞ ライブラリーをインクルード ＞ .ZIP形式のライブラリーをインストール　を選択する。
    1. 同様に、[Arduino-Temperature-Control-Libraryのコード](https://github.com/milesburton/Arduino-Temperature-Control-Library) をZip形式でダウンロードしてインストールする。
1. RHT03 ライブラリ
    1. Arduino IDEの　スケッチ ＞ ライブラリーをインクルード ＞ ライブラリを管理　を選択
    1. 「RHT03」を検索して 「SparkFun RHT03 Arduino Library」 をインストール。
1. M5 ATOM ライブラリ
    1. Arduino IDEの　スケッチ ＞ ライブラリーをインクルード ＞ ライブラリを管理　を選択
    1. 「M5 ATOM」を検索して 「Library for M5Atom Core development kit」 をインストール。「Would you like to install also all the missing dependencies?」と表示されたら 「Install all」を選択する。 

### 計測器ID、送信パスコードを設定
サンプルコードの次の変数の値を、ハウス環境モニタアプリの計測器画面に表示される値に変更します。

```cpp
#define DEVICE_ID "abcdefghi"
#define PASSCODE  "1234567890"  
```

計測器ID及び送信パスコードは[ハウス環境モニタアプリ](https://apps.apple.com/jp/app/id6444917835)の　設定　＞　計測器一覧　から計測器を選択することで確認できます。

### Wi-Fi設定

本サンプルプログラムは SmargConfig を利用しているため、Wi-FiのSSID、パスワードはスマホから設定します。
M5 Atom Liteの本体ボタンを押したままUSBに電源供給することで、SmartConfig の待受状態になる（本体LEDが赤色になる）のでSmartConfigを設定できるアプリから設定してください。

## ■使い方

- USB-Cに電源供給することで、10分に1回の頻度で計測値をハウス環境モニタサービスに送信します。


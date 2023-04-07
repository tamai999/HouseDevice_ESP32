#include <WiFi.h>               // WiFi接続用
#include <HTTPClient.h>         // HTTP通信用
#include <WiFiClientSecure.h>   // HTTPS用
#include <OneWire.h>            // DS18B20用
#include <DallasTemperature.h>  // DS18B20用
#include <SparkFun_RHT03.h>     // 温湿度センサ用
#include "M5Atom.h"             // M5Atom用

// ======================================
// 定数
// ======================================
#define DEBUG 1                   // ログ出力フラグ

#define DEVICE_ID "abcdefghi"     // 計測器ID。ハウス環境モニタアプリの　設定画面の計測器IDを設定してください。
#define PASSCODE  "1234567890"    // 計測器パスコード。ハウス環境モニタアプリの　設定画面から上記の計測器を選択し表示される送信パスコードを設定してください。

#define RHT03_DATA_PIN 32         // RHT03 温湿度センサーピン番号
#define TEMP_PIN       25         // DS18B20 外気温/地温センサーピン番号　※GPIOピンを指定

#define RUN_INTERVAL   600        // 送信間隔（10分）

// ======================================
// HTTPS用ルート証明書
// ======================================
const char* ROOT_CA =
"-----BEGIN CERTIFICATE-----\n" \
"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\n" \
"-----END CERTIFICATE-----\n";

// ======================================
// ログ
// ======================================
void serialLog(String text) {
#if DEBUG
  Serial.println(text);
  Serial.flush();
#else
  return;
#endif
}

// ======================================
// HTTPS通信処理
// ======================================
bool httpGet(String url) {
    WiFiClientSecure *client = new WiFiClientSecure;
    if (!client) {
      return false;
    }

    client->setCACert(ROOT_CA);
    HTTPClient https;
    if (!https.begin(*client, url)) {
      serialLog("Faild to connect http server.");

      delete client;
      return false;
    }
    
    https.setTimeout(10000);
    https.setConnectTimeout(10000);
    int status = https.GET();
    if (status < 0) {
      serialLog("Faild http get. status: " + String(status));

      https.end();
      delete client;
      return false;
    }

    serialLog("HTTP Response status: " + String(status));
    String body = https.getString();
    serialLog("HTTP Response body: " + body);
    https.end();
    delete client;

    if (status == HTTP_CODE_OK) {
      return true;
    }
    return false;
}

// ======================================
// メイン処理
// ======================================
void setup()
{
  const unsigned long startTime = millis();

  // ======================================
  // M5 ATOM Lite初期化
  // ======================================
  // シリアル初期化有無、I2Cの初期化有無、LED初期化有無
  M5.begin(true, false, true);
  delay(50);
  M5.dis.drawpix(0, 0x00FF00);  // 緑

  // ======================================
  // Deep Sleep設定
  // ======================================
  // Deep sleep時、周辺機器への電力供給を止める
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
  // ディープスリープから復帰した場合に保持しておくメモリ領域への電源供給は不要
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF); 
  // RTC_DATA_ATTRに保存したデータの保持は不要
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
  // Deep sleep時、水晶振動子への電力供給を止める
  esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL, ESP_PD_OPTION_OFF);
  
  // ======================================
  // デバッグ用シリアル通信初期化
  // ======================================
#if DEBUG
  Serial.begin(9600);
  while (!Serial) {
    ;
  }
  delay(5000);
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER) {
    serialLog("Start by timer.");
  } else {
    serialLog("Ready.");
  }
#endif

  // ======================================
  // センサー初期化
  // ======================================
  RHT03 rht;                      // RHT03 温湿度センサー
  rht.begin(RHT03_DATA_PIN);
  
  OneWire oneWire(TEMP_PIN);
  DallasTemperature ds(&oneWire); // DS18B20 外気温/地温センサー
  ds.begin();
  
  // ======================================
  // 外気温/地温取得
  // ======================================
  ds.requestTemperatures();
  float soilTemp = ds.getTempCByIndex(0);   // 注）地温と外気温のどちらが(0)でどちらが(1)となるかはセンサ接続後に確認してください。
  float outerTemp = ds.getTempCByIndex(1);
  serialLog("Soil Temp: " + String(soilTemp, 1) + " C");
  serialLog("Outer Temp: " + String(outerTemp, 1) + " C");

  // ======================================
  // 温湿度取得
  // ======================================
  float humidity = 0;
  float houseTemp = 0;

  for (int i = 0; i < 5; i++) {
    int updateRet = rht.update();
    if (updateRet == 1) {
      humidity = rht.humidity();
      houseTemp = rht.tempC();
      serialLog("Humidity: " + String(humidity, 1) + " %");
      serialLog("House Temp: " + String(houseTemp, 1) + " C");
      break;
    } else {
      serialLog("Did fail to get temperature/humidity. wait for retry.");
      delay(RHT_READ_INTERVAL_MS);
    }
  }

  // ======================================
  // Wi-Fi SSID/PW設定
  // 電源投入時にボタンが押されていたらSmartConfigを開始
  // ======================================
  M5.update();
  if (M5.Btn.isPressed()) {
    serialLog("Button Pressed.");
    M5.dis.drawpix(0, 0xFF0000);  // 赤

    // SmartConfig初期化
    WiFi.mode(WIFI_AP_STA);
    WiFi.beginSmartConfig();
    
    serialLog("Waiting for SmartConfig.");
    while (!WiFi.smartConfigDone()) {
      delay(500);
      serialLog(".");
    }
    serialLog("SmartConfig received.");
  } else {
    WiFi.mode(WIFI_STA);
    WiFi.begin();
    serialLog("Button Released.");
  }

  // WiFiに接続
  serialLog("Waiting for WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    serialLog(".");
  }
  // 接続成功
  serialLog("WiFi Connected.");
  long rssi = WiFi.RSSI();
  serialLog("RSSI: " + String(rssi) + " dBm");

  // ======================================
  // HTTPデータ送信
  // ======================================
  M5.dis.drawpix(0, 0x0000FF);  // 青

  String url = "https://asia-northeast1-vhousem.cloudfunctions.net/w?";
  url.concat("did=" + String(DEVICE_ID) + "&pc=" + String(PASSCODE));
  url.concat("&wifi=");
  url.concat(rssi);
  url.concat("&ht=");
  url.concat(houseTemp);
  url.concat("&st=");
  url.concat(soilTemp);
  url.concat("&h=");
  url.concat((int)humidity);
  url.concat("&ot=");
  url.concat(outerTemp);
  bool result = httpGet(url);

  if (result) {
    serialLog("Successfully sent.");
    M5.dis.drawpix(0, 0x00FF00);  // 緑
    delay(500);
    M5.dis.drawpix(0, 0x000000);
    delay(500);
    M5.dis.drawpix(0, 0x00FF00);  // 緑
  } else {
    serialLog("Failed to send.");
    M5.dis.drawpix(0, 0xFF0000);  // 赤
    delay(500);
    M5.dis.drawpix(0, 0x000000);
    delay(500);
    M5.dis.drawpix(0, 0xFF0000);  // 赤
  }
  delay(500);
  M5.dis.drawpix(0, 0x000000);
  delay(100);
    
  // ======================================
  // deep sleep
  // ======================================
  const unsigned int now = millis();
  const unsigned int runInterval = RUN_INTERVAL * 1000 * 1000 - (now - startTime) * 1000;

  esp_sleep_enable_timer_wakeup(runInterval);
  esp_deep_sleep_start();
}

void loop()
{
}

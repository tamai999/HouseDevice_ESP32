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

#define WIFI_RETRY_COUNT 4        // WiFi接続リトライ回数
#define RUN_AFTER_WIFI_FAILED 180 // WiFi接続失敗後の再起動時間（3分）

// ======================================
// HTTPS用ルート証明書
// ======================================
const char* ROOT_CA =
"-----BEGIN CERTIFICATE-----\n" \
"MIIFVzCCAz+gAwIBAgINAgPlk28xsBNJiGuiFzANBgkqhkiG9w0BAQwFADBHMQsw\n" \
"CQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2VzIExMQzEU\n" \
"MBIGA1UEAxMLR1RTIFJvb3QgUjEwHhcNMTYwNjIyMDAwMDAwWhcNMzYwNjIyMDAw\n" \
"MDAwWjBHMQswCQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZp\n" \
"Y2VzIExMQzEUMBIGA1UEAxMLR1RTIFJvb3QgUjEwggIiMA0GCSqGSIb3DQEBAQUA\n" \
"A4ICDwAwggIKAoICAQC2EQKLHuOhd5s73L+UPreVp0A8of2C+X0yBoJx9vaMf/vo\n" \
"27xqLpeXo4xL+Sv2sfnOhB2x+cWX3u+58qPpvBKJXqeqUqv4IyfLpLGcY9vXmX7w\n" \
"Cl7raKb0xlpHDU0QM+NOsROjyBhsS+z8CZDfnWQpJSMHobTSPS5g4M/SCYe7zUjw\n" \
"TcLCeoiKu7rPWRnWr4+wB7CeMfGCwcDfLqZtbBkOtdh+JhpFAz2weaSUKK0Pfybl\n" \
"qAj+lug8aJRT7oM6iCsVlgmy4HqMLnXWnOunVmSPlk9orj2XwoSPwLxAwAtcvfaH\n" \
"szVsrBhQf4TgTM2S0yDpM7xSma8ytSmzJSq0SPly4cpk9+aCEI3oncKKiPo4Zor8\n" \
"Y/kB+Xj9e1x3+naH+uzfsQ55lVe0vSbv1gHR6xYKu44LtcXFilWr06zqkUspzBmk\n" \
"MiVOKvFlRNACzqrOSbTqn3yDsEB750Orp2yjj32JgfpMpf/VjsPOS+C12LOORc92\n" \
"wO1AK/1TD7Cn1TsNsYqiA94xrcx36m97PtbfkSIS5r762DL8EGMUUXLeXdYWk70p\n" \
"aDPvOmbsB4om3xPXV2V4J95eSRQAogB/mqghtqmxlbCluQ0WEdrHbEg8QOB+DVrN\n" \
"VjzRlwW5y0vtOUucxD/SVRNuJLDWcfr0wbrM7Rv1/oFB2ACYPTrIrnqYNxgFlQID\n" \
"AQABo0IwQDAOBgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4E\n" \
"FgQU5K8rJnEaK0gnhS9SZizv8IkTcT4wDQYJKoZIhvcNAQEMBQADggIBAJ+qQibb\n" \
"C5u+/x6Wki4+omVKapi6Ist9wTrYggoGxval3sBOh2Z5ofmmWJyq+bXmYOfg6LEe\n" \
"QkEzCzc9zolwFcq1JKjPa7XSQCGYzyI0zzvFIoTgxQ6KfF2I5DUkzps+GlQebtuy\n" \
"h6f88/qBVRRiClmpIgUxPoLW7ttXNLwzldMXG+gnoot7TiYaelpkttGsN/H9oPM4\n" \
"7HLwEXWdyzRSjeZ2axfG34arJ45JK3VmgRAhpuo+9K4l/3wV3s6MJT/KYnAK9y8J\n" \
"ZgfIPxz88NtFMN9iiMG1D53Dn0reWVlHxYciNuaCp+0KueIHoI17eko8cdLiA6Ef\n" \
"MgfdG+RCzgwARWGAtQsgWSl4vflVy2PFPEz0tv/bal8xa5meLMFrUKTX5hgUvYU/\n" \
"Z6tGn6D/Qqc6f1zLXbBwHSs09dR2CQzreExZBfMzQsNhFRAbd03OIozUhfJFfbdT\n" \
"6u9AWpQKXCBfTkBdYiJ23//OYb2MI3jSNwLgjt7RETeJ9r/tSQdirpLsQBqvFAnZ\n" \
"0E6yove+7u7Y/9waLd64NnHi/Hm3lCXRSHNboTXns5lndcEZOitHTtNCjv0xyBZm\n" \
"2tIMPNuzjsmhDYAPexZ3FL//2wmUspO8IFgV6dtxQ/PeEMMA3KgqlbbC1j+Qa3bb\n" \
"bP6MvPJwNQzcmRk13NfIRmPVNnGuV/u3gm3c\n" \
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
  float humidity = -999.0;
  float houseTemp = -999.0;

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
  for (int i = 0; i <= WIFI_RETRY_COUNT; i++) {
    if (WiFi.status() != WL_CONNECTED) {
      if (i == WIFI_RETRY_COUNT) {
        // 接続失敗
        serialLog("Faild to Conenct WiFi.");
        // 一定時間後に再起動
        esp_sleep_enable_timer_wakeup(RUN_AFTER_WIFI_FAILED * 1000 * 1000);
        esp_deep_sleep_start();
      }

      delay(1000);
      serialLog(".");
    } else {
      // 接続成功
      break;
    }
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

  if (-200.0 <= rssi && rssi <= 0.0) {
    url.concat("&wifi=");
    url.concat(rssi);
  }

  if (-50.0 <= houseTemp && houseTemp <= 100.0) {
    url.concat("&ht=");
    url.concat(houseTemp);
  }

  if (-50.0 <= soilTemp && soilTemp <= 100.0) {
    url.concat("&st=");
    url.concat(soilTemp);
  }

  if (0.0 <= humidity && humidity <= 100.0) {
    url.concat("&h=");
    url.concat((int)humidity);
  }

  if (0.0 <= outerTemp && outerTemp <= 100.0) {
    url.concat("&ot=");
    url.concat(outerTemp);
  }

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

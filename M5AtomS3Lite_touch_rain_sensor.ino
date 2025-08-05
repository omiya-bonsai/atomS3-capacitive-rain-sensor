/*
 * AtomS3Lite用 2端子静電容量式レインセンサー スケッチ
 * MQTT送信、PUSHOVER通知、バッテリー監視機能付き
 * FASTLED制御機能付き
 *
 * ============================================================================
 * 【このスケッチの機能概要】
 * ============================================================================
 *
 * このプログラムは、M5AtomS3マイコンを使用して雨を自動検知するセンサーシステムです。
 * 2本の導線を使った静電容量式センサーで、雨粒による電気的変化を検出します。
 *
 * 【基本的な仕組み】
 * ・2本の端子（PIN1とPIN2）の間の静電容量を測定
 * ・雨粒が付着すると静電容量が変化することを利用
 * ・変化量が設定した閾値（15％）を超えると雨と判定
 * ・誤検出を防ぐため、3回連続で検出された場合のみ雨と確定
 *
 * 【4つの測定方式を自動選択】
 * 1. 充電時間測定方式：PIN2をHIGHにしてPIN1がHIGHになるまでの時間を測定
 * 2. 逆方向測定方式：上記の逆方向（PIN1→PIN2）で測定
 * 3. 発振検出方式：PIN1をトグルしてPIN2の状態変化をカウント
 * 4. アナログ測定方式：PIN1をVCCにしてPIN2のアナログ値を読取り
 *
 * 起動時に全ての方式をテストし、最も安定した方式を自動選択します。
 *
 * 【LED状態表示機能】
 * M5AtomS3の内蔵LED（WS2812）で動作状態を色で表示：
 * ・緑色点滅（200ms）：起動中・センサー校正中
 * ・青色常灯：正常動作中（雨なし）
 * ・紫色点滅（500ms）：雨検知中
 * ・赤色点滅（300ms）：エラー発生（ケーブル脱落、WiFi切断など）
 *
 * 【MQTT通信機能】
 * 30秒間隔でセンサーデータをMQTTブローカーに送信：
 * ・デバイスID、ベースライン値、現在値、変化率
 * ・雨検知状態、測定方式、稼働時間
 * ・ケーブル接続状態、エラー回数
 * ・UNIXタイムスタンプ（日本時間）
 *
 * 【PUSHOVER通知機能】
 * スマートフォンへのプッシュ通知：
 * ・雨検知通知：07:00-19:00の時間帯のみ、3時間のクールダウン付き
 * ・ケーブル脱落通知：センサー接続不良を検出時
 * ・バッテリー警告：25時間稼働後（静穏時間19:00-03:00を避ける）
 *
 * 【エラー検知機能】
 * ・ケーブル脱落検知：5回連続で測定エラーが発生した場合
 * ・WiFi接続監視：切断時の自動再接続
 * ・MQTT接続監視：切断時の自動再接続
 * ・NTP時刻同期：通知時間制御のための正確な時刻取得
 *
 * 【動的ベースライン調整】
 * 環境変化に自動適応：
 * ・30秒ごとにベースライン値を評価
 * ・5-10％の継続的な変化はドリフトとして補正
 * ・雨検知中は調整を停止して誤判定を防止
 *
 * 【ノイズフィルタリング】
 * 安定した検出のための信号処理：
 * ・5％未満の変化はノイズとして除去
 * ・ローパスフィルタ（α=0.8）で短期的な変動を平滑化
 * ・10％の追加安定性チェックで確実な検出
 *
 * 【時間制御機能】
 * 近隣への配慮とバッテリー節約：
 * ・通知時間制限：07:00-19:00のみ雨通知を送信
 * ・静穏時間：19:00-03:00はバッテリー警告も抑制
 * ・クールダウン：同じ種類の通知は3時間間隔で制限
 *
 * 【設定可能なパラメータ】
 * ・雨検知閾値：15％（RAIN_THRESHOLD_PERCENT）
 * ・ノイズ閾値：5％（NOISE_THRESHOLD）
 * ・安定性チェック：3回（STABILITY_CHECK_COUNT）
 * ・MQTT送信間隔：30秒（MQTT_SEND_INTERVAL）
 * ・通知時間帯：7-19時（PUSHOVER_START_HOUR～PUSHOVER_END_HOUR）
 * ・クールダウン：3時間（PUSHOVER_COOLDOWN_HOURS）
 *
 * 【電源管理】
 * ・バッテリー動作想定：25時間での警告通知
 * ・WiFi省電力：必要時のみ再接続
 * ・LED輝度調整：50/255で消費電力を抑制
 *
 * 【接続要件】
 * ・WiFi環境：SSID/パスワードはconfig.hで設定
 * ・MQTTブローカー：ローカルまたはクラウド
 * ・PUSHOVERアカウント：API token/User keyが必要
 * ・NTPサーバー：ntp.nict.jp（日本標準時取得）
 *
 * 【ハードウェア構成】
 * ・M5AtomS3 Lite（ESP32-S3搭載）
 * ・2端子静電容量式レインセンサー
 * ・GROVE/Dupontケーブル（PIN1/PIN2接続）
 * ・屋外設置用の防水ケース推奨
 *
 * 【動作フロー】
 * 1. 起動時：WiFi接続、NTP同期、MQTT接続テスト
 * 2. センサー校正：4つの測定方式をテストして最適方式を選択
 * 3. ベースライン確立：乾燥状態での基準値を測定
 * 4. 監視ループ：1秒間隔でセンサー測定、状態判定、通信処理
 * 5. 通知送信：条件を満たした場合のMQTT送信、PUSHOVER通知
 *
 * 【トラブルシューティング】
 * ・緑点滅が続く：センサー接続確認、config.h設定確認
 * ・赤点滅：ケーブル確認、WiFi設定確認
 * ・通知が来ない：時間帯確認、PUSHOVER設定確認
 * ・誤検出：RAIN_THRESHOLD_PERCENTを調整（15→20％など）
 *
 * 【セキュリティ注意事項】
 * ・config.hファイルには機密情報（WiFiパスワード、APIキー）を含む
 * ・GitHubなどにアップロード時はconfig.hを除外すること
 * ・MQTTブローカーのアクセス制御を適切に設定すること
 *
 * 【応用可能性】
 * ・農業：自動散水システムの雨検知センサー
 * ・住宅：洗濯物取込み通知、窓閉め通知
 * ・施設管理：屋外イベントの雨対策判断
 * ・気象観測：簡易雨量計としてのデータ収集
 *
 * 作成者：GitHub Copilot assisted development
 * 更新日：2025年8月4日
 * バージョン：v2.1（実運用モード、ケーブル脱落検知強化）
 * ライセンス：MIT License
 *
 * ============================================================================
 */

#include <M5AtomS3.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <FastLED.h>
#include "config.h"

#define SENSOR_PIN1 1 // センサー端子1
#define SENSOR_PIN2 2 // センサー端子2

// FASTLED設定
#define LED_PIN 35        // M5AtomS3の内蔵LED（WS2812）
#define NUM_LEDS 1        // LEDの数
#define LED_BRIGHTNESS 50 // LED輝度（0-255）

CRGB leds[NUM_LEDS];

// LED状態管理
enum LEDState
{
  LED_STARTUP,    // 起動中（緑点滅）
  LED_NORMAL,     // 正常動作（青常灯）
  LED_RAIN,       // 雨検知（紫点滅）
  LED_ERROR,      // エラー（赤点滅）
  LED_WIFI_ERROR, // WiFiエラー（オレンジ点滅）
  LED_MQTT_ERROR  // MQTTエラー（黄色点滅）
};

LEDState current_led_state = LED_STARTUP;
unsigned long last_led_update = 0;
bool led_blink_state = false;

#define MEASUREMENT_SAMPLES 20
#define BASELINE_SAMPLES 15
#define RAIN_THRESHOLD_PERCENT 15.0 // 閾値を15%に上げて誤検出を減らす
#define NOISE_THRESHOLD 5.0         // ノイズ閾値も上げる
#define LOWPASS_ALPHA 0.8
#define STABILITY_CHECK_COUNT 3              // 安定性チェック用
#define BASELINE_UPDATE_INTERVAL 30000       // 30秒ごとにベースライン更新
#define BASELINE_UPDATE_THRESHOLD 10.0       // ベースライン更新の変化率閾値
#define MQTT_SEND_INTERVAL 30000             // MQTT送信間隔（30秒）
#define BATTERY_CHECK_HOURS 25               // バッテリーチェック時間（25時間）
#define PUSHOVER_START_HOUR 7                // PUSHOVER通知開始時刻
#define PUSHOVER_END_HOUR 19                 // PUSHOVER通知終了時刻
#define QUIET_START_HOUR 19                  // 静穏時間開始
#define QUIET_END_HOUR 3                     // 静穏時間終了
#define DELAY_AFTER_QUIET 1                  // 静穏時間後の遅延（分）
#define PUSHOVER_COOLDOWN_HOURS 3            // PUSHOVER通知のクールダウン時間（3時間）
#define CABLE_ERROR_THRESHOLD 5              // ケーブル脱落判定の連続エラー回数
#define MEASUREMENT_ERROR_THRESHOLD 10       // 測定エラー判定の連続回数
#define DEBUG_IGNORE_TIME_LIMITS false       // デバッグ用：時刻制限を無視する場合はtrue
#define DEBUG_SHORT_COOLDOWN false           // デバッグ用：クールダウンを短縮する場合はtrue（1分）
#define DEBUG_FORCE_RESET_NOTIFICATION false // デバッグ用：毎回通知フラグを強制リセット
#define DEBUG_MODE false                     // デバッグ出力の有効/無効

// 24/7運用堅牢性設定
#define WATCHDOG_TIMEOUT_SECONDS 120                // ウォッチドッグタイムアウト（2分）
#define SCHEDULED_REBOOT_INTERVAL (168UL * 3600000) // 定期再起動間隔（7日間）
#define MEMORY_CHECK_INTERVAL 300000                // メモリチェック間隔（5分）
#define MEMORY_WARNING_THRESHOLD 8192               // メモリ警告閾値（8KB）
#define WIFI_RECONNECT_THRESHOLD 10                 // WiFi再接続エラー閾値
#define MQTT_RECONNECT_THRESHOLD 10                 // MQTT再接続エラー閾値
#define HEALTH_REPORT_INTERVAL 3600000              // ヘルスレポート間隔（1時間）

// NTPサーバー設定
const char *ntpServer = "ntp.nict.jp";
const long gmtOffset_sec = 9 * 3600; // JST (UTC+9)
const int daylightOffset_sec = 0;

unsigned long baseline_value = 0;
unsigned long current_value = 0;
float filtered_change_percent = 0;
bool is_raining = false;
int selected_method = 0;                // 1:充電時間, 2:逆充電, 3:発振, 4:アナログ
int rain_detection_count = 0;           // 連続検出カウンター
unsigned long last_baseline_update = 0; // 最後のベースライン更新時刻
bool baseline_calibrated = false;       // ベースライン校正完了フラグ

// 新機能用変数
unsigned long last_mqtt_send = 0;    // 最後のMQTT送信時刻
unsigned long startup_time = 0;      // 起動時刻
bool rain_notification_sent = false; // 雨通知送信フラグ
bool battery_warning_sent = false;   // バッテリー警告送信フラグ
bool cable_error_sent = false;       // ケーブルエラー通知送信フラグ
bool wifi_connected = false;

// PUSHOVER通知のクールダウン管理
unsigned long last_rain_notification = 0;    // 最後の雨通知時刻
unsigned long last_battery_notification = 0; // 最後のバッテリー通知時刻
unsigned long last_cable_notification = 0;   // 最後のケーブル通知時刻

// ケーブル脱落検知用
int consecutive_measurement_errors = 0; // 連続測定エラー回数
bool cable_connection_ok = true;        // ケーブル接続状態

// 24/7堅牢運用のための監視変数
unsigned long last_reboot_time = 0;      // 最後の再起動時刻
unsigned long last_memory_check = 0;     // 最後のメモリチェック時刻
unsigned long last_health_report = 0;    // 最後のヘルスレポート送信時刻
unsigned long wifi_disconnect_count = 0; // WiFi切断回数
unsigned long mqtt_disconnect_count = 0; // MQTT切断回数
unsigned long system_start_time = 0;     // システム開始時刻

// WiFi・MQTTクライアント
WiFiClient espClient;
PubSubClient mqtt_client(espClient);

// MQTTバッファサイズ設定
void setupMQTTBuffers()
{
  mqtt_client.setBufferSize(512); // バッファサイズを512バイトに設定
  mqtt_client.setKeepAlive(60);   // キープアライブを60秒に設定
}

// 関数の前方宣言
void sendPushoverNotification(const String &message, const String &title);
bool isNotificationTime();
unsigned long getUnixTimestamp();
void connectToMQTT();

// URLエンコーディングのためのヘルパー関数
String urlEncode(String str)
{
  String encodedString = "";
  char c;
  char code0;
  char code1;
  for (int i = 0; i < str.length(); i++)
  {
    c = str.charAt(i);
    if (c == ' ')
    {
      encodedString += "+";
    }
    else if (isalnum(c))
    {
      encodedString += c;
    }
    else
    {
      code1 = (c & 0xf) + '0';
      if ((c & 0xf) > 9)
      {
        code1 = (c & 0xf) - 10 + 'A';
      }
      c = (c >> 4) & 0xf;
      code0 = c + '0';
      if (c > 9)
      {
        code0 = c - 10 + 'A';
      }
      encodedString += '%';
      encodedString += code0;
      encodedString += code1;
    }
  }
  return encodedString;
}

// LED制御関数
void updateLEDStatus()
{
  unsigned long current_time = millis();

  switch (current_led_state)
  {
  case LED_STARTUP:
    // 緑色高速点滅（200ms間隔）
    if (current_time - last_led_update >= 200)
    {
      led_blink_state = !led_blink_state;
      if (led_blink_state)
      {
        leds[0] = CRGB::Green;
      }
      else
      {
        leds[0] = CRGB::Black;
      }
      FastLED.show();
      last_led_update = current_time;
    }
    break;

  case LED_NORMAL:
    // 青色常灯
    leds[0] = CRGB::Blue;
    FastLED.show();
    break;

  case LED_RAIN:
    // 紫色点滅（500ms間隔）
    if (current_time - last_led_update >= 500)
    {
      led_blink_state = !led_blink_state;
      if (led_blink_state)
      {
        leds[0] = CRGB::Purple;
      }
      else
      {
        leds[0] = CRGB::Black;
      }
      FastLED.show();
      last_led_update = current_time;
    }
    break;

  case LED_ERROR:
    // 赤色点滅（300ms間隔）
    if (current_time - last_led_update >= 300)
    {
      led_blink_state = !led_blink_state;
      if (led_blink_state)
      {
        leds[0] = CRGB::Red;
      }
      else
      {
        leds[0] = CRGB::Black;
      }
      FastLED.show();
      last_led_update = current_time;
    }
    break;

  case LED_WIFI_ERROR:
    // オレンジ色点滅（400ms間隔）
    if (current_time - last_led_update >= 400)
    {
      led_blink_state = !led_blink_state;
      if (led_blink_state)
      {
        leds[0] = CRGB::Orange;
      }
      else
      {
        leds[0] = CRGB::Black;
      }
      FastLED.show();
      last_led_update = current_time;
    }
    break;

  case LED_MQTT_ERROR:
    // 黄色点滅（350ms間隔）
    if (current_time - last_led_update >= 350)
    {
      led_blink_state = !led_blink_state;
      if (led_blink_state)
      {
        leds[0] = CRGB::Yellow;
      }
      else
      {
        leds[0] = CRGB::Black;
      }
      FastLED.show();
      last_led_update = current_time;
    }
    break;
  }
}

// ===== 24/7 堅牢運用のための関数 =====

// ソフトウェアウォッチドッグ（Arduino IDE環境対応）
unsigned long last_watchdog_feed = 0;

void feedWatchdog()
{
  last_watchdog_feed = millis();

  if (DEBUG_MODE)
  {
    Serial.println("DEBUG: Watchdog fed");
  }
}

// ウォッチドッグタイムアウトチェック
void checkWatchdogTimeout()
{
  unsigned long current_time = millis();

  // オーバーフロー対策
  if (current_time < last_watchdog_feed)
  {
    last_watchdog_feed = current_time;
    return;
  }

  // ウォッチドッグタイムアウトチェック
  if (current_time - last_watchdog_feed >= (WATCHDOG_TIMEOUT_SECONDS * 1000))
  {
    Serial.println("WATCHDOG TIMEOUT - System restart triggered");
    delay(100);
    ESP.restart();
  }
}

// メモリチェック関数
bool checkMemoryHealth()
{
  uint32_t free_heap = ESP.getFreeHeap();
  uint32_t free_psram = ESP.getFreePsram();

  if (DEBUG_MODE)
  {
    Serial.printf("DEBUG: Free Heap: %u bytes, Free PSRAM: %u bytes\n", free_heap, free_psram);
  }

  if (free_heap < MEMORY_WARNING_THRESHOLD)
  {
    Serial.printf("WARNING: Low heap memory: %u bytes (threshold: %u)\n",
                  free_heap, MEMORY_WARNING_THRESHOLD);
    return false;
  }

  return true;
}

// システムヘルスレポート送信
void sendSystemHealthReport()
{
  StaticJsonDocument<512> health_doc;
  health_doc["device_id"] = DEVICE_ID;
  health_doc["location"] = LOCATION;
  health_doc["timestamp"] = getUnixTimestamp();
  health_doc["type"] = "system_health";

  // システム情報
  health_doc["uptime_hours"] = (millis() - system_start_time) / 3600000;
  health_doc["free_heap"] = ESP.getFreeHeap();
  health_doc["free_psram"] = ESP.getFreePsram();
  health_doc["wifi_disconnects"] = wifi_disconnect_count;
  health_doc["mqtt_disconnects"] = mqtt_disconnect_count;
  health_doc["cpu_freq_mhz"] = ESP.getCpuFreqMHz();
  health_doc["wifi_rssi"] = WiFi.RSSI();

  // JSON文字列に変換
  String health_payload;
  serializeJson(health_doc, health_payload);

  // MQTT送信
  if (mqtt_client.connected())
  {
    String health_topic = String(MQTT_TOPIC_PREFIX) + "/health";
    if (mqtt_client.publish(health_topic.c_str(), health_payload.c_str()))
    {
      Serial.println("System health report sent");
      if (DEBUG_MODE)
      {
        Serial.println("Health payload: " + health_payload);
      }
    }
    else
    {
      Serial.println("Failed to send health report");
    }
  }

  last_health_report = millis();
}

// 定期再起動チェック
void checkScheduledReboot()
{
  unsigned long current_time = millis();

  // オーバーフロー対策
  if (current_time < last_reboot_time)
  {
    last_reboot_time = current_time;
    return;
  }

  if (current_time - last_reboot_time >= SCHEDULED_REBOOT_INTERVAL)
  {
    Serial.println("Scheduled reboot triggered - System has been running for over configured interval");

    // 再起動前に最後のヘルスレポートを送信
    sendSystemHealthReport();
    delay(1000);

    // 再起動実行
    ESP.restart();
  }
}

// WiFi/MQTT接続監視と再接続
void monitorConnections()
{
  static unsigned long last_wifi_check = 0;
  static unsigned long last_mqtt_check = 0;

  unsigned long current_time = millis();

  // WiFi接続チェック
  if (current_time - last_wifi_check >= 5000) // 5秒毎
  {
    if (WiFi.status() != WL_CONNECTED)
    {
      wifi_disconnect_count++;
      wifi_connected = false;

      Serial.println("WiFi disconnected - attempting reconnection");

      // 再接続試行回数が閾値を超えた場合は再起動
      if (wifi_disconnect_count >= WIFI_RECONNECT_THRESHOLD)
      {
        Serial.printf("WiFi reconnect threshold reached (%lu). Rebooting...\n",
                      WIFI_RECONNECT_THRESHOLD);
        ESP.restart();
      }

      WiFi.reconnect();
      setLEDState(LED_WIFI_ERROR);
    }
    else if (!wifi_connected)
    {
      wifi_connected = true;
      Serial.println("WiFi reconnected");
      setLEDState(LED_NORMAL);
    }

    last_wifi_check = current_time;
  }

  // MQTT接続チェック
  if (current_time - last_mqtt_check >= 10000) // 10秒毎
  {
    if (!mqtt_client.connected())
    {
      mqtt_disconnect_count++;

      Serial.println("MQTT disconnected - attempting reconnection");

      // 再接続試行回数が閾値を超えた場合は再起動
      if (mqtt_disconnect_count >= MQTT_RECONNECT_THRESHOLD)
      {
        Serial.printf("MQTT reconnect threshold reached (%lu). Rebooting...\n",
                      MQTT_RECONNECT_THRESHOLD);
        ESP.restart();
      }

      connectToMQTT();
      setLEDState(LED_MQTT_ERROR);
    }

    last_mqtt_check = current_time;
  }
}

// LED状態変更関数
void setLEDState(LEDState new_state)
{
  if (current_led_state != new_state)
  {
    current_led_state = new_state;
    last_led_update = 0; // 即座に更新
    led_blink_state = false;

    // 状態変更をログ出力
    Serial.print("LED state changed to: ");
    switch (new_state)
    {
    case LED_STARTUP:
      Serial.println("STARTUP (Green blink)");
      break;
    case LED_NORMAL:
      Serial.println("NORMAL (Blue solid)");
      break;
    case LED_RAIN:
      Serial.println("RAIN (Purple blink)");
      break;
    case LED_ERROR:
      Serial.println("ERROR (Red blink)");
      break;
    case LED_WIFI_ERROR:
      Serial.println("WIFI_ERROR (Orange blink)");
      break;
    case LED_MQTT_ERROR:
      Serial.println("MQTT_ERROR (Yellow blink)");
      break;
    }
  }
}

// MQTTテスト送信関数
bool testMQTTConnection()
{
  if (!mqtt_client.connected())
  {
    Serial.println("MQTT test: Not connected");
    return false;
  }

  // 簡単なテストメッセージを送信
  String testMessage = "test";
  bool result = mqtt_client.publish(mqtt_topic, testMessage.c_str(), false);

  Serial.print("MQTT test message sent: ");
  Serial.println(result ? "success" : "failed");

  mqtt_client.loop();
  delay(100);

  return result;
}

// 方法1: 静電容量測定（充電時間）
unsigned long measureCapacitanceChargeTime()
{
  unsigned long total_time = 0;
  int valid_count = 0;

  for (int i = 0; i < MEASUREMENT_SAMPLES; i++)
  {
    // 両端子をLOWにして放電
    pinMode(SENSOR_PIN1, OUTPUT);
    pinMode(SENSOR_PIN2, OUTPUT);
    digitalWrite(SENSOR_PIN1, LOW);
    digitalWrite(SENSOR_PIN2, LOW);
    delayMicroseconds(200); // 放電時間を延長

    // PIN1を入力、PIN2をHIGHにして充電開始
    pinMode(SENSOR_PIN1, INPUT);
    digitalWrite(SENSOR_PIN2, HIGH);

    unsigned long start_time = micros();
    unsigned long timeout = 50000; // タイムアウトを50msに延長

    // PIN1がHIGHになるまでの時間を測定
    while (digitalRead(SENSOR_PIN1) == LOW && (micros() - start_time) < timeout)
    {
      // 待機
    }

    unsigned long charge_time = micros() - start_time;

    // より厳しい有効性チェック
    if (charge_time < timeout && charge_time > 10 && charge_time < 45000)
    {
      total_time += charge_time;
      valid_count++;
    }

    // 放電
    digitalWrite(SENSOR_PIN2, LOW);
    delayMicroseconds(100); // 放電時間延長
  }

  return valid_count > 3 ? (total_time / valid_count) : 0; // 必要な有効測定数を緩和
}

// 方法2: 逆方向測定
unsigned long measureCapacitanceReverse()
{
  unsigned long total_time = 0;
  int valid_count = 0;

  for (int i = 0; i < MEASUREMENT_SAMPLES; i++)
  {
    // 両端子をLOWにして放電
    pinMode(SENSOR_PIN1, OUTPUT);
    pinMode(SENSOR_PIN2, OUTPUT);
    digitalWrite(SENSOR_PIN1, LOW);
    digitalWrite(SENSOR_PIN2, LOW);
    delayMicroseconds(200);

    // PIN2を入力、PIN1をHIGHにして充電開始
    pinMode(SENSOR_PIN2, INPUT);
    digitalWrite(SENSOR_PIN1, HIGH);

    unsigned long start_time = micros();
    unsigned long timeout = 50000;

    while (digitalRead(SENSOR_PIN2) == LOW && (micros() - start_time) < timeout)
    {
      // 待機
    }

    unsigned long charge_time = micros() - start_time;

    if (charge_time < timeout && charge_time > 10 && charge_time < 45000)
    {
      total_time += charge_time;
      valid_count++;
    }

    // 放電
    digitalWrite(SENSOR_PIN1, LOW);
    delayMicroseconds(100);
  }

  return valid_count > 3 ? (total_time / valid_count) : 0;
}

// 方法3: 発振検出
unsigned long measureOscillation()
{
  // PIN1を出力、PIN2を入力に設定
  pinMode(SENSOR_PIN1, OUTPUT);
  pinMode(SENSOR_PIN2, INPUT);

  unsigned long pulse_count = 0;
  unsigned long start_time = millis();
  unsigned long end_time = start_time + 500; // 0.5秒測定

  int last_state = digitalRead(SENSOR_PIN2);

  while (millis() < end_time)
  {
    // PIN1をトグルして発振を誘起
    digitalWrite(SENSOR_PIN1, HIGH);
    delayMicroseconds(10);
    digitalWrite(SENSOR_PIN1, LOW);
    delayMicroseconds(10);

    // PIN2の状態変化をカウント
    int current_state = digitalRead(SENSOR_PIN2);
    if (current_state != last_state && current_state == HIGH)
    {
      pulse_count++;
    }
    last_state = current_state;
  }

  return pulse_count * 2; // 0.5秒なので2倍して1秒あたりに換算
}

// 方法4: アナログ差分測定
unsigned long measureAnalogDifference()
{
  // PIN1をVCC、PIN2をアナログ入力として設定
  pinMode(SENSOR_PIN1, OUTPUT);
  digitalWrite(SENSOR_PIN1, HIGH);

  // 安定化のため少し待機
  delay(10);

  float sum = 0;
  int valid_readings = 0;
  int low_readings = 0;   // 異常に低い値のカウント
  int high_readings = 0;  // 異常に高い値のカウント
  float variance_sum = 0; // 分散計算用

  for (int i = 0; i < 100; i++) // サンプル数を増やして安定化
  {
    int reading = analogRead(SENSOR_PIN2);

    // 基本的な範囲チェック
    if (reading >= 0 && reading <= 4095)
    {
      sum += reading;
      valid_readings++;

      // ケーブル脱落の兆候をチェック
      if (reading < 100) // 異常に低い値（ケーブル脱落の可能性）
      {
        low_readings++;
      }
      else if (reading > 3900) // 異常に高い値（オープン回路の可能性）
      {
        high_readings++;
      }
    }

    delay(1); // 短い間隔で測定
  }

  // 測定後にPINをフローティングにして影響を最小化
  pinMode(SENSOR_PIN1, INPUT);

  // ケーブル脱落検知ロジック
  if (valid_readings > 50)
  {
    float average = sum / valid_readings;

    // 1. 異常に低い値が多い場合（50%以上）
    if (low_readings > 50)
    {
      Serial.print("Cable error detected: Too many low readings (");
      Serial.print(low_readings);
      Serial.println("/100)");
      return 0; // エラーとして返す
    }

    // 2. 異常に高い値が多い場合（オープン回路）
    if (high_readings > 50)
    {
      Serial.print("Cable error detected: Too many high readings (");
      Serial.print(high_readings);
      Serial.println("/100)");
      return 0; // エラーとして返す
    }

    // 3. ベースラインと比較して極端に低い場合（キャリブレーション後のみ）
    if (baseline_calibrated && baseline_value > 0)
    {
      float baseline_deviation = ((float)baseline_value - average) / baseline_value * 100.0;
      if (baseline_deviation > 50.0) // ベースラインから50%以上低下
      {
        Serial.print("Cable error detected: Extreme baseline deviation (");
        Serial.print(baseline_deviation, 1);
        Serial.println("%)");
        return 0; // エラーとして返す
      }
    }

    return (unsigned long)average;
  }
  else
  {
    Serial.print("Cable error detected: Insufficient valid readings (");
    Serial.print(valid_readings);
    Serial.println("/100)");
    return 0; // 有効な読み取りが少なすぎる
  }
}

// UNIX時刻取得関数
unsigned long getUnixTimestamp()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    return 0; // 時刻取得失敗
  }
  return mktime(&timeinfo);
}

// ケーブル接続状態チェック関数
void checkCableConnection()
{
  // 連続測定エラーでケーブル脱落を判定
  if (consecutive_measurement_errors >= CABLE_ERROR_THRESHOLD)
  {
    if (cable_connection_ok)
    {
      cable_connection_ok = false;
      Serial.println("CABLE DISCONNECTION DETECTED!");
      Serial.print("Consecutive measurement errors: ");
      Serial.println(consecutive_measurement_errors);
    }

    // ケーブル脱落中で、まだ通知を送っていない場合の処理
    if (!cable_error_sent)
    {
      // クールダウンチェック（独立したケーブル専用クールダウン）
      unsigned long current_time = millis();
      unsigned long cooldown_period = DEBUG_SHORT_COOLDOWN ? 60000UL : (PUSHOVER_COOLDOWN_HOURS * 3600000UL);
      unsigned long time_since_last = current_time - last_cable_notification;

      Serial.println("=== CABLE ERROR NOTIFICATION CHECK ===");
      Serial.print("Cable disconnected: ");
      Serial.println(cable_connection_ok ? "NO" : "YES");
      Serial.print("Cable notification already sent: ");
      Serial.println(cable_error_sent ? "YES" : "NO");
      Serial.print("Time since last cable notification: ");
      Serial.print(time_since_last / 1000);
      Serial.print(" sec (required: ");
      Serial.print(cooldown_period / 1000);
      Serial.println(" sec)");

      bool cooldown_ok = (time_since_last >= cooldown_period);
      bool time_ok = DEBUG_IGNORE_TIME_LIMITS ? true : isNotificationTime();
      bool wifi_ok = wifi_connected;

      Serial.print("Cooldown check: ");
      Serial.println(cooldown_ok ? "PASS" : "FAIL");
      Serial.print("Time window check: ");
      Serial.println(time_ok ? "PASS" : "FAIL");
      Serial.print("WiFi check: ");
      Serial.println(wifi_ok ? "PASS" : "FAIL");

      if (cooldown_ok && time_ok && wifi_ok)
      {
        Serial.println("*** ALL CONDITIONS MET - SENDING CABLE ERROR NOTIFICATION ***");
        String message = "Cable disconnection detected! Sensor PIN1 or PIN2 may be loose. Check connections. Error count: " + String(consecutive_measurement_errors);
        sendPushoverNotification(message, "Cable Error");
        last_cable_notification = current_time;
        cable_error_sent = true;
        Serial.println("*** CABLE ERROR NOTIFICATION SENT ***");
      }
      else
      {
        Serial.println("Cable error notification blocked by conditions above");
      }
      Serial.println("=== END CABLE ERROR CHECK ===");
    }
    else
    {
      // 定期的にケーブル脱落状態を報告（30秒ごと）
      static unsigned long last_cable_status_log = 0;
      if (millis() - last_cable_status_log > 30000)
      {
        Serial.print("Cable still disconnected (errors: ");
        Serial.print(consecutive_measurement_errors);
        Serial.println(") - notification already sent");
        last_cable_status_log = millis();
      }
    }
  }
  else if (consecutive_measurement_errors == 0 && !cable_connection_ok)
  {
    // 測定が復旧したらケーブル接続OK
    cable_connection_ok = true;
    cable_error_sent = false;
    Serial.println("*** CABLE CONNECTION RESTORED - notification flag reset ***");
  }
}

// WiFi接続関数
void setupWiFi()
{
  Serial.print("Connecting to WiFi SSID: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20)
  {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    wifi_connected = true;
    Serial.println();
    Serial.print("WiFi connected successfully");
    Serial.print(" - IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");

    // NTP同期
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    Serial.print("NTP time sync initiated with server: ");
    Serial.println(ntpServer);

    // NTP同期確認（最大10秒待機）
    Serial.print("Waiting for NTP sync");
    int ntp_attempts = 0;
    while (ntp_attempts < 20)
    {
      struct tm timeinfo;
      if (getLocalTime(&timeinfo))
      {
        Serial.println(" - SUCCESS");
        Serial.print("Current time: ");
        Serial.print(timeinfo.tm_year + 1900);
        Serial.print("/");
        Serial.print(timeinfo.tm_mon + 1);
        Serial.print("/");
        Serial.print(timeinfo.tm_mday);
        Serial.print(" ");
        Serial.print(timeinfo.tm_hour);
        Serial.print(":");
        Serial.print(timeinfo.tm_min);
        Serial.print(":");
        Serial.println(timeinfo.tm_sec);
        break;
      }
      Serial.print(".");
      delay(500);
      ntp_attempts++;
    }

    if (ntp_attempts >= 20)
    {
      Serial.println(" - FAILED");
      Serial.println("WARNING: NTP sync failed, time-based functions may not work correctly");
    }
  }
  else
  {
    wifi_connected = false;
    Serial.println();
    Serial.print("WiFi connection failed - Status: ");
    Serial.println(WiFi.status());

    // WiFiステータスの説明
    switch (WiFi.status())
    {
    case WL_IDLE_STATUS:
      Serial.println("WiFi idle");
      break;
    case WL_NO_SSID_AVAIL:
      Serial.println("SSID not available");
      break;
    case WL_SCAN_COMPLETED:
      Serial.println("Scan completed");
      break;
    case WL_CONNECT_FAILED:
      Serial.println("Connection failed");
      break;
    case WL_CONNECTION_LOST:
      Serial.println("Connection lost");
      break;
    case WL_DISCONNECTED:
      Serial.println("Disconnected");
      break;
    default:
      Serial.println("Unknown status");
      break;
    }
  }
}

// MQTT接続関数
void setupMQTT()
{
  if (!wifi_connected)
  {
    Serial.println("Cannot setup MQTT: WiFi not connected");
    return;
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Cannot setup MQTT: WiFi disconnected");
    wifi_connected = false;
    return;
  }

  // MQTTバッファ設定
  setupMQTTBuffers();

  mqtt_client.setServer(mqtt_server, mqtt_port);

  Serial.print("Connecting to MQTT broker: ");
  Serial.print(mqtt_server);
  Serial.print(":");
  Serial.println(mqtt_port);

  int attempts = 0;
  while (!mqtt_client.connected() && attempts < 5)
  {
    Serial.print("MQTT connection attempt ");
    Serial.print(attempts + 1);
    Serial.print("/5 with client ID: ");
    Serial.println(mqtt_client_id);

    if (mqtt_client.connect(mqtt_client_id))
    {
      Serial.println("MQTT connected successfully");
      Serial.print("Using topic: ");
      Serial.println(mqtt_topic);
      Serial.print("Buffer size: ");
      Serial.println(mqtt_client.getBufferSize());
    }
    else
    {
      Serial.print("MQTT connection failed, rc=");
      Serial.print(mqtt_client.state());
      Serial.print(" (");

      // MQTTエラーコードの説明
      switch (mqtt_client.state())
      {
      case -4:
        Serial.print("Connection timeout");
        break;
      case -3:
        Serial.print("Connection lost");
        break;
      case -2:
        Serial.print("Connect failed");
        break;
      case -1:
        Serial.print("Disconnected");
        break;
      case 1:
        Serial.print("Bad protocol version");
        break;
      case 2:
        Serial.print("Bad client ID");
        break;
      case 3:
        Serial.print("Server unavailable");
        break;
      case 4:
        Serial.print("Bad credentials");
        break;
      case 5:
        Serial.print("Not authorized");
        break;
      default:
        Serial.print("Unknown error");
        break;
      }
      Serial.println(")");

      delay(2000);
      attempts++;
    }
  }

  if (!mqtt_client.connected())
  {
    Serial.println("Failed to connect to MQTT broker after 5 attempts");
  }
}

// MQTT再接続関数（堅牢運用用）
void connectToMQTT()
{
  if (!wifi_connected || WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Cannot connect to MQTT: WiFi not connected");
    return;
  }

  if (mqtt_client.connected())
  {
    Serial.println("MQTT already connected");
    return;
  }

  Serial.print("Reconnecting to MQTT broker: ");
  Serial.print(mqtt_server);
  Serial.print(":");
  Serial.println(mqtt_port);

  // 単一の再接続試行
  if (mqtt_client.connect(mqtt_client_id))
  {
    Serial.println("MQTT reconnected successfully");
  }
  else
  {
    Serial.print("MQTT reconnection failed, rc=");
    Serial.println(mqtt_client.state());
  }
}

// MQTT送信関数
void sendMQTTData()
{
  // WiFi接続状態チェック
  if (!wifi_connected)
  {
    Serial.println("MQTT send failed: WiFi not connected");
    return;
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("MQTT send failed: WiFi disconnected");
    wifi_connected = false;
    return;
  }

  // MQTT接続の実際の状態を確認するため、loopを先に実行
  mqtt_client.loop();
  delay(10);

  // MQTT接続チェック
  if (!mqtt_client.connected())
  {
    Serial.print("MQTT not connected (state: ");
    Serial.print(mqtt_client.state());
    Serial.print("), attempting reconnection... ");
    setupMQTT();
    if (!mqtt_client.connected())
    {
      Serial.println("MQTT reconnection failed");
      return;
    }
    Serial.println("MQTT reconnected successfully");
  }

  // 接続確認のための追加チェック
  Serial.print("MQTT connection status: ");
  Serial.print(mqtt_client.connected() ? "Connected" : "Disconnected");
  Serial.print(" (state: ");
  Serial.print(mqtt_client.state());
  Serial.println(")");

  // JSONデータ作成（より小さなペイロード）
  DynamicJsonDocument doc(512); // サイズを小さく
  doc["id"] = mqtt_client_id;
  doc["baseline"] = baseline_value;
  doc["current"] = current_value;
  doc["change"] = round(filtered_change_percent * 100) / 100.0; // 小数点以下2桁に制限
  doc["rain"] = is_raining;
  doc["method"] = selected_method;
  doc["uptime"] = round((millis() - startup_time) / 3600000.0 * 100) / 100.0;
  doc["cable_ok"] = cable_connection_ok;
  doc["errors"] = consecutive_measurement_errors;
  doc["timestamp"] = getUnixTimestamp();

  String jsonString;
  serializeJson(doc, jsonString);

  Serial.print("JSON payload (");
  Serial.print(jsonString.length());
  Serial.print(" bytes): ");
  Serial.println(jsonString);

  // Retain=falseで送信、QoS=0
  bool sent = mqtt_client.publish(mqtt_topic, jsonString.c_str(), false);
  Serial.print("MQTT publish result: ");
  Serial.println(sent ? "success" : "failed");

  if (!sent)
  {
    Serial.print("MQTT client state after failed publish: ");
    Serial.println(mqtt_client.state());
    Serial.print("WiFi status: ");
    Serial.println(WiFi.status());

    // 接続をリセットして再試行
    Serial.println("Attempting MQTT reconnection...");
    mqtt_client.disconnect();
    delay(1000);
    setupMQTT();

    if (mqtt_client.connected())
    {
      Serial.println("Retrying publish after reconnection...");
      mqtt_client.loop();
      delay(10);
      bool retry_sent = mqtt_client.publish(mqtt_topic, jsonString.c_str(), false);
      Serial.print("Retry publish result: ");
      Serial.println(retry_sent ? "success" : "failed");
    }
  }

  mqtt_client.loop();
}

// 時刻チェック関数
bool isNotificationTime()
{
  if (DEBUG_IGNORE_TIME_LIMITS)
  {
    Serial.println("DEBUG: Ignoring time limits for notification");
    return true;
  }

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("WARNING: Failed to get time from NTP, allowing notification");
    return true; // 時刻取得失敗時はデフォルトで通知許可
  }

  int hour = timeinfo.tm_hour;
  bool result = (hour >= PUSHOVER_START_HOUR && hour < PUSHOVER_END_HOUR);

  Serial.print("Time check: ");
  Serial.print(hour);
  Serial.print(":xx (");
  Serial.print(PUSHOVER_START_HOUR);
  Serial.print("-");
  Serial.print(PUSHOVER_END_HOUR);
  Serial.print(") -> ");
  Serial.println(result ? "ALLOW" : "BLOCK");

  return result;
}

// バッテリー警告時刻チェック
bool isBatteryWarningTime()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    return true; // 時刻取得失敗時は通知
  }

  int hour = timeinfo.tm_hour;
  int minute = timeinfo.tm_min;

  // 19:00-翌3:00の静穏時間中かチェック
  bool inQuietTime = (hour >= QUIET_START_HOUR || hour < QUIET_END_HOUR);

  if (inQuietTime)
  {
    // 3:01以降なら通知OK
    return (hour >= QUIET_END_HOUR && minute >= DELAY_AFTER_QUIET);
  }

  return true; // 静穏時間外なら通知OK
}

// PUSHOVER通知関数
void sendPushoverNotification(const String &message, const String &title = "Rain Sensor")
{
  Serial.print("PUSHOVER: Attempting to send notification: ");
  Serial.println(title);

  if (!wifi_connected)
  {
    Serial.println("PUSHOVER: Failed - WiFi not connected");
    return;
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("PUSHOVER: Failed - WiFi disconnected");
    return;
  }

  Serial.println("PUSHOVER: WiFi OK, sending HTTP request...");

  HTTPClient http;
  http.begin("https://api.pushover.net/1/messages.json");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  // メッセージをURLエンコード
  String encodedMessage = urlEncode(message);
  String encodedTitle = urlEncode(title);

  String postData = "token=" + String(pushover_api_token) +
                    "&user=" + String(pushover_user_key) +
                    "&title=" + encodedTitle +
                    "&message=" + encodedMessage;

  Serial.print("PUSHOVER: Payload length: ");
  Serial.println(postData.length());
  Serial.print("PUSHOVER: Full payload: ");
  Serial.println(postData);
  Serial.print("PUSHOVER: API token length: ");
  Serial.println(String(pushover_api_token).length());
  Serial.print("PUSHOVER: User key length: ");
  Serial.println(String(pushover_user_key).length());

  int httpResponseCode = http.POST(postData);

  Serial.print("PUSHOVER notification result: ");
  Serial.print(title);
  Serial.print(" -> HTTP ");
  Serial.print(httpResponseCode);

  if (httpResponseCode == 200)
  {
    Serial.println(" (SUCCESS)");
    String response = http.getString();
    Serial.print("PUSHOVER response: ");
    Serial.println(response);
  }
  else
  {
    Serial.print(" (ERROR: ");
    String errorResponse = http.getString();
    Serial.print(errorResponse);
    Serial.println(")");
    Serial.print("PUSHOVER error details - HTTP code: ");
    Serial.print(httpResponseCode);
    Serial.print(", Response: ");
    Serial.println(errorResponse);
  }

  http.end();
}

void setup()
{
  M5.begin();
  Serial.begin(115200);

  // ===== 24/7 堅牢運用のための初期化 =====

  // ソフトウェアウォッチドッグ初期化
  last_watchdog_feed = millis();

  // システム開始時刻を記録
  system_start_time = millis();
  last_reboot_time = millis();
  last_memory_check = millis();
  last_health_report = millis();

  Serial.println("24/7 robust operation initialized:");
  Serial.printf("- Software watchdog timeout: %d seconds\n", WATCHDOG_TIMEOUT_SECONDS);
  Serial.printf("- Scheduled reboot interval: %lu hours\n",
                SCHEDULED_REBOOT_INTERVAL / 3600000);
  Serial.printf("- Memory warning threshold: %u bytes\n",
                MEMORY_WARNING_THRESHOLD);

  // ===== 標準初期化 =====

  // FASTLED初期化
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(LED_BRIGHTNESS);
  FastLED.clear();
  FastLED.show();

  // 起動中LED状態に設定
  setLEDState(LED_STARTUP);

  Serial.println("AtomS3Lite 2-Terminal Rain Sensor - Production Mode");
  Serial.println("With MQTT, PUSHOVER, Battery monitoring and LED status");

  startup_time = millis();

  // 各通知タイプのクールダウンタイマーと送信フラグを初期化
  // 起動直後でも通知が送れるように
  last_rain_notification = 0;
  last_battery_notification = 0;
  last_cable_notification = 0;
  rain_notification_sent = false;
  battery_warning_sent = false;
  cable_error_sent = false;

  Serial.println("=== PRODUCTION MODE INITIALIZED ===");
  Serial.println("All notification systems ready");
  Serial.print("Cooldown period: ");
  Serial.print(PUSHOVER_COOLDOWN_HOURS);
  Serial.println(" hours");
  Serial.print("Notification time window: ");
  Serial.print(PUSHOVER_START_HOUR);
  Serial.print(":00 - ");
  Serial.print(PUSHOVER_END_HOUR);
  Serial.println(":00");
  Serial.println("=== INDEPENDENT NOTIFICATION TIMERS ===");
  Serial.println("- Rain notifications: Independent 3-hour cooldown (07:00-19:00)");
  Serial.println("- Battery warnings: Independent 3-hour cooldown (avoid 19:00-03:00)");
  Serial.println("- Cable errors: Independent 3-hour cooldown (07:00-19:00)");
  Serial.println("====================================");

  // WiFi接続
  setupWiFi();

  // WiFi接続
  setupWiFi();

  // MQTT接続
  if (wifi_connected)
  {
    setupMQTT();

    // MQTT接続テスト
    if (mqtt_client.connected())
    {
      Serial.println("Testing MQTT connection...");
      testMQTTConnection();
    }
  }

  delay(2000);
  Serial.println("Testing sensor with multiple measurement methods...");
  Serial.println("Please ensure sensor is completely dry during calibration.");

  delay(2000);

  // 各測定方法をテスト
  Serial.println("\n=== Testing measurement methods ===");

  // 方法1: 充電時間測定
  Serial.println("Method 1: Capacitance charge time (PIN1->PIN2)");
  unsigned long cap1_sum = 0;
  int cap1_valid = 0;

  for (int i = 0; i < BASELINE_SAMPLES; i++)
  {
    updateLEDStatus(); // LED更新
    unsigned long value = measureCapacitanceChargeTime();
    Serial.print("  Test ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(value);
    Serial.println(" μs");

    if (value > 0 && value < 45000) // 範囲を拡大
    {
      cap1_sum += value;
      cap1_valid++;
    }
    delay(100);
  }

  // 方法2: 逆方向充電時間測定
  Serial.println("Method 2: Capacitance charge time (PIN2->PIN1)");
  unsigned long cap2_sum = 0;
  int cap2_valid = 0;

  for (int i = 0; i < BASELINE_SAMPLES; i++)
  {
    updateLEDStatus(); // LED更新
    unsigned long value = measureCapacitanceReverse();
    Serial.print("  Test ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(value);
    Serial.println(" μs");

    if (value > 0 && value < 45000) // 範囲を拡大
    {
      cap2_sum += value;
      cap2_valid++;
    }
    delay(100);
  }

  // 方法3: 発振測定
  Serial.println("Method 3: Oscillation detection");
  unsigned long osc_sum = 0;
  int osc_valid = 0;

  for (int i = 0; i < 5; i++)
  {
    updateLEDStatus(); // LED更新
    unsigned long value = measureOscillation();
    Serial.print("  Test ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(value);
    Serial.println(" pulses/sec");

    if (value > 10 && value < 50000)
    {
      osc_sum += value;
      osc_valid++;
    }
    delay(200);
  }

  // 方法4: アナログ測定
  Serial.println("Method 4: Analog difference");
  unsigned long analog_sum = 0;
  int analog_valid = 0;

  for (int i = 0; i < 10; i++)
  {
    updateLEDStatus(); // LED更新
    unsigned long value = measureAnalogDifference();
    Serial.print("  Test ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.println(value);

    if (value > 10 && value < 4000)
    {
      analog_sum += value;
      analog_valid++;
    }
    delay(100);
  }

  // 最も安定した方法を選択
  Serial.println("\n=== Selecting best method ===");

  // より適切な判定基準でMethod 4（アナログ）を優先
  if (analog_valid >= 5 && analog_sum / analog_valid > 100) // アナログ測定を優先
  {
    baseline_value = analog_sum / analog_valid;
    selected_method = 4;
    Serial.print("Selected Method 4 (Analog). Baseline: ");
    Serial.println(baseline_value);
  }
  else if (cap1_valid >= 5 && cap1_sum / cap1_valid > 50) // 有効なベースライン値
  {
    baseline_value = cap1_sum / cap1_valid;
    selected_method = 1;
    Serial.print("Selected Method 1 (Charge Time). Baseline: ");
    Serial.print(baseline_value);
    Serial.println(" μs");
  }
  else if (cap2_valid >= 5 && cap2_sum / cap2_valid > 50)
  {
    baseline_value = cap2_sum / cap2_valid;
    selected_method = 2;
    Serial.print("Selected Method 2 (Reverse Charge). Baseline: ");
    Serial.print(baseline_value);
    Serial.println(" μs");
  }
  else if (osc_valid >= 3 && osc_sum / osc_valid > 100)
  {
    baseline_value = osc_sum / osc_valid;
    selected_method = 3;
    Serial.print("Selected Method 3 (Oscillation). Baseline: ");
    Serial.print(baseline_value);
    Serial.println(" pulses/sec");
  }
  else
  {
    Serial.println("ERROR: No valid measurement method found!");
    Serial.println("Please check sensor connections and try again.");

    // フォールバックでアナログ測定を使用
    if (analog_valid > 0)
    {
      baseline_value = analog_sum / analog_valid;
      selected_method = 4;
      Serial.print("Fallback to Method 4 (Analog). Baseline: ");
      Serial.println(baseline_value);
    }
    else
    {
      baseline_value = 1000; // 最終フォールバック
      selected_method = 4;
      Serial.println("Using default baseline for Method 4");
    }
  }

  Serial.println("\nCalibration complete. Starting rain detection...");

  // ベースライン測定直後の実測値確認
  Serial.println("\n=== Verifying baseline stability ===");
  unsigned long verification_sum = 0;
  int verification_count = 0;

  for (int i = 0; i < 5; i++)
  {
    updateLEDStatus(); // LED更新
    unsigned long test_value = 0;
    switch (selected_method)
    {
    case 1:
      test_value = measureCapacitanceChargeTime();
      break;
    case 2:
      test_value = measureCapacitanceReverse();
      break;
    case 3:
      test_value = measureOscillation();
      break;
    case 4:
      test_value = measureAnalogDifference();
      break;
    }

    Serial.print("Verification ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.println(test_value);

    if (test_value > 0)
    {
      verification_sum += test_value;
      verification_count++;
    }
    delay(500);
  }

  if (verification_count > 0)
  {
    unsigned long avg_verification = verification_sum / verification_count;
    float baseline_drift = abs((long)avg_verification - (long)baseline_value);
    float drift_percent = (baseline_drift / baseline_value) * 100.0;

    Serial.print("Average verification: ");
    Serial.println(avg_verification);
    Serial.print("Baseline drift: ");
    Serial.print(drift_percent, 2);
    Serial.println("%");

    // ドリフトが大きい場合はベースラインを調整
    if (drift_percent > 10.0)
    {
      Serial.println("Large baseline drift detected - adjusting baseline");
      baseline_value = avg_verification;
      Serial.print("New baseline: ");
      Serial.println(baseline_value);
    }
  }

  baseline_calibrated = true;
  last_baseline_update = millis();

  // キャリブレーション完了 - 正常動作状態に移行
  setLEDState(LED_NORMAL);
  Serial.println("=== Calibration complete - System ready for production ===");
  Serial.println("*** RAIN SENSOR SYSTEM OPERATIONAL ***");
  Serial.print("Selected measurement method: ");
  Serial.println(selected_method);
  Serial.print("Baseline value: ");
  Serial.println(baseline_value);
  Serial.print("Rain detection threshold: ");
  Serial.print(RAIN_THRESHOLD_PERCENT);
  Serial.println("%");
  Serial.print("Cable error threshold: ");
  Serial.print(CABLE_ERROR_THRESHOLD);
  Serial.println(" consecutive errors");
  Serial.println("System monitoring started...");
  Serial.println("==========================================");

  M5.Lcd.fillScreen(BLACK);
  delay(1000);
}

void loop()
{
  // ===== 24/7 堅牢運用のための監視処理 =====

  // ウォッチドッグフィード
  feedWatchdog();

  // ウォッチドッグタイムアウトチェック
  checkWatchdogTimeout();

  // 接続監視
  monitorConnections();

  // 定期再起動チェック
  checkScheduledReboot();

  // メモリチェック（5分間隔）
  static unsigned long last_memory_check_time = 0;
  if (millis() - last_memory_check_time >= MEMORY_CHECK_INTERVAL)
  {
    checkMemoryHealth();
    last_memory_check_time = millis();
  }

  // システムヘルスレポート送信（1時間間隔）
  if (millis() - last_health_report >= HEALTH_REPORT_INTERVAL)
  {
    sendSystemHealthReport();
  }

  // ===== 標準センサー処理 =====

  // LED状態更新（常時）
  updateLEDStatus();

  // 選択された測定方法で現在値を取得
  switch (selected_method)
  {
  case 1:
    current_value = measureCapacitanceChargeTime();
    break;
  case 2:
    current_value = measureCapacitanceReverse();
    break;
  case 3:
    current_value = measureOscillation();
    break;
  case 4:
    current_value = measureAnalogDifference();
    break;
  default:
    current_value = measureCapacitanceChargeTime();
    break;
  }

  // 測定エラーチェック
  static unsigned long previous_value = 0;
  static int unstable_measurement_count = 0;

  if (current_value == 0)
  {
    consecutive_measurement_errors++;
    Serial.print("Measurement error (method ");
    Serial.print(selected_method);
    Serial.print(") - consecutive errors: ");
    Serial.println(consecutive_measurement_errors);

    // エラー状態LEDに変更
    if (consecutive_measurement_errors >= CABLE_ERROR_THRESHOLD)
    {
      setLEDState(LED_ERROR);
    }

    // 前回の値を保持（ただし、初回は処理をスキップ）
    static unsigned long last_valid_value = 0;
    if (last_valid_value > 0)
    {
      current_value = last_valid_value;
    }
    else
    {
      // ケーブル接続チェック
      checkCableConnection();
      delay(1000);
      return;
    }
  }
  else
  {
    // 追加の異常検知：急激な値の変化をチェック
    if (baseline_calibrated && previous_value > 0)
    {
      float value_change_percent = abs((long)current_value - (long)previous_value) * 100.0 / previous_value;

      if (value_change_percent > 30.0) // 前回から30%以上変化
      {
        unstable_measurement_count++;
        Serial.print("Unstable measurement detected: ");
        Serial.print(value_change_percent, 1);
        Serial.print("% change (count: ");
        Serial.print(unstable_measurement_count);
        Serial.println(")");

        if (unstable_measurement_count >= 3) // 3回連続で不安定
        {
          Serial.println("Cable connection may be unstable - treating as measurement error");
          consecutive_measurement_errors++;
          unstable_measurement_count = 0; // リセット
        }
      }
      else
      {
        unstable_measurement_count = 0; // 安定した測定なのでリセット
      }
    }

    // 正常測定時はエラーカウンターをリセット
    if (consecutive_measurement_errors > 0 && unstable_measurement_count == 0)
    {
      consecutive_measurement_errors = 0;
      // エラーから回復した場合、LED状態を適切に設定
      if (is_raining)
      {
        setLEDState(LED_RAIN);
      }
      else
      {
        setLEDState(LED_NORMAL);
      }
    }

    // 有効な測定値を記録
    static unsigned long last_valid_value = current_value;
    last_valid_value = current_value;
    previous_value = current_value; // 次回の比較用に保存
  }

  // ケーブル接続状態チェック
  checkCableConnection();

  // 変化率計算
  float change_percent = 0;
  if (baseline_value > 0 && current_value > 0)
  {
    long diff = (long)current_value - (long)baseline_value;
    change_percent = ((float)diff / baseline_value) * 100.0;
  }

  // ノイズフィルター
  if (abs(change_percent) < NOISE_THRESHOLD)
  {
    change_percent = 0;
  }

  // ローパスフィルター
  filtered_change_percent = LOWPASS_ALPHA * filtered_change_percent + (1.0 - LOWPASS_ALPHA) * change_percent;

  // 動的ベースライン調整（雨が検出されていない場合のみ）
  if (!is_raining && baseline_calibrated &&
      (millis() - last_baseline_update) > BASELINE_UPDATE_INTERVAL)
  {

    float avg_change = abs(filtered_change_percent);

    // 継続的な小さな変化はベースラインのドリフトと判断
    if (avg_change > 5.0 && avg_change < BASELINE_UPDATE_THRESHOLD)
    {
      unsigned long new_baseline = baseline_value + (baseline_value * filtered_change_percent / 100.0);

      Serial.print("Updating baseline from ");
      Serial.print(baseline_value);
      Serial.print(" to ");
      Serial.println(new_baseline);

      baseline_value = new_baseline;
      filtered_change_percent = 0; // フィルターをリセット
      last_baseline_update = millis();
    }
  }

  // 雨判定（連続検出による安定化）
  bool rain_detected = abs(filtered_change_percent) > RAIN_THRESHOLD_PERCENT;

  // 追加の安定性チェック：現在値がベースラインから大きく乖離している場合のみ
  bool significant_change = abs((long)current_value - (long)baseline_value) > (baseline_value * 0.1);
  rain_detected = rain_detected && significant_change;

  if (rain_detected)
  {
    rain_detection_count++;
    if (rain_detection_count >= STABILITY_CHECK_COUNT)
    {
      if (!is_raining)
      {
        is_raining = true;
        // 雨検知時のLED状態変更（エラー状態でない場合のみ）
        if (consecutive_measurement_errors < CABLE_ERROR_THRESHOLD)
        {
          setLEDState(LED_RAIN);
        }
      }
    }
  }
  else
  {
    rain_detection_count = 0;
    // 雨判定から通常状態に戻る際のヒステリシス
    if (is_raining && abs(filtered_change_percent) < (RAIN_THRESHOLD_PERCENT * 0.7))
    {
      is_raining = false;
      // 雨停止時のLED状態変更（エラー状態でない場合のみ）
      if (consecutive_measurement_errors < CABLE_ERROR_THRESHOLD)
      {
        setLEDState(LED_NORMAL);
      }
    }
  }

  // デバッグ出力
  Serial.print("Method: ");
  Serial.print(selected_method);
  Serial.print(", Baseline: ");
  Serial.print(baseline_value);
  Serial.print(", Current: ");
  Serial.print(current_value);
  Serial.print(", Change: ");
  Serial.print(change_percent, 2);
  Serial.print("%, Filtered: ");
  Serial.print(filtered_change_percent, 2);
  Serial.print("%, Count: ");
  Serial.print(rain_detection_count);
  Serial.print(", SignificantChange: ");
  Serial.print(significant_change ? "YES" : "NO");
  Serial.print(", Cable: ");
  Serial.print(cable_connection_ok ? "OK" : "ERROR");
  Serial.print(", Errors: ");
  Serial.print(consecutive_measurement_errors);
  Serial.print(", Rain: ");
  Serial.println(is_raining ? "YES" : "NO");

  // MQTT送信（30秒ごと）
  if (wifi_connected && (millis() - last_mqtt_send) >= MQTT_SEND_INTERVAL)
  {
    sendMQTTData();
    last_mqtt_send = millis();
  }

  // PUSHOVER雨検知通知（07:00-19:00のみ、3時間クールダウン）
  if (is_raining)
  {
    Serial.print("Rain status: is_raining=true, notification_sent=");
    Serial.print(rain_notification_sent ? "true" : "false");
    Serial.print(", uptime=");
    Serial.print(millis() / 1000);
    Serial.println(" sec");

    if (!rain_notification_sent)
    {
      Serial.println("Rain detected - checking notification conditions:");

      // 時刻チェック
      bool time_ok = isNotificationTime();
      Serial.print("  Time check (07:00-19:00): ");
      Serial.println(time_ok ? "OK" : "FAILED");

      if (!time_ok)
      {
        // 現在時刻を表示
        struct tm timeinfo;
        if (getLocalTime(&timeinfo))
        {
          Serial.print("  Current time: ");
          Serial.print(timeinfo.tm_hour);
          Serial.print(":");
          Serial.print(timeinfo.tm_min);
          Serial.print(":");
          Serial.println(timeinfo.tm_sec);
        }
        else
        {
          Serial.println("  Failed to get current time from NTP");
        }
      }

      // クールダウンチェック（独立した雨検知専用クールダウン）
      unsigned long current_time = millis();
      unsigned long time_since_last = current_time - last_rain_notification;
      unsigned long cooldown_required = DEBUG_SHORT_COOLDOWN ? 60000UL : (PUSHOVER_COOLDOWN_HOURS * 3600000UL);

      bool cooldown_ok = (time_since_last >= cooldown_required);

      Serial.print("  Rain cooldown check: ");
      Serial.print(time_since_last / 1000);
      Serial.print(" sec since last (required: ");
      Serial.print(cooldown_required / 1000);
      Serial.print(" sec) - ");
      Serial.println(cooldown_ok ? "OK" : "WAITING");

      // WiFi接続チェック
      Serial.print("  WiFi connected: ");
      Serial.println(wifi_connected ? "YES" : "NO");

      if (time_ok && cooldown_ok)
      {
        Serial.println("  All conditions met - sending PUSHOVER notification");
        Serial.println("  *** ABOUT TO CALL sendPushoverNotification() ***");
        String message = "Rain detected! Current: " + String(current_value) +
                         ", Change: " + String(filtered_change_percent, 1) + "%";
        sendPushoverNotification(message, "Rain Alert");
        Serial.println("  *** sendPushoverNotification() COMPLETED ***");
        last_rain_notification = current_time;
        rain_notification_sent = true;
        Serial.print("  rain_notification_sent flag set to: ");
        Serial.println(rain_notification_sent ? "true" : "false");
        Serial.print("  last_rain_notification set to: ");
        Serial.println(last_rain_notification);
      }
      else
      {
        Serial.println("  Notification blocked by conditions above");
      }
    }
    else
    {
      Serial.println("Rain notification already sent - waiting for rain to stop");
    }
  }

  // 雨が止んだらフラグリセット
  if (!is_raining && rain_notification_sent)
  {
    rain_notification_sent = false;
    Serial.println("*** Rain stopped - notification flag reset, ready for next rain detection ***");
    // 雨停止時のLED状態確認（エラー状態でない場合のみ正常状態に）
    if (consecutive_measurement_errors < CABLE_ERROR_THRESHOLD)
    {
      setLEDState(LED_NORMAL);
    }
  }
  else if (!is_raining)
  {
    // 静的変数でログの頻度を制限
    static unsigned long last_no_rain_log = 0;
    if (millis() - last_no_rain_log > 30000) // 30秒ごとにログ
    {
      Serial.println("No rain detected - system ready for rain notification");
      last_no_rain_log = millis();
    }
  }

  // バッテリー残量警告（25時間後、独立した3時間クールダウン）
  unsigned long uptime_hours = (millis() - startup_time) / 3600000;
  if (uptime_hours >= BATTERY_CHECK_HOURS && !battery_warning_sent && isBatteryWarningTime())
  {
    unsigned long current_time = millis();
    unsigned long cooldown_period = DEBUG_SHORT_COOLDOWN ? 60000UL : (PUSHOVER_COOLDOWN_HOURS * 3600000UL);
    unsigned long time_since_last = current_time - last_battery_notification;

    Serial.println("=== BATTERY WARNING CHECK ===");
    Serial.print("Uptime: ");
    Serial.print(uptime_hours);
    Serial.print(" hours (threshold: ");
    Serial.print(BATTERY_CHECK_HOURS);
    Serial.println(" hours)");
    Serial.print("Battery warning already sent: ");
    Serial.println(battery_warning_sent ? "YES" : "NO");
    Serial.print("Time since last battery notification: ");
    Serial.print(time_since_last / 1000);
    Serial.print(" sec (required: ");
    Serial.print(cooldown_period / 1000);
    Serial.println(" sec)");

    bool cooldown_ok = (time_since_last >= cooldown_period);
    bool time_ok = isBatteryWarningTime();
    bool wifi_ok = wifi_connected;

    Serial.print("Battery cooldown check: ");
    Serial.println(cooldown_ok ? "PASS" : "FAIL");
    Serial.print("Battery time window check: ");
    Serial.println(time_ok ? "PASS" : "FAIL");
    Serial.print("WiFi check: ");
    Serial.println(wifi_ok ? "PASS" : "FAIL");

    if (cooldown_ok && time_ok && wifi_ok)
    {
      Serial.println("*** ALL CONDITIONS MET - SENDING BATTERY WARNING ***");
      String message = "Battery may be low after " + String(uptime_hours) + " hours of operation";
      sendPushoverNotification(message, "Battery Warning");
      last_battery_notification = current_time;
      battery_warning_sent = true;
      Serial.println("*** BATTERY WARNING SENT ***");
    }
    else
    {
      Serial.println("Battery warning blocked by conditions above");
    }
    Serial.println("=== END BATTERY WARNING CHECK ===");
  }

  // 表示更新
  if (is_raining)
  {
    M5.Lcd.fillScreen(GREEN);
    Serial.println("*** RAIN DETECTED ***");
  }
  else
  {
    M5.Lcd.fillScreen(BLACK);
  }

  // WiFi接続維持
  if (wifi_connected && WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi disconnected, attempting reconnection...");
    // WiFi切断時はエラー状態に（ただし他のエラーがない場合）
    if (consecutive_measurement_errors < CABLE_ERROR_THRESHOLD)
    {
      setLEDState(LED_ERROR);
    }
    setupWiFi();
    // WiFi復旧時にLED状態を適切に戻す
    if (wifi_connected && consecutive_measurement_errors < CABLE_ERROR_THRESHOLD)
    {
      if (is_raining)
      {
        setLEDState(LED_RAIN);
      }
      else
      {
        setLEDState(LED_NORMAL);
      }
    }
  }

  // MQTT接続維持
  if (wifi_connected && !mqtt_client.connected())
  {
    Serial.println("MQTT disconnected, attempting reconnection...");
    // MQTT切断は通信エラーとしてログのみ（LED状態は変更しない）
    setupMQTT();
  }

  // MQTT loop
  if (mqtt_client.connected())
  {
    mqtt_client.loop();
  }

  // 定期的に通知システムの状態を表示（60秒ごと）
  static unsigned long last_notification_status_log = 0;
  if (millis() - last_notification_status_log > 60000)
  {
    Serial.println("=== NOTIFICATION SYSTEM STATUS ===");
    Serial.print("Rain notification sent: ");
    Serial.print(rain_notification_sent ? "YES" : "NO");
    Serial.print(", Time since last: ");
    Serial.print((millis() - last_rain_notification) / 1000);
    Serial.println(" sec");

    Serial.print("Battery warning sent: ");
    Serial.print(battery_warning_sent ? "YES" : "NO");
    Serial.print(", Time since last: ");
    Serial.print((millis() - last_battery_notification) / 1000);
    Serial.println(" sec");

    Serial.print("Cable error sent: ");
    Serial.print(cable_error_sent ? "YES" : "NO");
    Serial.print(", Time since last: ");
    Serial.print((millis() - last_cable_notification) / 1000);
    Serial.println(" sec");

    Serial.print("Cable connection: ");
    Serial.print(cable_connection_ok ? "OK" : "ERROR");
    Serial.print(", Consecutive errors: ");
    Serial.println(consecutive_measurement_errors);

    unsigned long cooldown_time = DEBUG_SHORT_COOLDOWN ? 60 : (PUSHOVER_COOLDOWN_HOURS * 3600);
    Serial.print("Cooldown period: ");
    Serial.print(cooldown_time);
    Serial.println(" sec");
    Serial.println("===================================");

    last_notification_status_log = millis();
  }

  delay(1000);
}

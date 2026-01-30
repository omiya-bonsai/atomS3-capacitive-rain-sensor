/*
 * config.example.h - 設定ファイルのサンプル
 *
 * このファイルをconfig.hにコピーして、実際の設定値に変更してください。
 *
 * 【重要】
 * ・このファイルは設定例です。実際の値は記載しないでください。
 * ・config.hファイルには機密情報が含まれるため、Gitリポジトリにコミットしないよう注意してください。
 * ・.gitignoreにconfig.hを追加することを強く推奨します。
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// WiFi設定
// ============================================================================
const char *ssid = "your_wifi_ssid";         // WiFiネットワーク名（SSID）
const char *password = "your_wifi_password"; // WiFiパスワード

// ============================================================================
// MQTT設定
// ============================================================================
const char *mqtt_server = "192.168.1.100";     // MQTTブローカーのIPアドレス
const int mqtt_port = 1883;                    // MQTTポート番号（通常1883、TLS使用時は8883）
const char *mqtt_client_id = "rain_sensor_01"; // MQTTクライアントID（ユニークな名前にしてください）
const char *mqtt_topic = "sensors/rain/data";  // MQTTトピック名

// ============================================================================
// デバイス識別設定
// ============================================================================
const char *DEVICE_ID = "rain_sensor_01";            // デバイス識別ID（ヘルスレポートで使用）
const char *LOCATION = "Garden Sensor";              // センサー設置場所名（ヘルスレポートで使用）
const char *MQTT_TOPIC_PREFIX = "sensors/rain/data"; // MQTTトピックプレフィックス（ヘルス用）

// ============================================================================
// PUSHOVER通知設定
// ============================================================================
// PUSHOVERアカウント設定（https://pushover.net/で取得）
const char *pushover_api_token = "your_app_token_here"; // PUSHOVERアプリトークン（30文字）
const char *pushover_user_key = "your_user_key_here";   // PUSHOVERユーザーキー（30文字）

// ============================================================================
// ハードウェア設定
// ============================================================================
#define SENSOR_PIN1 1 // センサー端子1
#define SENSOR_PIN2 2 // センサー端子2

// FASTLED設定
#define LED_PIN 35        // M5AtomS3の内蔵LED（WS2812）
#define NUM_LEDS 1        // LEDの数
#define LED_BRIGHTNESS 50 // LED輝度（0-255）

// ============================================================================
// 検知・フィルタ設定
// ============================================================================
#define MEASUREMENT_SAMPLES 20
#define BASELINE_SAMPLES 15
#define RAIN_THRESHOLD_PERCENT 22.0 // 雨判定しきい値（%）
#define NOISE_THRESHOLD 8.0         // ノイズしきい値（%）
#define LOWPASS_ALPHA 0.8
#define STABILITY_CHECK_COUNT 10       // 連続検知回数（秒相当）
#define BASELINE_UPDATE_INTERVAL 30000 // ベースライン更新間隔（ms）
#define BASELINE_UPDATE_THRESHOLD 10.0 // ベースライン更新の変化率閾値（%）

// ============================================================================
// 通信・通知設定
// ============================================================================
#define MQTT_SEND_INTERVAL 30000      // MQTT送信間隔（ms）
#define BATTERY_CHECK_HOURS 25        // バッテリーチェック時間（時間）
#define PUSHOVER_START_HOUR 7         // PUSHOVER通知開始時刻
#define PUSHOVER_END_HOUR 19          // PUSHOVER通知終了時刻
#define QUIET_START_HOUR 19           // 静穏時間開始
#define QUIET_END_HOUR 3              // 静穏時間終了
#define DELAY_AFTER_QUIET 1           // 静穏時間後の遅延（分）
#define PUSHOVER_COOLDOWN_HOURS 3     // PUSHOVER通知のクールダウン時間（時間）
#define ENABLE_BATTERY_PUSHOVER false // バッテリー通知をPUSHOVERで送信しない

// 夜露・高湿度の時間帯
#define DEW_START_HOUR 3        // 夜露・高湿度が出やすい時間帯開始
#define DEW_END_HOUR 6          // 夜露・高湿度が出やすい時間帯終了
#define DEW_EXTRA_THRESHOLD 5.0 // 夜露時間帯の追加しきい値（%）

// ============================================================================
// エラー・デバッグ設定
// ============================================================================
#define CABLE_ERROR_THRESHOLD 5              // ケーブル脱落判定の連続エラー回数
#define MEASUREMENT_ERROR_THRESHOLD 10       // 測定エラー判定の連続回数
#define DEBUG_IGNORE_TIME_LIMITS false       // デバッグ用：時刻制限を無視する場合はtrue
#define DEBUG_SHORT_COOLDOWN false           // デバッグ用：クールダウンを短縮する場合はtrue（1分）
#define DEBUG_FORCE_RESET_NOTIFICATION false // デバッグ用：毎回通知フラグを強制リセット
#define DEBUG_MODE false                     // デバッグ出力の有効/無効

// ============================================================================
// 24/7運用堅牢性設定
// ============================================================================
#define WATCHDOG_TIMEOUT_SECONDS 120                // ウォッチドッグタイムアウト（秒）
#define SCHEDULED_REBOOT_INTERVAL (168UL * 3600000) // 定期再起動間隔（ms）
#define MEMORY_CHECK_INTERVAL 300000                // メモリチェック間隔（ms）
#define MEMORY_WARNING_THRESHOLD 8192               // メモリ警告閾値（bytes）
#define WIFI_RECONNECT_THRESHOLD 10                 // WiFi再接続エラー閾値
#define MQTT_RECONNECT_THRESHOLD 10                 // MQTT再接続エラー閾値
#define HEALTH_REPORT_INTERVAL 3600000              // ヘルスレポート間隔（ms）

// ============================================================================
// NTP設定
// ============================================================================
const char *ntpServer = "ntp.nict.jp";
const long gmtOffset_sec = 9 * 3600; // JST (UTC+9)
const int daylightOffset_sec = 0;

// ============================================================================
// 設定値の説明
// ============================================================================
/*
 * 【WiFi設定について】
 * ssid: お使いのWiFiルーターのネットワーク名
 * password: WiFiネットワークのパスワード
 *
 * 【MQTT設定について】
 * mqtt_server: MQTTブローカーのIPアドレスまたはホスト名
 *   - ローカル: "192.168.1.100" など
 *   - クラウド: "mqtt.example.com" など
 * mqtt_port: MQTTポート番号
 *   - 標準: 1883
 *   - TLS暗号化: 8883
 * mqtt_client_id: 複数のセンサーを使用する場合はユニークな名前に変更
 *   例: "rain_sensor_garden", "rain_sensor_balcony"
 * mqtt_topic: データを送信するトピック名
 *   例: "home/sensors/rain", "garden/weather/rain"
 *
 * 【PUSHOVER設定について】
 * 1. https://pushover.net/ でアカウントを作成
 * 2. アプリケーションを登録してAPI Tokenを取得
 * 3. User Keyをダッシュボードから確認
 * 4. 両方とも30文字の英数字文字列です
 *
 * 【位置情報設定について】
 * location_name: 通知メッセージに表示される場所の名前
 *   例: "ベランダ", "庭", "温室", "駐車場"
 */

// ============================================================================
// MQTTデータサンプル
// ============================================================================
/*
 * このセンサーが送信するMQTTデータの例:
 *
 * トピック: sensors/rain/data
 * ペイロード（JSON形式）:
 * {
 *   "id": "rain_sensor_01",
 *   "baseline": 1450,
 *   "current": 1523,
 *   "change": 5.03,
 *   "rain": false,
 *   "method": 4,
 *   "uptime": 2.45,
 *   "cable_ok": true,
 *   "errors": 0,
 *   "timestamp": 1722758400
 * }
 *
 * フィールドの説明:
 * - id: センサーID
 * - baseline: ベースライン値（乾燥時の基準値）
 * - current: 現在の測定値
 * - change: ベースラインからの変化率（％）
 * - rain: 雨検知状態（true/false）
 * - method: 使用中の測定方式（1-4）
 * - uptime: 稼働時間（時間）
 * - cable_ok: ケーブル接続状態（true/false）
 * - errors: 連続測定エラー回数
 * - timestamp: UNIXタイムスタンプ（日本時間）
 */

// ============================================================================
// セキュリティに関する注意事項
// ============================================================================
/*
 * 【重要なセキュリティ対策】
 *
 * 1. config.hファイルの管理:
 *    - 実際のパスワードやトークンを含むconfig.hファイルは
 *      Gitリポジトリにコミットしないでください
 *    - .gitignoreに以下を追加: config.h
 *
 * 2. WiFiセキュリティ:
 *    - WPA2/WPA3暗号化を使用してください
 *    - デフォルトのルーターパスワードは変更してください
 *
 * 3. MQTTセキュリティ:
 *    - 可能であればTLS暗号化（ポート8883）を使用してください
 *    - ユーザー名・パスワード認証を設定してください
 *    - ファイアウォールでMQTTポートへのアクセスを制限してください
 *
 * 4. PUSHOVERセキュリティ:
 *    - API TokenとUser Keyは他人と共有しないでください
 *    - 定期的にトークンを再生成することを検討してください
 *
 * 5. デバイスセキュリティ:
 *    - デバイスを物理的に保護してください
 *    - 定期的にファームウェアを更新してください
 */

// ============================================================================
// トラブルシューティング用設定
// ============================================================================
/*
 * 【よくある問題と対処法】
 *
 * WiFi接続できない場合:
 * - SSIDとパスワードが正しいか確認
 * - WiFiルーターの2.4GHz帯が有効か確認（5GHzは非対応）
 * - WiFiルーターとの距離を近づけてテスト
 *
 * MQTT接続できない場合:
 * - ブローカーのIPアドレスとポート番号を確認
 * - ファイアウォール設定を確認
 * - MQTTブローカーが起動しているか確認
 *
 * PUSHOVER通知が来ない場合:
 * - API TokenとUser Keyが正しいか確認
 * - PUSHOVERアプリがスマートフォンにインストールされているか確認
 * - 通知時間帯（07:00-19:00）内か確認
 * - クールダウン時間（3時間）が経過しているか確認
 *
 * 【24/7堅牢運用について】
 * このシステムには以下の堅牢性機能が組み込まれています：
 * - ソフトウェアウォッチドッグ（120秒タイムアウト）
 * - 定期再起動（168時間＝7日間隔）
 * - メモリ監視（最小8KB閾値）
 * - WiFi/MQTT自動再接続（閾値：各10回）
 * - システムヘルスレポート（1時間間隔）
 * - 接続監視とエラーカウント
 *
 * 長期運用時の注意点：
 * - 電源は安定したものを使用してください
 * - WiFiルーターの電源も安定させてください
 * - 月1回程度、動作確認を行ってください
 * - MQTTブローカーとPUSHOVERサービスの稼働状況を確認してください
 */

#endif // CONFIG_H

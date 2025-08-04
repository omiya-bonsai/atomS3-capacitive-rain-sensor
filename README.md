# AtomS3Lite 2端子静電容量式レインセンサー

M5AtomS3を使用した自動雨検知システムです。2本の導線を使った静電容量式センサーで雨を検知し、MQTT通信とPushover通知によってリアルタイムで雨情報をお知らせします。

<img width="501" height="502" alt="スクリーンショット 2025-08-04 11 46 36" src="https://github.com/user-attachments/assets/58dad73d-e72d-491d-b473-0046595ba19a" />


## 📋 目次

- [特徴](#特徴)
- [ハードウェア要件](#ハードウェア要件)
- [セットアップ](#セットアップ)
- [設定](#設定)
- [使用方法](#使用方法)
- [LED状態表示](#led状態表示)
- [トラブルシューティング](#トラブルシューティング)
- [APIリファレンス](#apiリファレンス)
- [ライセンス](#ライセンス)

## ✨ 特徴

### 🌧️ 高精度雨検知
- **4つの測定方式**を自動選択して最適な検知性能を実現
- **動的ベースライン調整**で環境変化に自動適応
- **誤検出防止**：3回連続検出による安定化フィルター
- **ノイズ除去**：ローパスフィルター（α=0.8）とノイズ閾値（5%）

### 📡 IoT通信機能
- **MQTT通信**：30秒間隔でセンサーデータを送信
- **Pushover通知**：スマートフォンへリアルタイム通知
- **時間制御**：07:00-19:00の時間帯のみ通知（近隣配慮）
- **クールダウン**：3時間間隔で通知頻度を制限

### 🔧 エラー検知・監視
- **ケーブル脱落検知**：5回連続エラーで自動検出
- **WiFi接続監視**：切断時の自動再接続
- **バッテリー監視**：25時間稼働後の警告通知
- **NTP時刻同期**：正確な時刻制御

### 💡 視覚的状態表示
- **LED状態表示**：動作状況を色で直感的に表示
  - 🟢 緑点滅：起動中・校正中
  - 🔵 青常灯：正常動作
  - 🟣 紫点滅：雨検知中
  - 🔴 赤点滅：エラー発生

## 🛠️ ハードウェア要件

### 必須コンポーネント
- **M5AtomS3 Lite**（ESP32-S3搭載）
- **静電容量式レインセンサ基板**（[NAOTO-001](https://www.switch-science.com/products/8202) - Switch Science）
- **GROVE/Dupontケーブル**（PIN1/PIN2接続用）
- **防水ケース**（屋外設置用）

### 推奨環境
- **WiFi環境**：2.4GHz帯対応
- **電源**：USB-C または バッテリー（25時間連続動作）
- **設置場所**：雨が直接当たる屋外環境

## 🚀 セットアップ

### 1. 開発環境の準備

```bash
# Arduino IDEまたはPlatformIOを使用
# 必要なライブラリをインストール
```

**必要なライブラリ:**
- M5AtomS3
- WiFi
- PubSubClient
- HTTPClient
- ArduinoJson
- FastLED

### 2. 配線

```
M5AtomS3    センサー
G1      →   Pulse Out
G2      →   Sensor In
```

### 3. ファイル構成

```
rain_sensor/
├── rain_sensor.ino      # メインスケッチ
├── config.h             # 設定ファイル（要作成）
├── config.example.h     # 設定サンプル
└── README.md           # このファイル
```

## ⚙️ 設定

### config.hファイルの作成

`config.example.h`をコピーして`config.h`を作成し、あなたの環境に合わせて設定してください：

```cpp
// WiFi設定
const char* ssid = "あなたのWiFi_SSID";
const char* password = "あなたのWiFiパスワード";

// MQTT設定
const char* mqtt_server = "192.168.1.100";  // MQTTブローカーのIP
const int mqtt_port = 1883;
const char* mqtt_topic = "sensors/rain";
const char* mqtt_client_id = "rain_sensor_01";

// Pushover設定
const char* pushover_api_token = "あなたのAPIトークン";
const char* pushover_user_key = "あなたのユーザーキー";

// 位置情報（オプション）
const char* location_name = "庭先";
```

### 設定パラメータ

| パラメータ | デフォルト値 | 説明 |
|-----------|-------------|------|
| `RAIN_THRESHOLD_PERCENT` | 15.0% | 雨検知の閾値 |
| `NOISE_THRESHOLD` | 5.0% | ノイズ除去の閾値 |
| `STABILITY_CHECK_COUNT` | 3回 | 連続検出回数 |
| `MQTT_SEND_INTERVAL` | 30秒 | MQTT送信間隔 |
| `PUSHOVER_START_HOUR` | 7時 | 通知開始時刻 |
| `PUSHOVER_END_HOUR` | 19時 | 通知終了時刻 |
| `PUSHOVER_COOLDOWN_HOURS` | 3時間 | 通知クールダウン |

## 📱 使用方法

### 1. 初回起動

1. M5AtomS3にスケッチをアップロード
2. センサーを**完全に乾燥した状態**で起動
3. 緑LEDの点滅中に自動校正が実行されます
4. 青LED常灯になれば準備完了

### 2. 動作確認

```
シリアルモニター出力例:
=== Testing measurement methods ===
Method 1: Capacitance charge time (PIN1->PIN2)
Method 4: Analog difference
Selected Method 4 (Analog). Baseline: 1250
=== Calibration complete - System ready ===
```

### 3. 雨検知テスト

- センサーに水を数滴垂らして動作確認
- 紫LEDの点滅と通知が確認できれば正常

## 🔍 LED状態表示

| LED状態 | 動作状況 | 対処法 |
|---------|----------|--------|
| 🟢 緑点滅（200ms） | 起動中・校正中 | しばらく待機 |
| 🔵 青常灯 | 正常動作（雨なし） | 正常 |
| 🟣 紫点滅（500ms） | 雨検知中 | 正常（雨検知） |
| 🔴 赤点滅（300ms） | エラー発生 | 配線・WiFi確認 |

## 📊 MQTTデータ形式

30秒間隔で以下のJSONデータを送信：

```json
{
  "id": "rain_sensor_01",
  "baseline": 1250,
  "current": 1350,
  "change": 8.0,
  "rain": true,
  "method": 4,
  "uptime": 2.5,
  "cable_ok": true,
  "errors": 0,
  "timestamp": 1691123456
}
```

### データフィールド説明

- `id`: デバイスID
- `baseline`: ベースライン値（乾燥時の基準値）
- `current`: 現在の測定値
- `change`: 変化率（%）
- `rain`: 雨検知状態（true/false）
- `method`: 使用中の測定方式（1-4）
- `uptime`: 稼働時間（時間）
- `cable_ok`: ケーブル接続状態
- `errors`: 連続エラー回数
- `timestamp`: UNIX時刻（日本時間）

## 🔧 トラブルシューティング

### よくある問題と解決方法

#### 🟢 緑点滅が続く
**原因**: センサー接続不良または校正失敗
**解決方法**:
1. PIN1、PIN2の配線を確認
2. センサーが完全に乾燥していることを確認
3. 再起動して校正をやり直し

#### 🔴 赤点滅
**原因**: ケーブル脱落またはWiFi切断
**解決方法**:
1. センサーケーブルの接続を確認
2. config.hのWiFi設定を確認
3. WiFiルーターとの距離を確認

#### 通知が来ない
**原因**: 時間帯制限またはPushover設定エラー
**解決方法**:
1. 現在時刻が07:00-19:00の範囲内か確認
2. Pushover API token/User keyを確認
3. 3時間のクールダウン時間を確認

#### 誤検知が多い
**原因**: 閾値が低すぎる
**解決方法**:
1. `RAIN_THRESHOLD_PERCENT`を15→20%に変更
2. センサーの設置場所を見直し
3. 風の影響を受けにくい場所に移動

### デバッグモード

開発・テスト時は以下のフラグを有効化：

```cpp
#define DEBUG_IGNORE_TIME_LIMITS true   // 時刻制限を無視
#define DEBUG_SHORT_COOLDOWN true       // 1分クールダウン
```

## 🌐 応用例

### 農業用途
- **自動散水システム**: 雨検知時の散水停止
- **温室管理**: 換気制御との連携
- **作物保護**: 降雨予測との組み合わせ

### 家庭用途
- **洗濯物通知**: 雨検知時の取り込み促進
- **窓閉め通知**: 外出時の雨対策
- **ガーデニング**: 水やりタイミングの最適化

### 施設管理
- **屋外イベント**: 雨対策の判断支援
- **建設現場**: 作業中止判断の補助
- **気象観測**: 簡易雨量計としての活用

## 📚 APIリファレンス

### 主要関数

#### 測定関数
```cpp
unsigned long measureCapacitanceChargeTime()    // 充電時間測定
unsigned long measureCapacitanceReverse()       // 逆方向測定
unsigned long measureOscillation()              // 発振検出
unsigned long measureAnalogDifference()         // アナログ測定
```

#### 通信関数
```cpp
void sendMQTTData()                             // MQTT送信
void sendPushoverNotification(message, title)   // Pushover通知
bool isNotificationTime()                       // 時刻チェック
```

#### LED制御
```cpp
void setLEDState(LEDState state)                // LED状態変更
void updateLEDStatus()                          // LED更新
```

## 🔒 セキュリティ注意事項

⚠️ **重要**: 以下の情報は機密情報として適切に管理してください

- **config.h**: WiFiパスワード、APIキーを含む
- **GitHubアップロード時**: .gitignoreにconfig.hを追加
- **MQTTブローカー**: 適切なアクセス制御を設定
- **Pushover**: API tokenとUser keyの厳重管理

## 📄 ライセンス

MIT License

Copyright (c) 2025 omiya-bonsai with GitHub Copilot assisted development

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

---

## 📞 サポート

質問やバグ報告は、GitHubのIssuesページにお願いします。

**作成者**: omiya-bonsai with GitHub Copilot assisted development  
**更新日**: 2025年8月4日  
**バージョン**: v2.1（実運用モード、ケーブル脱落検知強化）

---

### 🙏 謝辞

このプロジェクトは、オープンソースコミュニティの多くのライブラリとツールの恩恵を受けています。特に以下のプロジェクトと制作者の方々に感謝いたします：

#### オープンソースライブラリ・プラットフォーム
- **M5Stack team** for M5AtomS3 library
- **Arduino community** for ESP32 support
- **FastLED project** for LED control
- **PubSubClient** for MQTT communication
- **ArduinoJson** for JSON handling

#### ハードウェア・販売プラットフォーム
- **NAOTO** for [静電容量式レインセンサ基板](https://www.switch-science.com/products/8202)
- **スイッチサイエンス** for providing the marketplace and distribution platform

#### AI開発支援
- **Claude (Anthropic)** for code development assistance and documentation
- **Gemini (Google)** for technical consultation and problem-solving support
- **GitHub Copilot** for code completion and development acceleration


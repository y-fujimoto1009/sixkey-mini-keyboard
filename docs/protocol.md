# 6キー設定 V1

JSON: format = sixkey-keymap, version = 1, keys = K1〜K6の6要素。
encoder.clockwise / counterclockwise / press が回転・押し込み。
割り当て番号の定義は model.js の actions にあり、ファームの validAction と対応。

USB CDC 115200 bps / LF終端、ASCII。
読み出し: `K6/1 GET`
保存: `K6/1 SET 49,50,51,52,53,54,2001,2002,2003`
応答: `K6/1 MAP ` に続いて上記9番号。拒否: `K6/1 ERR 理由`。
保存時は入力を検証し、HIDを解放、EEPROMエミュレーションへ保存して物理フラッシュを読み戻す。
サイトはACKと再取得を照合する。再接続後の保持は実機で確認すること。
フラッシュ書き込み中の電源断では初期設定へ戻る可能性がある。

初期値: K1〜K6 = 1〜6、右回転 = 音量上、左回転 = 音量下、押し込み = ミュート。
英字の表示は大文字だが、送信は通常の文字キー（大文字化はShift/Caps Lock依存）。
キーと押し込みは押している間有効。エンコーダー回転とメディア操作は短い入力。
同時押しはUSBの6キー制限がある。CtrlショートカットはWindows/Linux向け。
V1はゲームを含みません。Play V2は3ゲームとキーボードを含みます。更新は利用者がBOOTモードで明示的に行います。

ビルド: Arduino-Pico 5.5.0、Waveshare RP2040 Zero、125MHz、2MB Flash / no filesystem、Pico SDK USB。
ライブラリ: Adafruit GFX、Adafruit ST7735/ST7789、コア標準Keyboard・SPI・EEPROM。
配線: LCD SCK=GP2, MOSI=GP3, DC=GP4, CS=GP5, RESET=GP6, BLK=GP7。
K1〜K6=GP26,27,28,13,14,15。エンコーダーA/B/押し込み=GP10,11,12。
液晶初期化と回転は動作実績のある設定を維持。実機での新ファーム動作は未検証。

## Play V2（ゲーム入り）

`K6/1 CAPS` → `K6/1 CAPS PLAY2`。旧V1は未対応コマンドとしてERRを返します。
`K6/1 GAME 0` → キーボード、1 → ジャンプ、2 → キャッチ、3 → 記憶パズル。
応答は`K6/1 GAME 番号`。0〜3の単一数字だけを受け付けます。
切り替え時はHIDを解放し、全キーを離して500ms経過するまで入力を受け付けません。
ゲーム中はHIDキー・メディア操作を送信しません。GET/SETによるキー設定は維持し、SETが成功するとキーボードへ戻ります。
モードはRAM内だけで管理し、電源投入時はゲームメニューです。K1で前、K3で次、K2で決定、K6でキーボードへ。
ゲーム中はK6でメニュー、終了後はK2でリトライ。キーボード中はK4+K6を約1.2秒長押しでメニューへ戻ります。この長押しでは割り当て済みのキー入力もPCに送られます。

Play V2ソースは`firmware/SixKey_Play/`の2ファイルを同じフォルダーに置きます。ボードとライブラリはV1と同じです。

```sh
arduino-cli compile --fqbn rp2040:rp2040:waveshare_rp2040_zero:flash=2097152_0,freq=125,usbstack=picosdk --output-dir build firmware/SixKey_Play
```

ゲームロジックのCortex-M0エミュレーター検証は`tests/README.md`を参照してください。LCD・USB・GPIOと実物の遊び心地は実機未検証です。

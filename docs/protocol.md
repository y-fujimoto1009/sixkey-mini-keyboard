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
このファームはゲームを含まない。更新は利用者がBOOTモードで明示的に行う。

ビルド: Arduino-Pico 5.5.0、Waveshare RP2040 Zero、125MHz、2MB Flash / no filesystem、Pico SDK USB。
ライブラリ: Adafruit GFX、Adafruit ST7735/ST7789、コア標準Keyboard・SPI・EEPROM。
配線: LCD SCK=GP2, MOSI=GP3, DC=GP4, CS=GP5, RESET=GP6, BLK=GP7。
K1〜K6=GP26,27,28,13,14,15。エンコーダーA/B/押し込み=GP10,11,12。
液晶初期化と回転は動作実績のある設定を維持。実機での新ファーム動作は未検証。

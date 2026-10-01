# 6キー ミニキーボード（RP2040-Zero）

RP2040-Zero、6個のMX互換キースイッチ、押し込み付きEC11エンコーダー、0.96インチST7735 LCDを使ったミニキーボードです。

## キー配置を設定する

**[キーマップ設定サイトを開く](https://y-fujimoto1009.github.io/sixkey-mini-keyboard/)**

6個のキーとエンコーダーの右回転・左回転・押し込みに操作を割り当て、設定JSONを保存できます。USB経由で本体を設定するときは、パソコン版Chrome / Edgeと対応ファームウェアを使用します。

上段はK4・K5・K6、下段はK1・K2・K3です。サイトのUSB接続後、読み込み・保存はそれぞれのボタンで行います。

## ダウンロード

| 内容 | ファイル |
| --- | --- |
| Rev8 ガーバー・ドリル一式 | [Gerber ZIP](hardware/sixkey-rev8-gerbers.zip) |
| 回路図 | [Rev8 回路図PDF](hardware/schematic-rev8.pdf) |
| 部品付き基板の3Dモデル（mm） | [STL](models/keyboard-assembly-mm.stl) / [STEP](models/keyboard-assembly.step) |
| 基板だけの3Dモデル（mm） | [STL](models/pcb-only-mm.stl) |
| 個別部品を含む3Dモデル一式 | [3Dモデル ZIP](models/sixkey-3d-models.zip) |
| 設定サイト用ファームウェア | [UF2](docs/sixkey-config-v1.uf2) / [ソース](firmware/SixKey_Config/SixKey_Config.ino) |

ガーバーの版と確認範囲は[基板の説明](hardware/README.md)、Tinkercadへの取り込み方法と仮寸法は[3Dモデルの説明](models/README.md)を参照してください。

## 確認状況

設定JSON、75種類の割り当て、ファームウェアとの操作番号の一致、UF2形式はソフトウェア検証済みです。本体へのUSB書き込み・電源再投入後の保持・新しいファームウェアのLCD/HID動作は実機未検証です。

UF2を書き込むと本体の既存ファームウェアを置き換えます。設定サイト内の手順を確認してください。[通信仕様](docs/protocol.md)も公開しています。

## サイトを更新する

`docs/`がGitHub Pagesの公開フォルダーです。`main`へ更新をpushするとサイトが更新されます。Node.jsで`npm test`を実行するとソフトウェア検証を行えます。

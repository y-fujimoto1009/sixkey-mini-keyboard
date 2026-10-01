# 6キー ミニキーボード（RP2040-Zero）

RP2040-Zero、6個のMX互換キースイッチ、押し込み付きEC11エンコーダー、0.96インチST7735 LCDを使ったミニキーボードです。

## キー配置を設定する

**[キーマップ設定サイトを開く](https://y-fujimoto1009.github.io/sixkey-mini-keyboard/)**

6個のキーとエンコーダーの右回転・左回転・押し込みに操作を割り当て、設定JSONを保存できます。USB経由で本体を設定するときは、パソコン版Chrome / Edgeと対応ファームウェアを使用します。

上段はK4・K5・K6、下段はK1・K2・K3です。サイトのUSB接続後、読み込み・保存はそれぞれのボタンで行います。

## 本体LCDで遊ぶミニゲーム

現行は**Play V2.2**です。本体の色確認画面でRGB/BGRと反転設定を切り替え、実物に合う設定を保存できます。実機での表示・保存確認は未完了です。

[設定サイトのミニゲーム欄](https://y-fujimoto1009.github.io/sixkey-mini-keyboard/#games)から、[3ゲーム入りUF2](docs/sixkey-play-v2.2.uf2)をダウンロードできます。

- **うさぎのぴょんぴょん散歩**：ジャンプで20個のきのこを飛び越えるアクション。
- **ねこのおやつあつめ**：左右に動いていちごを20個集めるキャッチゲーム。
- **お花のきおくあそび**：光る順番を覚え、K1〜K3で答える8段の記憶パズル。

BOOT接続してRPI-RP2にUF2をコピーすると、3つまとめて本体に入ります。ゲーム中はPCへのHID入力を止めます。ゲームの進行は10項目のCPUエミュレーター検証、サイトの接続処理は模擬シリアル通信で検証済みです。キーボード機能とキー設定の保存形式も含む共通ファームです。

メニューではK1・K3で選択、K2で決定。ゲーム中K6でメニュー、終了後K2でリトライ。メニューのK6でキーボードへ戻ります。キーボードからはK4+K6を約1.2秒長押し（割り当て済みの入力もPCへ送信）。サイトのUSB開始ボタンからもゲームを選べます。

[ゲーム入りソース](firmware/SixKey_Play/)と[通信・ビルド仕様](docs/protocol.md)を公開しています。実機への書き込み・LCD表示・ゲーム操作・設定保持は未検証です。

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

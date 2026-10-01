# ゲームロジックの検証

`Games.h`をCortex-M0向けにコンパイルし、Unicornで実行します。メニュー選択、ジャンプの失敗と20個クリア、キャッチの移動端・失敗・クリア、記憶パズルの入力待ち・8段クリア・間違いを検証します。

描画はスタブ、LCD・USB・GPIOなどの周辺機器は未再現です。実機確認の代わりにはなりません。

```sh
python -m pip install unicorn pyelftools
arm-none-eabi-g++ -mcpu=cortex-m0plus -mthumb -Os -ffreestanding -fno-exceptions -fno-rtti -nostdlib -I tests tests/game_logic.cpp -Wl,-Ttext=0x10000000,-Tdata=0x20000000,-e,run_game_tests -lc -lgcc -o game-tests.elf
python tests/run_arm_tests.py game-tests.elf
```

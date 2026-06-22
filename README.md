This software includes code generated with the assistance of [Gemini & Grok], an AI developed by [Google & xAI].

PyxelでGrokに作成させた横スクロールシューティングをレトロパソコン及び各言語に移植しようとする実験。実機での動作は未保証です。

キーボード操作の場合 X68K/TOWNS/Windows ZショットXボム MSX1/V9968/MSX2 XショットCボム

MSX1 z88dkで作成。→部分アセンブラ化しました。(2026/5/5)

X68K_MPY　Pyxel版を整数化してmicroPythonにテスト移植。(MPUノーウェイト推奨)。放棄。

X68K クラス化をやめて、elf2x68kへ移植。

TOWNS FM TOWNS-gccクロスコンパイル開発環境+DJGPP12.2.0への移植。横長画面にしてるため、実機で液晶だと縞が出ます。CRTの場合は不明です。

PYXEL 逆にグラフィックを寄せた物。

V9968 (MSX2++)X68K版のz88dk(2025/1/26以前の版)縮小移植。改造OpenMSX用。

BMP 旧版の画像データファイル

MSX2 MSX1版の移植。

MSX2ROM MSX2版をROM化。

Direct2D Windows(x64)版。VS2022→2026で作成。(XInputのみ対応) F11でフルスクリーン F12でFPS表示 Bボタン/Xキー開始でライフ付きモード

raylib gcc(MINGW)+raylibによるWindows(x64)版およびEmscriptenによるWeb版。F11でフルスクリーン。

raylibcs VC#+raylib-csによるWindows(x64)版。gcc版の移植。(要.NET SDK 10 & raylib.dll) 

raylibpy Pyxel版をraylib-python-cffiに移植。

raylibgo gcc版をGo+raylib-goに移植。

raylibrust raylib-rsへの移植、作りかけ/環境変数INCLUDEにVS2026のINCLUDEパスを追加してください。

raylibphp php-raylib(composerで入れてください)への移植、作りかけ/exe化の予定なし

JavaScript 作りかけ

作りかけ3種は各.png/.mp3/.wavを他フォルダからコピーしてください。

↓Pyxel版を実行

https://mazone-ma3.github.io/github.io/py/yokosht_plus.html

↓WebMSXでMSX1版を実行

https://webmsx.org/cbios/?MACHINE=MSX1J&rom=https://github.com/mazone-ma3/PRJ_YOKO/raw/refs/heads/main/MSX1/BIN/yokosht.rom

↓WebMSXでMSX2版を実行

https://webmsx.org/cbios/?MACHINE=MSX2J&rom=https://github.com/mazone-ma3/PRJ_YOKO/raw/refs/heads/main/MSX2ROM/BIN/yokoshtm2.rom

↓EmscriptenによるWeb版を実行

https://mazone-ma3.github.io/github.io/PRJYOKO/

↓元のPyxel版のコード

https://github.com/mazone-ma3/py/blob/main/yokosht.py

↓元のPyxel版を実行

https://mazone-ma3.github.io/github.io/py/yokosht.html

YouTubeの方に各機種版の動画を上げてあります。

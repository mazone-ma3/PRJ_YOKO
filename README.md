This software includes code generated with the assistance of [Gemini & Grok], an AI developed by [Google & xAI].

PyxelでGrokに作成させた横スクロールシューティングをレトロパソコンに移植しようとする実験。実機での動作は未保証です。

MSX1 z88dkによる作りかけ。→部分アセンブラ化しました。(2026/5/5)

X68K_MPY　Pyxel版を整数化してmicroPythonにテスト移植した物。(MPUノーウェイト推奨)

X68K クラス化をやめて、elf2x68kに移植した物。

TOWNS FM TOWNS-gccクロスコンパイル開発環境+DJGPP12.2.0に移植した物。

PYXEL 逆にグラフィックを寄せた物。

V9968 (MSX2++)X68K版のz88dk(2025/1/26以前の版)縮小移植。

BMP 旧版の画像データファイル

MSX2 MSX1版の移植。試作。

MSX2ROM MSX2版をROM化した物。

Direct2D Windows(x64)版。VS2022での作りかけ。(XInputのみ対応) F11でフルスクリーン F12でFPS表示 Bボタン/Xキー開始でライフ付きモード

raylib gcc(MINGW)+raylibによるWindows(x64)版およびEmscriptenによるWeb版。試作。F11でフルスクリーン。

raylibcs VC#+raylib-csによるWindows(x64)版。gcc版の移植。試作。(要.NET SDK 10 & raylib.dll) 

raylibpy Pyxel版をraylib-python-cffiに移植した物。試作。

キーボード操作の場合 X68K/TOWNS/Windows ZショットXボム MSX1/V9968/MSX2 XショットCボム

※TOWNS版は横長画面にしてるため、実機で液晶だと縞が出ることがあります。CRTの場合は不明です。

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

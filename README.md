# M302を用いたlowcode評価システム

* Arduino UNO
* M302N

を用いたUECSの測定ノードのプログラムをWEBサイトで
簡単な入力を用いてソースコードを作成する。

## Hardware

### CO2センサ
* murata IMG_CA0012

### 光量子センサ
* 高精度A/Dコンバータ
  Adafruit ADS1115
* 光量子センサ
  [Apogee SQ-100X-SS](https://www.senecom.co.jp/sq100x.html)

## Directories

### main

メインプログラムを収めておく。

### eeprom

EEPROM編集を行うプログラムを収めておく。

### ADS1115

ADS1115のサンプルプログラム

### html

パラメータなどを入力する画面。
inoソースコードを生成するphpプログラムを含む。


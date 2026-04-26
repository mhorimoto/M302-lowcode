# M302を用いたlowcode評価システム

* Arduino UNO
* M302N05専用

SHT4xだけを実装あとはスキップ

を用いたUECSの測定ノードのプログラムをWEBサイトで
簡単な入力を用いてソースコードを作成する。

## Directories

### main

メインプログラムを収めておく。

### eeprom

EEPROM編集を行うプログラムを収めておく。

### html

パラメータなどを入力する画面。
inoソースコードを生成するphpプログラムを含む。

## CCMTBL(uecsM302Send)によるサンプリングの自動化

setup()のときにLowCoreのCCMTBLをLoop Searchして Fast Lookup Table (FSTLB) を作ってそれを実行 Loop 時に参照する。

'''
       F E D C B A 9 8 7 6 5 4 3 2 1 0
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
FSTLB |f|f|f|f|f|f|f|f|L|L|L|L|/|/|/|v|
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      ------^--------- --^----       ^
            |            |           |
            |            |           +----- Valid Flag  0:inValid , 1:Valid
            |            +----------------- LLLL Lv Value
            +------------------------------ Function
'''


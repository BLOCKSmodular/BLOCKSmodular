# MIDI Implementation
##モード切り替えフラグ
**CV/Gate**
| Mode | MIDI CC | Value        | MIDI Channel |
| :--- | :------ | :----------- | ------------ |
| OFF  | 2       | 0            | 16           |
| A    | 2       | 16 (1~32)    | 16           |
| B    | 2       | 48 (33~64)   | 16           |
| C    | 2       | 80 (65~96)   | 16           |
| D    | 2       | 112 (97~127) | 16           |

**Audio**
| Mode | MIDI CC | Value        | MIDI Channel |
| :--- | :------ | :----------- | :----------- |
| OFF  | 1       | 0            | 16           |
| A    | 1       | 16 (1~32)    | 16           |
| B    | 1       | 48 (33~64)   | 16           |
| C    | 1       | 80 (65~96)   | 16           |
| D    | 1       | 112 (97~127) | 16           |


## CV/Gate Mode詳細
### Mode A（Morph Looper）
**BLOCKS → Bela**
Note
| Description | MIDI CC | Value        | MIDI Channel |
| :---------- | :------ | :----------- | :----------- |
| Rec start   | 1       | 0            | 1            |
| A           | 1       | 16 (1~32)    | 1            |
| B           | 1       | 48 (33~64)   | 1            |
| C           | 1       | 80 (65~96)   | 1            |
| D           | 1       | 112 (97~127) | 1            |

CC
| Description | MIDI CC | Value        | MIDI Channel |
| :---------- | :------ | :----------- | :----------- |
| Rec start   | 1       | 0            | 1            |
| A           | 1       | 16 (1~32)    | 1            |
| B           | 1       | 48 (33~64)   | 1            |
| C           | 1       | 80 (65~96)   | 1            |
| D           | 1       | 112 (97~127) | 1            |
### Mode B（Microtonal）

### Mode C
### Mode D

## Audio Mode詳細
### Mode A (グラニュラー)
**BLOCKS → Bela**
CC
| Description               | MIDI CC | Value     | MIDI Channel |
| :------------------------ | :------ | :-------- | :----------- |
| X: Sample start position  | 1, 2    | 0 ~ 2^14  | 1~4          |
| Y : Grain size            | 3, 4    | 0 ~ 2 ^14 | 1~4          |
| Z : Window function shape | 5, 6    | 0 ~ 2^14  | 1~4          |

### Mode B
### Mode C
### Mode D


### Note
##### 録音開始停止フラグ
- pad1: 24
- pad2: 25
- pad3: 26
- pad4: 27
##### 再生中フラグ
- pad1: 36
- pad2: 37
- pad3: 38
- pad4: 39

### CC
##### XYZ
- X: 12, 13
- Y: 14, 15
- Z: 16, 17
##### Microtonal
- CV1: 96, 97
- CV2: 98, 99
- CV3: 100,  101
- CV4: 102, 103
- CV5: 104, 105
- CV6: 106, 107
- CV7: 108, 109
- CV8: 110, 111

## Input(from Max7)
### CC
##### 録音クリアー
- pad1: 72
- pad2: 73
- pad3: 74
- pad4: 75
##### XYZ
- トラック1X: 84
- トラック1Y: 85
- トラック1Z: 86
- トラック2X: 87
- トラック2Y: 88
- トラック2Z: 89
##### モード切り替え
- MorphLooper: 96
- MicrotonalBlock: 97
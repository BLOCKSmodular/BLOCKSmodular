# MIDI Implementation
##モード切り替えフラグ
**General**
| Mode     | MIDI CC | Value            | MIDI Channel   |
| :------- | :------ | :--------------- | :------------- |
| Audio/CV | 8       | 0(Audio)/127(CV) | 16(MIDI: 0xBF) |

**Audio**
| Mode                 | MIDI CC | Value        | MIDI Channel   |
| :------------------- | :------ | :----------- | :------------- |
| OFF                  | 1       | 0            | 16(MIDI: 0xBF) |
| A(Granular sampling) | 1       | 16 (1~32)    | 16(MIDI: 0xBF) |
| B(Sample playback)   | 1       | 48 (33~64)   | 16(MIDI: 0xBF) |
| C(Karplus strong)    | 1       | 80 (65~96)   | 16(MIDI: 0xBF) |
| D(Logistic map)      | 1       | 112 (97~127) | 16(MIDI: 0xBF) |

**CV/Gate**
| Mode               | MIDI CC | Value        | MIDI Channel |
| :----------------- | :------ | :----------- | ------------ |
| OFF                | 2       | 0            | 16           |
| A(Morph looper)    | 2       | 16 (1~32)    | 16           |
| B(Microtonal)      | 2       | 48 (33~64)   | 16           |
| C(未定)            | 2       | 80 (65~96)   | 16           |
| D(Euclid sequence) | 2       | 112 (97~127) | 16           |

## Audio Mode詳細
### Mode A (Granular sampling)
**BLOCKS → Bela**
CC
| Description               | MIDI CC | Value | MIDI Channel |
| :------------------------ | :------ | :---- | :----------- |
| X: Sample start position  | 1, 2    | 14bit | 1~4          |
| Y : Grain size            | 3, 4    | 14bit | 1~4          |
| Z : Window function shape | 5, 6    | 14bit | 1~4          |

### Mode B(Sample playback)
### Mode C(Karplus strong)
### Mode D(Logistic map)

## CV/Gate Mode詳細
### Mode A（Morph looper）
**BLOCKS → Bela**
*CC*
| Description | MIDI CC | Value             | MIDI Channel |
| :---------- | :------ | :---------------- | :----------- |
| Rec         | 1       | 0(End)/127(Begin) | 9, 10        |
| Play        | 2       | 0(Stop)/127(Play) | 9, 10        |
| X           | 3, 4    | 14bit             | 9, 10        |
| Y           | 5, 6    | 14bit             | 9, 10        |
| Z           | 7, 8    | 14bit             | 9, 10        |

*Note*
| Description | Note number | Value   | MIDI Channel |
| :---------- | :---------- | :------ | :----------- |
| Clear       | 1           | Note on | 9, 10        |

### Mode B（Microtonal）
**BLOCKS → Bela**
*CC*
| Description | MIDI CC | Value | MIDI Channel |
| :---------- | :------ | :---- | :----------- |
| Distance    | 1, 2    | 14bit | 9~12         |
| Z           | 3, 4    | 14bit | 9~12         |


### Mode C
### Mode D
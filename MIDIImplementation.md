# MIDI Implementation
##モード切り替えフラグ

**Audio**
| Mode                 | MIDI CC | Value        | MIDI Channel |
| :------------------- | :------ | :----------- | :----------- |
| OFF                  | 1       | 0            | 16           |
| A(Granular sampling) | 1       | 16 (1~32)    | 16           |
| B(Sample playback)   | 1       | 48 (33~64)   | 16           |
| C(Karplus strong)    | 1       | 80 (65~96)   | 16           |
| D(Logistic map)      | 1       | 112 (97~127) | 16           |

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
| Rec         | 1       | 0(End)/127(Begin) | 1~2          |
| Play        | 2       | 0(Stop)/127(Play) | 1~2          |
| X           | 3, 4    | 14bit             | 1~2          |
| Y           | 5,6     | 14bit             | 1~2          |
| Z           | 7,8     | 14bit             | 1~2          |

*Note*
| Description | Note number | Value   | MIDI Channel |
| :---------- | :---------- | :------ | :----------- |
| Clear       | 1           | Note on | 1~2          |

### Mode B（Microtonal）
**BLOCKS → Bela**
*CC*
| Description | MIDI CC | Value | MIDI Channel |
| :---------- | :------ | :---- | :----------- |
| Distance    | 1,2     | 14bit | 1~4          |
| Z           | 3,4     | 14bit | 1~4          |


### Mode C
### Mode D
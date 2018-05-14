# MIDI Implementation
##モード切り替え
| Mode               | MIDI CC | Value | MIDI Channel   |
| :----------------- | :------ | :---- | :------------- |
| Init               | 1       | 0     | 16(MIDI: 0xBF) |
| Granular synthesis | 1       | 1     | 16(MIDI: 0xBF) |
| Sample playback    | 1       | 2     | 16(MIDI: 0xBF) |
| Karplus strong     | 1       | 3     | 16(MIDI: 0xBF) |
| Logistic map       | 1       | 4     | 16(MIDI: 0xBF) |
| Sine circle map    | 1       | 5     | 16(MIDI: 0xBF) |
| Morph looper       | 1       | 6     | 16(MIDI: 0xBF) |
| Microtonal         | 1       | 7     | 16(MIDI: 0xBF) |
| Euclid             | 1       | 8     | 16(MIDI: 0xBF) |

## モード詳細
### Granular synthesis
| Description               | MIDI CC | Value | MIDI Channel |
| :------------------------ | :------ | :---- | :----------- |
| X: Sample start position  | 1, 2    | 14bit | 1~4          |
| Y : Grain size            | 3, 4    | 14bit | 1~4          |
| Z : Window function shape | 5, 6    | 14bit | 1~4          |

### Sample playback
| Description          | MIDI CC | Value             | MIDI Channel |
| :------------------- | :------ | :---------------- | :----------- |
| Transpose(Play/Stop) | 1       | 0(stop)/127(play) | 1~4          |

### Karplus strong
| Description          | MIDI CC | Value | MIDI Channel |
| :------------------- | :------ | :---- | :----------- |
| Distance: Pitch      | 1, 2    | 14bit | 1~4          |
| Z(Pressure): Decay   | 3, 4    | 14bit | 1~4          |
| Touch start: trigger | 5       | 127   | 1~4          |

### Logistic map
| Description                     | MIDI CC | Value | MIDI Channel |
| :------------------------------ | :------ | :---- | :----------- |
| Distance: Logistic map alpha    | 1, 2    | 14bit | 1            |
| Z(Pressure): Volume             | 3, 4    | 14bit | 1            |
| Distance: Sine circle map alpha | 1, 2    | 14bit | 2            |
| Z(Pressure): Volume             | 3, 4    | 14bit | 2            |


### Morph looper
| Description | MIDI CC | Value             | MIDI Channel |
| :---------- | :------ | :---------------- | :----------- |
| Rec         | 1       | 0(End)/127(Begin) | 1, 2         |
| Play        | 2       | 0(Stop)/127(Play) | 1, 2         |
| X           | 3, 4    | 14bit             | 1, 2         |
| Y           | 5, 6    | 14bit             | 1, 2         |
| Z           | 7, 8    | 14bit             | 1, 2         |

*Note*
| Description | Note number | Value   | MIDI Channel |
| :---------- | :---------- | :------ | :----------- |
| Clear       | 1           | Note on | 1, 2         |

### Microtonal
| Description | MIDI CC | Value | MIDI Channel |
| :---------- | :------ | :---- | :----------- |
| Distance    | 1, 2    | 14bit | 1~4          |
| Z           | 3, 4    | 14bit | 1~4          |


### Euclid sequence
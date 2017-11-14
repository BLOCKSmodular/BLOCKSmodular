# MIDI Implementation

## Output(to Max7)

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

### Note

##### 録音クリアー

- pad1: 72
- pad2: 73
- pad3: 74
- pad4: 75

### CC

##### XYZ

- トラック1X: 84
- トラック1Y: 85
- トラック1Z: 86


- トラック2X: 87
- トラック2Y: 88
- トラック2Z: 89
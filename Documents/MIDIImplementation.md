# MIDI Implementation
##Mode change

| Mode          | MIDI CC | Value |    MIDI Channel    |
| :------------ | :-----: | :---: | :----------------: |
| Microtone     |    1    |   0   | 16(MIDI Hex: 0xBF) |
| Euclid        |    1    |   1   | 16(MIDI Hex: 0xBF) |
| Chaotic noise |    1    |   2   | 16(MIDI Hex: 0xBF) |
| Physical drum |    1    |   3   | 16(MIDI Hex: 0xBF) |
| Granular      |    1    |   4   | 16(MIDI Hex: 0xBF) |

<br>
## Detail of each modes
- #### Microtone
| Description                  | MIDI CC | Value |       MIDI Channel       |
| :--------------------------- | :-----: | :---: | :----------------------: |
| CV Odd: Distance from center |  1, 2   | 14bit | 1~4(MIDI Hex: 0xB0~0xB3) |
| CV Even: Z                   |  3, 4   | 14bit | 1~4(MIDI Hex: 0xB0~0xB3) |

- #### Euclid
| Description            | MIDI CC |  Value  |       MIDI Channel       |
| :--------------------- | :-----: | :-----: | :----------------------: |
| Gate: Sequence trigger |    1    | 0 or127 | 1~4(MIDI Hex: 0xB0~0xB3) |

- #### Chaotic noise
| Description                             | MIDI CC | Value |       MIDI Channel       |
| :-------------------------------------- | :-----: | :---: | :----------------------: |
| Chaotic parameter: distance from center |  1, 2   | 14bit | 1~4(MIDI Hex: 0xB0~0xB3) |
| Amp: pressure                           |  3, 4   | 14bit | 1~4(MIDI Hex: 0xB0~0xB3) |

- #### Physical drum
| Description                 | MIDI CC |  Value   |       MIDI Channel       |
| :-------------------------- | :-----: | :------: | :----------------------: |
| Pitch: distance from center |  1, 2   |  14bit   | 1~4(MIDI Hex: 0xB0~0xB3) |
| Decay: pressure             |  3, 4   |  14bit   | 1~4(MIDI Hex: 0xB0~0xB3) |
| Trigger: Touch start        |    5    | 0 or 127 | 1~4(MIDI Hex: 0xB0~0xB3) |

- #### Granular
| Description        | MIDI CC | Value |       MIDI Channel       |
| :----------------- | :-----: | :---: | :----------------------: |
| Sample position: X |  1, 2   | 14bit | 1~4(MIDI Hex: 0xB0~0xB3) |
| Grain size: Y      |  3, 4   | 14bit | 1~4(MIDI Hex: 0xB0~0xB3) |
| Loudness: pressure |  5, 6   | 14bit | 1~4(MIDI Hex: 0xB0~0xB3) |
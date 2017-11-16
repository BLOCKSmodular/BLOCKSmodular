// int cs = 11; //4922の3pinにつなぐ
// int cs2 = 18; //別の4922の3pin
int cs[] = {11, A5, A4, A3};
int sck = 7;//4922の4pin
int sdi = 6; //4922の5pin
int ldac = 5; //4922の16pin
int cvBoardCount = 4;
const float lowpass = 0.85;
float filtered[] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

void setup() {
  // put your setup code here, to run once:
  delay(100);
  Serial.begin(115200);//74800 115200
  for (int i = 0; i < 4; i++) {
    pinMode(cs[i], OUTPUT);
    //    Serial.println(i);
  }
  pinMode(sck, OUTPUT);
  pinMode(sdi, OUTPUT);
  pinMode(ldac, OUTPUT);
  Serial.println("Block modular !");
}

void loop() {
  while (Serial.available() > 0) //more than 2bytes
  {
    uint8_t upperByte = Serial.read();
    if (upperByte & 128 != 128) //上位バイトフラグの判定
    {
      continue;
    }
    uint8_t header = (upperByte >> 4) & 15;
    uint8_t boardIndex;
    uint8_t channel;
    switch (header) {
      case 8://CV1
        boardIndex = 0;
        channel = 0;
        break;
      case 9://CV2
        boardIndex = 0;
        channel = 1;
        break;
      case 10://CV3
        boardIndex = 1;
        channel = 0;
        break;
      case 11:
        boardIndex = 1;
        channel = 1;
        break;
      case 12:
        boardIndex = 2;
        channel = 0;
        break;
      case 13:
        boardIndex = 2;
        channel = 1;
        break;
      case 14:
        boardIndex = 3;
        channel = 0;
        break;
      case 15:
        boardIndex = 3;
        channel = 1;
        break;
      default:
        continue;
        break;
    }
    uint8_t lowerByte = Serial.read();
//    if (lowerByte & 128 == 128) //下位バイトフラグの判定
//    {
//      continue;
//    }
    uint16_t uData = upperByte & 15;
    uint16_t lData = lowerByte & 127;
    uint16_t da_value = (uData << 7) | lData;
    da_value = da_value * 2;//データが11bitできているので12bit相当に変換
    const uint8_t cvOutIndex = boardIndex * 2 + channel;
    //filtered[cvOutIndex] = lowpass * filtered[cvOutIndex]  + (1.0 - lowpass) * (float)da_value;//Lowpass filter
//    if (0.0 <= filtered[cvOutIndex] && filtered[cvOutIndex] < 4096.0)
//    {
//      da_value = (uint16_t)filtered[cvOutIndex];
      digitalWrite(ldac, HIGH);
      digitalWrite(cs[boardIndex], LOW);
      DACout(sdi, sck, channel, da_value);
      digitalWrite(cs[boardIndex], HIGH);
      digitalWrite(ldac, LOW);
      delay(3);//delay 3ms
//    }
  }
  delay(3);//delay 3ms
}

void DACout(int dataPin, int clockPin, int destination, int value)
{
  int i ;

  // コマンドデータの出力
  digitalWrite(dataPin, destination) ; // 出力するピン(OUTA/OUTB)を選択する
  digitalWrite(clockPin, HIGH) ;
  digitalWrite(clockPin, LOW) ;
  digitalWrite(dataPin, LOW) ;       // VREFバッファは使用しない
  digitalWrite(clockPin, HIGH) ;
  digitalWrite(clockPin, LOW) ;
  digitalWrite(dataPin, HIGH) ;      // 出力ゲインは１倍とする
  digitalWrite(clockPin, HIGH) ;
  digitalWrite(clockPin, LOW) ;
  digitalWrite(dataPin, HIGH) ;      // アナログ出力は有効とする
  digitalWrite(clockPin, HIGH) ;
  digitalWrite(clockPin, LOW) ;
  // ＤＡＣデータビット出力
  for (i = 11 ; i >= 0 ; i--) {
    if (((value >> i) & 0x1) == 1) digitalWrite(dataPin, HIGH) ;
    else                           digitalWrite(dataPin, LOW) ;
    digitalWrite(clockPin, HIGH) ;
    digitalWrite(clockPin, LOW) ;
  }
}

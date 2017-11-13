// int cs = 11; //4922の3pinにつなぐ
// int cs2 = 18; //別の4922の3pin
int cs[] = {11, A5, A4, A3};
int sck = 7;//4922の4pin
int sdi = 6; //4922の5pin
int ldac = 5; //4922の16pin
int cvBoardCount = 4;

void setup() {
  // put your setup code here, to run once:
  delay(100);
  Serial.begin(57600);//74800 115200
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
  if (Serial.available() > 1) //more than 2bytes
  {
    byte twobytes[];
    Serial.readBytes(twobytes, 2);
    uint8_t boardIndex = twobytes[0] >> 6 & B00000011;
    uint8_t channel = bitRead(twobytes[0], 5);
    word da_value =  word(twobyte[0] & B00001111, twobyte[1]);
    if (0 <= da_value && da_value < 4096)
    {
      digitalWrite(ldac, HIGH);
      digitalWrite(cs[boardIndex], LOW);
      DACout(sdi, sck, channel, da_value);
      digitalWrite(cs[boardIndex], HIGH);
      digitalWrite(ldac, LOW);
    }
  }
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



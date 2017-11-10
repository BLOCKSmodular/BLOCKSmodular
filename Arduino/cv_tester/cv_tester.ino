// int cs = 11; //4922の3pinにつなぐ
// int cs2 = 18; //別の4922の3pin
int cs[] = {11, 18, 17, 16};
int sck = 12;//4922の4pin
int sdi = 13; //4922の5pin
int ldac = 19; //4922の16pin
int LED_PIN = 10;

void setup() {
  // put your setup code here, to run once:
  delay(100);
  Serial.begin(115200);
  for(int i = 0; i < 4; i++){
    pinMode(cs[i], OUTPUT);
//    Serial.println(i);
  }
  pinMode(sck, OUTPUT);
  pinMode(sdi, OUTPUT);
  pinMode(ldac, OUTPUT); 
  Serial.println("Block modular !");
}

void loop() {
  testCV( 0, cs[0]);
  testCV( 1, cs[0]);
}

uint16_t concatValues( uint8_t value_high, uint8_t value_low)
{
  uint16_t val = (value_high << 8) | (value_low & 0b11111111);
  return val;
}


void DACout(int dataPin,int clockPin,int destination,int value)
{
     int i ;

     // コマンドデータの出力
     digitalWrite(dataPin,destination) ;// 出力するピン(OUTA/OUTB)を選択する
     digitalWrite(clockPin,HIGH) ;
     digitalWrite(clockPin,LOW) ;
     digitalWrite(dataPin,LOW) ;        // VREFバッファは使用しない
     digitalWrite(clockPin,HIGH) ;
     digitalWrite(clockPin,LOW) ;
     digitalWrite(dataPin,HIGH) ;       // 出力ゲインは１倍とする
     digitalWrite(clockPin,HIGH) ;
     digitalWrite(clockPin,LOW) ;
     digitalWrite(dataPin,HIGH) ;       // アナログ出力は有効とする
     digitalWrite(clockPin,HIGH) ;
     digitalWrite(clockPin,LOW) ;
     // ＤＡＣデータビット出力
     for (i=11 ; i>=0 ; i--) {
          if (((value >> i) & 0x1) == 1) digitalWrite(dataPin,HIGH) ;
          else                           digitalWrite(dataPin,LOW) ;
          digitalWrite(clockPin,HIGH) ;
          digitalWrite(clockPin,LOW) ;
     }
}

void testCV(int channel, int cs){
  
        digitalWrite(ldac, HIGH);
        digitalWrite(cs, LOW);
        DACout(sdi, sck, channel, 4095);
        digitalWrite(cs, HIGH);
        digitalWrite(ldac,LOW);
        delay(500);
        
        digitalWrite(ldac, HIGH);
        digitalWrite(cs, LOW);
        DACout(sdi, sck, channel, 0);
        digitalWrite(cs, HIGH);
        digitalWrite(ldac,LOW);
        delay(500);
}



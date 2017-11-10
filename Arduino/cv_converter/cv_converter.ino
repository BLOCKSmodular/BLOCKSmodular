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
  // put your main code here, to run repeatedly:


  if(Serial.available() > 16){
    char head = Serial.read();
    if(head ==104){//recieved header
      for(int cv = 0; cv < 8; cv++){
        int channel = 0;
        uint8_t value_high = Serial.read();
        uint8_t value_low = Serial.read();
        uint16_t concat_value = concatValues(value_high, value_low);
        uint16_t da_value = map(concat_value, 0, 65535, 0, 4095);
        Serial.println(da_value);
        if(cv >=4){
          channel = 1;
        }
        uint8_t value_high = Serial.read();
        uint8_t value_low = Serial.read();
        int concat_value = concatValues(value_high, value_low);
        int da_value = map(concat_value, 0, 65535, 0, 4095);
       
        int cs_id = cv;
        if(channel == 1){
          cs_id = cv - 4;
        }
        digitalWrite(ldac, HIGH);
        digitalWrite(cs[cs_id], LOW);
        DACout(sdi, sck, channel, da_value);
        digitalWrite(cs[cs_id], HIGH);
        digitalWrite(ldac,LOW);
       }
     }
   }
      
//        if(i >3){
//          channel = 1;
//        }
//        uint8_t value_high = Serial.read();
//        uint8_t value_low = Serial.read();
//        int concat_value = concatValues(value_high, value_low);
//        int da_value = map(concat_value, 0, 65535, 0, 4095);
//       
//        int cs_id = i;
//        if(channel == 1){
//          cs_id = i -4;
//        }
//        digitalWrite(ldac, HIGH);
//        digitalWrite(cs[cs_id], LOW);
//        DACout(sdi, sck, channel, da_value);
//        digitalWrite(cs[cs_id], HIGH);
//        digitalWrite(ldac,LOW);
//      
//      }
//    }
//  }
//  delay(2);
}

uint16_t concatValues( uint8_t value_high, uint8_t value_low)
{
  uint16_t val = (value_high << 8) | (value_low & 0b11111111);
  return val;
//  uint16_t concat_value = value_high << 8;
//  concat_value = concat_value + value_low; 
//  return concat_value;
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





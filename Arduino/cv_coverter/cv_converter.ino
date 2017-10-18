int cs = 11; //4922の3pinにつなぐ
int cs2 = 18; //別の4922の3pin
int sck = 12;//4922の4pin
int sdi = 13; //4922の5pin
int ldac = 19; //4922の16pin

void setup() {
  // put your setup code here, to run once:
  delay(100);
  Serial.begin(9600);
  pinMode(cs, OUTPUT);
  pinMode(cs2, OUTPUT);  
  pinMode(sck, OUTPUT);
  pinMode(sdi, OUTPUT);
  pinMode(ldac, OUTPUT);  
}

void loop() {
  // put your main code here, to run repeatedly:
  if(Serial.available() > 2){
    int order = Serial.read();
    if(order ==97 || order == 98 || order == 99 || order ==100){
      
      uint8_t value_high = Serial.read();
      uint8_t value_low = Serial.read();
      
      int concat_value = value_high << 8;
      concat_value = concat_value + value_low; 
      
//      concat_value = constrain(concat_value, 1, 5000);
      int da_value = map(concat_value, 0, 5000, 0, 4000);
      concat_value = map(concat_value, 0, 5000, 0, 255);
      
      
      analogWrite(10, concat_value);
      Serial.write(concat_value);
      
      if(order == 97){
        //CV1
//        analogWrite(3, concat_value);
        
        digitalWrite(ldac, HIGH);
        digitalWrite(cs, LOW);
        DACout(sdi, sck, 0, da_value);
        digitalWrite(cs, HIGH);
        digitalWrite(ldac,LOW);
        
      }
      else if (order ==98){
        //CV2
//        analogWrite(5, concat_value);
        digitalWrite(ldac, HIGH);
        digitalWrite(cs2, LOW);
        DACout(sdi, sck, 0, da_value);
        digitalWrite(cs2, HIGH);
        digitalWrite(ldac,LOW);
      }
      else if (order == 99){
        //CV3
        analogWrite(6, concat_value);
        
      }
      else if (order == 100){
        //CV4
        analogWrite(9, concat_value);
      }
    }
  }
  delay(10);
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

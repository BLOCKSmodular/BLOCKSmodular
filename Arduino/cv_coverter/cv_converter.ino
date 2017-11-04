// int cs = 11; //4922の3pinにつなぐ
// int cs2 = 18; //別の4922の3pin
int cs = [11, 18, 17, 16];
const sck = 12;//4922の4pin
const sdi = 13; //4922の5pin
const ldac = 19; //4922の16pin
const LED_PIN = 10;

void setup() {
  // put your setup code here, to run once:
  delay(100);
  Serial.begin(15200);
  for()
  pinMode(cs[i], OUTPUT);
  pinMode(sck, OUTPUT);
  pinMode(sdi, OUTPUT);
  pinMode(ldac, OUTPUT);  
}

void loop() {
  // put your main code here, to run repeatedly:
  if(Serial.available() > 16){
    char head = Serial.read();
    if(order =='H'){
      for(int i = 0; i < 8; i++){
        int channel = 0;
        if(i >3){channel = 1;}
        uint8_t value_high = Serial.read();
        uint8_t value_low = Serial.read();
        int concat_value = concatValues(value_high, value_low);
        int da_value = map(concat_value, 0, 65536, 0, 4095);
       
        int cs_id = i;
        if(channel == 1){
          cs_id = i -4;
        }
        digitalWrite(ldac, HIGH);
        digitalWrite(cs[cs_id], LOW);
        DACout(sdi, sck, 0, da_value);
        digitalWrite(cs[cs_id], HIGH);
        digitalWrite(ldac,LOW);

      }

  delay(10);
}

int concatValues( int value_high, int value_low){
  int concat_value = value_high << 8;
  concat_value = concat_value + value_low; 
  return concat_value;
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

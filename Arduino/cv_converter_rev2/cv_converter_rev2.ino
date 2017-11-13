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
  if(Serial.available() > 16){
    char head = Serial.read();
    if(head == 104)
    {
      for(int i = 0; i < cvBoardCount; i++)
      {
          cvOut2channel(cs[i]);
      }
    }
  }
}

{

  }
}



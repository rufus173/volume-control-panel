#define P1PIN A0
#define P2PIN A1
#define MAX_P_VAL 1023
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(P1PIN,INPUT);
  pinMode(P2PIN,INPUT);
  while (!Serial){
    ;
  }
}

void loop() {
  // read the values of the pots
  int p1val = analogRead(P1PIN);
  int p2val = analogRead(P2PIN);

  //calculate percentages to send over serial
  int p1percent = ((float)p1val / (float)MAX_P_VAL)*100;
  int p2percent = ((float)p1val / (float)MAX_P_VAL)*100;

  Serial.println(p1percent);
}

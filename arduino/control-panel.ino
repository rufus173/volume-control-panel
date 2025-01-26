#define P1PIN A0
#define P2PIN A1
#define MAX_P_VAL 1023

#include <SPI.h>

#define CS 12

//control registers addresses
#define DECODE_MODE 9
#define INTENSITY 10
#define SCAN_LIMIT 11
#define SHUTDOWN 12
#define DISPLAY_TEST 16

      //data cascades through 16 bits at a time
      //when more then 16 bits are sent, the first bits start being pushed out to the next chip
      // c2       c1
      // 00000000 01010001 << 10000001
      // 01010001 10000001

const int bar_renders[][4] = {
  {0x00,0x00,0x00,0x00},
  {0x00,0x00,0x00,0xc0},
  {0x00,0x00,0x30,0xf0},
  {0x00,0x0c,0x3c,0xfc},
  {0x03,0x0f,0x3f,0xff},
};

class LedMatrix {
  private:
    uint16_t cs = 7;
  public:
    void send_data(uint8_t address, uint8_t value){
      digitalWrite(cs, LOW);//tell chip data is transfering
      //flush all with no-ops
      for (int i = 0;i < 4;i++){
        delay(3);
        SPI.transfer(0xf0); //the 0 in f0 is the noop register
        SPI.transfer(0xff);
      }
      delay(3); //data comes out after 16.5 clock cycles so this makes sure it can pass though propperly
      SPI.transfer(address);
      SPI.transfer(value);
      delay(3);
      //give the data time to reach its destination
      digitalWrite(cs,HIGH);
    }
    //constructor
    LedMatrix(uint16_t cs_pin){
      cs = cs_pin;
    }
    void begin(){
      delay(100);
      send_data(SCAN_LIMIT,0x07); //display all dots
      send_data(DECODE_MODE,0x00); //directly address pixels
      send_data(INTENSITY,0x01);
      send_data(SHUTDOWN,0x01);
    }
    void set_column(uint8_t column,uint8_t bitmask){ //or together R_* to make a column
      send_data(column+1,bitmask);      
    }
    void display_percents(uint8_t percent1, uint8_t percent2){
      uint8_t bar_count_1 = 0;
      uint8_t bar_count_2 = 0;
      if (percent1 <= 5){
        bar_count_1 = 0;
      }else if ((percent1 > 5 ) && (percent1 <= 35)){
        bar_count_1 = 1;
      }else if ((percent1 > 35) && (percent1 <= 65)){
        bar_count_1 = 2;
      }else if ((percent1 > 65) && (percent1 <= 85)){
        bar_count_1 = 3;
      }else if (percent1 > 85){
        bar_count_1 = 4;
      }
      if (percent2 <= 5){
        bar_count_2 = 0;
      }else if ((percent2 > 5 ) && (percent2 <= 35)){
        bar_count_2 = 1;
      }else if ((percent2 > 35) && (percent2 <= 65)){
        bar_count_2 = 2;
      }else if ((percent2 > 65) && (percent2 <= 85)){
        bar_count_2 = 3;
      }else if (percent2 > 85){
        bar_count_2 = 4;
      }
      //display bar 1 
      for (int i = 0; i < 4; i++){
        set_column(i,bar_renders[bar_count_1][i]);
      }
      for (int i = 0; i < 4; i++){
        set_column(i+4,bar_renders[bar_count_2][i]);
      }
      //display bar 2
    }
};

static LedMatrix led_matrix(CS);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  SPI.begin();
  pinMode(P1PIN,INPUT);
  pinMode(P2PIN,INPUT);
  pinMode(CS,OUTPUT);
  led_matrix.begin();
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
  int p2percent = ((float)p2val / (float)MAX_P_VAL)*100;

  Serial.print(p1percent);
  Serial.print(",");
  Serial.println(p2percent);
  led_matrix.display_percents(p1percent,p2percent);
}

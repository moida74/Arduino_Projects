/*  
Tidsmaskin - a Time Machine
By Morten and Kristoffer 2016

Unfinished version 0.1

*/


#include <Servo.h>


/* Define 4 digit 7 segment LED (SMA20564) pins */
#define d1 A1
#define d2 A2
#define d3 A3
#define d4 A4

/* Define input pins */
int potensiometer = A0;
int knapp = 13;

/* Define servo pin */
int servoPin = 12;

/* Define led pins */
int led1 = 2;
int led2 = 3;
int led3 = 4;
int led4 = 5;

/* Define shift register (74HC595) pins */
int DS = 6;
int SH_CP = 7;
int ST_CP = 8;

/* Define digits */
const byte digit[11] ={
	B01111110,
	B00001100,
	B10110110,
	B10011110,
	B11001100,
	B11011010,
	B11111010,
	B00001110,
	B11111110,
	B11011110,
	000000000
};

const byte d[4] ={A1,A2,A3,A4};


Servo tidsviser; 

void setup() {
  // put your setup code here, to run once:
  pinMode(A0, INPUT);
  pinMode(knapp, INPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);

  pinMode(d1,OUTPUT);
  pinMode(d2,OUTPUT);
  pinMode(d3,OUTPUT);
  pinMode(d4,OUTPUT);

  pinMode(DS, OUTPUT);
  pinMode(SH_CP, OUTPUT);
  pinMode(ST_CP, OUTPUT);


  digitalWrite(d1,LOW);
  digitalWrite(d2,LOW);
  digitalWrite(d3,LOW);
  digitalWrite(d4,LOW);

  clearDigits();

  digitalWrite(led1,LOW);
  digitalWrite(led2,LOW);
  digitalWrite(led3,LOW);
  digitalWrite(led4,LOW);

  tidsviser.attach(servoPin);  


  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:

  	int i = analogRead(potensiometer);
  	int aar = map(i,0,1023,0,2000);
    int deg = map(i,0,1023,180,20);
	
	tidsviser.write(deg);

	if (digitalRead(knapp)==LOW) {
		

		int d = 100;
		
		for(int k=0;k<3;k++) {

			digitalWrite(led4,LOW);
			digitalWrite(led1,HIGH);
			delay(d);
			digitalWrite(led1,LOW);
			digitalWrite(led2,HIGH);
			delay(d);
			digitalWrite(led2,LOW);
			digitalWrite(led3,HIGH);
			delay(d);
			digitalWrite(led3,LOW);
			digitalWrite(led4,HIGH);
			delay(d);
		} 
		
		digitalWrite(led4,LOW);
		int c = constrain(aar/117,1,1000);
		for (int i=0;i<aar;i+=c){

			writeNumberToLED(i,10);
		} 
		delay(150);
		writeNumberToLED(aar,200);
		delay(150);
		writeNumberToLED(aar,200);
		delay(150);
		writeNumberToLED(aar,2000);




	} else {
		digitalWrite(led1,LOW);
		digitalWrite(led2,LOW);
		digitalWrite(led3,LOW);
		digitalWrite(led4,LOW);

	}
}

/* 
	**** SPECIAL FUNCTIONS **** 	
*/



// Write digit to shift register and output to LED

void outputDigit(int pos, int dig) {

	for (int i=0;i<4;i++) digitalWrite(d[i], HIGH);	
	digitalWrite(ST_CP, LOW); 
	shiftOut(DS, SH_CP, MSBFIRST, digit[dig]);
	digitalWrite(ST_CP, HIGH); 
	digitalWrite(d[pos-1], LOW);
	delay(1);
}

// Clear LED digits
void clearDigits() {

	digitalWrite(DS,LOW);
	digitalWrite(SH_CP,LOW);
	digitalWrite(ST_CP,LOW);

	for(int i=0;i<8;i++) {
		digitalWrite(SH_CP,HIGH);
		digitalWrite(SH_CP,LOW);
	}
	digitalWrite(ST_CP,HIGH);
	digitalWrite(ST_CP,LOW);
}

// Write a number to the LED display for a given number of milliseconds

void writeNumberToLED(int number,unsigned long duration) {


	int  t1000 = constrain(int(number/1000),0,9);
	int  t100  = constrain((int(number-t1000*1000)/100),0,9);
	int  t10   = constrain((int(number-t1000*1000-t100*100)/10),0,9);
	int  t1    = constrain((int(number-t1000*1000-t100*100-t10*10)),0,9);

	if (t1000==0) {
			t1000=10;
			if (t100==0) {
				t100=10;
				if (t10==0) {
					t10=10;
				}
			}
		}  

	unsigned long ms = millis();

	while (millis()<ms+duration) {
			
				  outputDigit(1,t1000);
				  outputDigit(2,t100);
	  			  outputDigit(3,t10);
				  outputDigit(4,t1);
			}
			
	clearDigits();

} 







/*  
	Tidsmaskin - a Time Machine
	Invented by Kristoffer 
	with assistance from Dad
	Version 1.0, February 2016
*/

#include <Servo.h>

/* Define 4 digit 7 segment LED (SMA20564) d1-d4 pins */
#define d1 A1
#define d2 A2
#define d3 A3
#define d4 A4
const byte d[4] ={d1,d2,d3,d4};

/* Define 8-bit digit characters */
const byte digit[11] ={
	B01111110, // 0..9
	B00001100, 
	B10110110, 
	B10011110,
	B11001100,
	B11011010,
	B11111010,
	B00001110,
	B11111110,
	B11011110,
	B00000000 // blank (whitespace)
};

/* Servo for the year-o-meter hand */
Servo tidsviser; 

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

/* Define buzzer pins */
int buzzer = 9;

int globalDelay = 1;
/* Setup */

void setup() {

	// Define input pins
	pinMode(A0, INPUT);
	pinMode(knapp, INPUT);

	// Define output pins
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
	pinMode(buzzer, OUTPUT);

	// Init LED display
	digitalWrite(d1,LOW);
	digitalWrite(d2,LOW);
	digitalWrite(d3,LOW);
	digitalWrite(d4,LOW);
	clearDigits();

	// Init LED lamps
	digitalWrite(led1,LOW);
	digitalWrite(led2,LOW);
	digitalWrite(led3,LOW);
	digitalWrite(led4,LOW);

	// Attach year-o-meter
	tidsviser.attach(servoPin);  

	// Debug via serial
	
	Serial.begin(9600);

	// Ready sound
	tone(buzzer,1000,50);
	delay(100);
	tone(buzzer,2000,50);
	delay(100);
	tone(buzzer,4000,50);
	delay(100);

}

/* Main program loop */

void loop() {

	// Read potentiometer and map values in years and degrees
	int i = analogRead(potensiometer);
  	int aar = map(i,0,1023,0,2000);
    int deg = map(i,0,1023,180,20);
	
	tidsviser.write(deg); 	// Update year-o-meter

	if (digitalRead(knapp)==LOW) { // Button pressed

		int d = 100;

		// Flash the LED bulbs and create random tones:	
		for(int k=0;k<3;k++) {

			digitalWrite(led4,LOW);
			digitalWrite(led1,HIGH);
			tone(buzzer,random(1000)+2000);
			delay(d/2);
			tone(buzzer,random(1000)+100,50);
			delay(d/2);
			digitalWrite(led1,LOW);
			digitalWrite(led2,HIGH);
			tone(buzzer,random(1000)+2000);
			delay(d/2);
			tone(buzzer,random(1000)+100,50);
			delay(d/2);
			digitalWrite(led2,LOW);
			digitalWrite(led3,HIGH);
			tone(buzzer,random(1000)+2000);
			delay(d/2);
			tone(buzzer,random(1000)+100,50);
			delay(d/2);
			digitalWrite(led3,LOW);
			digitalWrite(led4,HIGH);
			tone(buzzer,random(1000)+2000);
			delay(d/2);
			tone(buzzer,random(1000)+100,50);
			delay(d/2);
	} 
		
		digitalWrite(led4,LOW);

		// Count LED display from 0 to selected year while playing a sound:
		int c = constrain(aar/117,1,1000);
		for (int i=0;i<aar;i+=c){
			tone(buzzer,i+100);
			writeNumberToLED(i,10);
		} 
		noTone(buzzer);
		delay(150);

		// Flash selected year on LED display and beep:
		int minFreq = 200 + aar;
		tone(buzzer,minFreq/4,50);
		writeNumberToLED(aar,200);
		delay(150);
		tone(buzzer,minFreq,50);
		writeNumberToLED(aar,200);
		delay(150);
		tone(buzzer,minFreq*2,50);
		writeNumberToLED(aar,2000);
	} 
}

/* 
	**** SPECIAL FUNCTIONS **** 	
*/

// Write an 8-bit character (dig) to the shift register and output it to a position (pos) on the LED
void outputDigit(int pos, int dig) {

	for (int i=0;i<4;i++) digitalWrite(d[i], HIGH);	// stop output on all LEDs
	digitalWrite(ST_CP, LOW);  // stop output, prepare input
	shiftOut(DS, SH_CP, MSBFIRST, digit[dig]); // write the character to the shift register
	digitalWrite(ST_CP, HIGH); // start output
	digitalWrite(d[pos-1], LOW); // output the digit to the chosen LED
	delay(globalDelay); // A short delay to increase the brightness when multiplexing
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

// Write a number (number) to the LED display for a given number of milliseconds (duration)
void writeNumberToLED(int input_number,unsigned long duration) {

	// Split digits:
	int t1 = (input_number%10);
	int t10 = ((input_number/10)%10);
	int t100 = ((input_number/100)%10);
	int t1000 = (input_number/1000);

	// Remove leading zeros:
	if (t1000==0) {
		t1000=10; 	
	if (t100==0) {
		t100=10; 	
	if (t10==0) {
		t10=10; 	
	}}}  

	// Output digits using multiplexing:
	unsigned long ms = millis();	
	while (millis()<ms+duration) {
		  outputDigit(1,t1000);
		  outputDigit(2,t100);
		  outputDigit(3,t10);
		  outputDigit(4,t1);
	}
	
	// Clean up:
	clearDigits();
} 
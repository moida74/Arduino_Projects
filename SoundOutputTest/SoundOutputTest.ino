// SoundOutputTest.ino
#include <TimerOne.h>


int soundPin = 5;
int vol = 100;

void setup() {

	pinMode(soundPin, OUTPUT);
	Timer1.initialize(2000);         // initialize timer1, and set a 1/2 second period
 	Timer1.attachInterrupt(createSoundWave);
}

void loop() {

}

void createSoundWave() {

	vol = -vol;
	int out = 128 + vol;
	analogWrite(soundPin, out);
	delay(4);

}


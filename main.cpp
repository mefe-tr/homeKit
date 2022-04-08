#include <iostream>
#include <wiringPi.h>
#include "sht7x.h"
#include "Mqtt.h"
#include <thread>
#include <string>

// LED Pin - wiringPi pin 0 is BCM_GPIO 17.
// we have to use BCM numbering when initializing with wiringPiSetupSys
// when choosing a different pin number please use the BCM numbering, also
// update the Property Pages - Build Events - Remote Post-Build Event command
// which uses gpio export for setup for wiringPiSetupSys
using namespace std;

float temperature = 0.0, humudity = 0.0;

uint32_t counter = 0;

void getTemperetatureAndHumudity() {
	Sht7x sht7x(&temperature, &humudity);
	sht7x.measure();
}
void publishData() {
	this_thread::sleep_for(chrono::seconds(20)); //start this task after 20sec

	Mqtt mqtt(&temperature, &humudity);
	mqtt.run();
}
int main(void)
{
	cout << "Starting program..."<<endl;

	//wiringPiSetupSys();
	wiringPiSetupPhys();
	
	thread th_sht7x(getTemperetatureAndHumudity);
	thread th_mqtt(publishData);

	th_sht7x.join();
	th_mqtt.join();

	return 0;
}
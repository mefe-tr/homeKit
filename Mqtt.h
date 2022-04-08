#ifndef __MQTT_H__
#define __MQTT_H__

#include <iostream>
#include <string>
#include <thread>
#include <mutex>

using namespace std;

class Mqtt
{
#define PUBLISH_RELOAD_TIME	60 //second
#define PUBLISH_WAIT_SENSITIVE 30 //second
private:
	float* temperature, * humudity;
	string fileName = "mqtt.py";
	uint64_t lastPublisedTime = 0;
	mutex mRun;

	void controlRunningProcess();
	void publishData();
	void waitForNextPublishment();

public:
	Mqtt(float* t = nullptr, float* h=nullptr)
	{
		if( t != nullptr) temperature = t;
		if( h != nullptr) humudity = h;
	}

	~Mqtt(){ }

	void run();
};

#endif


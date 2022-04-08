#ifndef __SHT7X_H__
#define __SHT7X_H__

#include <iostream>
#include <wiringPi.h>
#include <pthread.h>
#include <mutex>

using namespace std;

class Sht7x
{
//command line = gpio readall
#define	DATA	13 //Wpi => GPIO.2, BCM => GPIO 27, Pys => GPIO 13
#define SCK		15 //Wpi => GPIO.3, BCM => GPIO 22, Pys => GPIO 15

						  //adr command	r/w
#define STATUS_REG_W 0x06 //000 0011 0
#define STATUS_REG_R 0x07 //000 0011 1
#define MEASURE_TEMP 0x03 //000 0001 1
#define MEASURE_HUMI 0x05 //000 0010 1
#define RESET		 0x1e //000 1111 0

#define MEASUREMENT_RELOAD_TIME 240 //second
#define MESAREMENT_WAIT_SENSITIVE 60 //second
#define TEMP_RES_14BIT	
//#define TEMP_RES_12BIT
#ifdef TEMP_RES_14BIT
#define d2	0.01
#else
#define d2	0.04
#endif

//#define TEMP_VDD_5_0V
//#define TEMP_VDD_4_0V
//#define TEMP_VDD_3_5V
//#define TEMP_VDD_3_0V
//#define TEMP_VDD_2_5V
#define TEMP_VDD_3_3V

#ifdef TEMP_VDD_5_0V
#define d1 -40.1
#elif defined(TEMP_VDD_4_0V)
#define d1 -39.8
#elif defined(TEMP_VDD_3_5V)
#define d1 -39.7
#elif  defined(TEMP_VDD_3_0V)
#define d1 -39.6
#elif  defined(TEMP_VDD_2_5V)
#define d1 -39.4
#else
#define d1 -39.65
#endif

private:
	enum meaMode
	{
		TEMP = 0,
		HUM
	};

	typedef union uValue
	{
		uint16_t u16;
		float	f;
	};

	const float C1 = -2.0468;		// for 12 Bit RH
	const float C2 = +0.0367;		// for 12 Bit RH
	const float C3 = -0.0000015955; // for 12 Bit RH
	const float T1 = +0.01;			// for 12 Bit RH
	const float T2 = +0.00008;		// for 12 Bit RH
	float *humudity, *temperature, dewPoint;
	uint32_t convertTimeHum, convertTimeTemp;
	bool isThrowError;
	mutex mMeasurement;
	uint64_t lastMeasurementTime = 0;

	void throwError(string funName);
	void measureData();
	void waitForNextMeasurement();
	void startCommunication();
	void resetCommunication();
	bool softReset();
	bool writeByte(uint8_t byte);
	bool readByte( uint8_t &byte, bool ACK);
	bool readStatusRegister(uint8_t &value, uint8_t& checkSum);
	bool writeStatusRegister(uint8_t value);
	bool measureTempOrHum(float &value, uint8_t &checkSum, meaMode mode);
	bool calculateMeasurement(float& humudity, float& temperature);
	bool calculateDewPoint(float& dewPoint, float humudity, float temperature);

	
public:
	Sht7x(float* t = nullptr, float *h=nullptr)
	{ 
		if (h != nullptr) humudity = h;
		if (t != nullptr) temperature = t;
	}

	~Sht7x() { }
	
	void measure();
	float getHumudity() { return *humudity; }
	float getTemperature() { return *temperature; }
};

#endif

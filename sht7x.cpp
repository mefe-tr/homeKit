#include <iostream>
#include "sht7x.h"
#include <stdio.h>
#include <wiringPi.h>
#include <math.h>
#include <chrono>
#include <pthread.h>
#include <thread>
#include <mutex>

void Sht7x::throwError(string funName)
{
	try
	{
		isThrowError = true;
		cout << "Error!.. Function Name = " << funName << endl;
		this->softReset();
		this_thread::sleep_for(chrono::milliseconds(15));
		this->resetCommunication();
	}
	catch (const std::exception&)
	{
		
	}
}

void Sht7x::startCommunication()
{
//			_____		 ________
// DATA:		 |_______|
//              ___     ___
// SCK :	___|   |___|   |______
	//Start Position

	digitalWrite(SCK, LOW);
	digitalWrite(DATA, HIGH);
	delayMicroseconds(1);

	digitalWrite(SCK, HIGH);
	delayMicroseconds(1);
	digitalWrite(DATA, LOW);
	delayMicroseconds(1);

	digitalWrite(SCK, LOW);
	delayMicroseconds(5);
	digitalWrite(SCK, HIGH);
	delayMicroseconds(1);

	digitalWrite(DATA, HIGH);
	delayMicroseconds(1);
	digitalWrite(SCK, LOW);
	delayMicroseconds(1);
}
void Sht7x::resetCommunication()
{
	pinMode(DATA, OUTPUT);
	pinMode(SCK, OUTPUT);

	digitalWrite(SCK, LOW);
	digitalWrite(DATA, HIGH);
	delayMicroseconds(1);

	for (size_t i = 0; i < 9; i++)
	{
		digitalWrite(SCK, HIGH);
		delayMicroseconds(1);
		digitalWrite(SCK, LOW);
		delayMicroseconds(1);
	}

	this->startCommunication();
}
bool Sht7x::softReset()
{

	try
	{
		this->resetCommunication();
		if (!this->writeByte(RESET)) return false;
		
		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
}
bool Sht7x::writeByte(uint8_t byte)
{
	try
	{
		uint8_t ack = 0xFF;

		pinMode(DATA, OUTPUT); 
		digitalWrite(SCK, LOW);

		for (size_t i = 0x80; i > 0x00; i/=2)
		{
			if( (byte & i) != 0x00 )
				digitalWrite(DATA, HIGH);
			else
				digitalWrite(DATA, LOW);

			digitalWrite(SCK, HIGH);
			delayMicroseconds(5);
			digitalWrite(SCK, LOW);
			delayMicroseconds(5);
		}

		
		digitalWrite(SCK, LOW);
		pinMode(DATA, INPUT); //release pin
		delayMicroseconds(1);

		digitalWrite(SCK, HIGH);
		delayMicroseconds(5);

		ack = digitalRead(DATA);
		if (ack != 0) return false; //ACK false
		//9. clock
		digitalWrite(SCK, LOW);
		delayMicroseconds(5);

		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
}
bool Sht7x::readByte(uint8_t& byte, bool ACK)
{
	try
	{
		uint8_t val = 0, ack = 0xFF;

		pinMode(DATA, INPUT);
		digitalWrite(SCK, LOW);

		for (size_t i = 0x80; i > 0x00; i/=2)
		{
			digitalWrite(SCK, HIGH);
			delayMicroseconds(5);

			if (digitalRead(DATA) != 0x00)
				val |= i;

			digitalWrite(SCK, LOW);
			delayMicroseconds(5);
		}

		pinMode(DATA, OUTPUT);
		if(ACK)
			digitalWrite(DATA, LOW);
		else
			digitalWrite(DATA, HIGH);

		delayMicroseconds(1);

		digitalWrite(SCK, HIGH);
		delayMicroseconds(5);
		digitalWrite(SCK, LOW);
		delayMicroseconds(5);

		digitalWrite(DATA, HIGH);
		delayMicroseconds(1);

		byte = val;
		return true;
	}
	catch (const std::exception&)
	{
		byte = 0;
		return false;
	}
}
bool Sht7x::readStatusRegister(uint8_t& value, uint8_t& checkSum)
{
	try
	{
		this->startCommunication();
		
		if (!this->writeByte(STATUS_REG_R)) return false;

		if (!this->readByte(value, true)) return false;
		if (!this->readByte(checkSum, false)) return false;

		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
}
bool Sht7x::writeStatusRegister(uint8_t value)
{
	try
	{
		this->startCommunication();
		if (!this->writeByte(STATUS_REG_W)) return false;
		if (!this->writeByte(value)) return false;

		return true;
	} 
	catch (const std::exception&)
	{
		return false;
	}
}
bool Sht7x::measureTempOrHum(float& value, uint8_t& checkSum, meaMode mode)
{
	try
	{
		uint8_t valLow = 0, valHigh = 0;
		this->startCommunication();

		if (mode == TEMP)
		{
			if (!this->writeByte(MEASURE_TEMP)) return false;
		}
		else
		{
			if (!this->writeByte(MEASURE_HUMI)) return false;
		}

		auto startTime	 = chrono::steady_clock::now();
		while(1)
		{
			if (digitalRead(DATA) == 0) break;

			auto currentTime = chrono::steady_clock::now();
			if (chrono::duration_cast<chrono::milliseconds>(currentTime - startTime).count() >= 2000)
				return false;
			else
				this_thread::sleep_for(chrono::milliseconds(100));
		}

		auto endTime = chrono::steady_clock::now();
		if (mode == TEMP)
		{
			convertTimeTemp = chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count();
		}
		else
		{
			convertTimeHum = chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count();
		}

		if (!this->readByte(valHigh, true)) return false;
		if (!this->readByte(valLow, true)) return false;
		if (!this->readByte(checkSum, false)) return false;

		uint16_t val = ((uint16_t)((uint16_t)valHigh << 8) & 0xFF00) | ((uint16_t)valLow & 0x00FF);

		uValue vUnion;
		vUnion.u16 = val;
		value = val;

		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
}
bool Sht7x::calculateMeasurement(float& humudity, float& temperature)
{
	try
	{
		float humLineer, humReal, temp;

		temp = temperature * d2 + d1;

		humLineer  = (humudity * humudity * C3) + (humudity * C2) + C1;
		humReal = ((temp - 25) * (T1 + (T2 * humudity))) + humLineer;

		if(temp > 100.0) return false;
		if (humReal < 0.1) return false;

		humudity	= humReal;
		temperature = temp;

		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
}
bool Sht7x::calculateDewPoint(float& dewPoint, float humudity, float temperature)
{
	try
	{
		float k = ( log10( humudity ) - 2) / 0.4343 + (17.62 * temperature) / (243.12 + temperature);
		dewPoint = 243.12 * k / (17.62 - k);
		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
}
void Sht7x::measureData()
{
	float tempTemperature, tempHumudity;
	uint8_t checkSum;

	if (this->measureTempOrHum(tempHumudity, checkSum, HUM))
	{
		if (this->measureTempOrHum(tempTemperature, checkSum, TEMP))
		{
			if (this->calculateMeasurement(tempHumudity, tempTemperature))
			{
				if (this->humudity != nullptr) *(this->humudity) = tempHumudity;
				if (this->temperature != nullptr) *(this->temperature) = tempTemperature;

				if (this->calculateDewPoint(this->dewPoint, tempHumudity, tempTemperature))
				{
					isThrowError = false;

					cout << "Temp = " << *(this->temperature) << " Humunity = " << *(this->humudity) << " Dew Point = " << this->dewPoint << 
						" tTime = " << convertTimeTemp << " hTime = " << convertTimeHum << " Last Measurement Time : " << unsigned(lastMeasurementTime) << endl;
				}
				else
				{
					this->throwError("calculateDewPoint(this->dewPoint, tempHumudity, tempTemperature)");
				}
			}
			else
			{
				this->throwError("calculateMeasurement(tempHumudity, temperature)");
			}
		}
		else
			this->throwError("measureTempOrHum(tempTemperature, checkSum, TEMP)");
	}
	else
		this->throwError("measureTempOrHum(tempHumudity, checkSum, HUM)");
}
void Sht7x::waitForNextMeasurement()
{
	try
	{
		uint32_t waitTime = 1, waitSensetiveTime = 1;

		auto startTime = chrono::steady_clock::now();
		while (1) {
			auto currentTime = chrono::steady_clock::now();

			waitTime = (isThrowError) ? 5 : MEASUREMENT_RELOAD_TIME;
			waitSensetiveTime = (isThrowError) ? 1 : MESAREMENT_WAIT_SENSITIVE;
			if (chrono::duration_cast<chrono::seconds>(currentTime - startTime).count() >= waitTime)
				break;
			else
				this_thread::sleep_for(chrono::seconds(waitSensetiveTime));
		}

		auto endTime = chrono::steady_clock::now();
		lastMeasurementTime = chrono::duration_cast<chrono::seconds>(endTime - startTime).count();
	}
	catch (const std::exception&)
	{

	}
}
void Sht7x::measure()
{
	uint8_t status = 0xFF, checkSum = 0xFF;

	lock_guard<mutex> lock(mMeasurement); //lock function to avoid multiprocessing

	pinMode(VCC, OUTPUT);
	pinMode(SCK, OUTPUT);
	pinMode(DATA, OUTPUT);

	digitalWrite(VCC, HIGH);
	this_thread::sleep_for(chrono::milliseconds(15));

	this->softReset();
	this_thread::sleep_for(chrono::milliseconds(15));
	this->resetCommunication();

	while (1)
	{
		try
		{
			//this->readStatusRegister(status, checkSum);
			//cout << "Status = " << unsigned(status) << " CheckSum = " << unsigned(checkSum) << endl;
			this->measureData();
			this->waitForNextMeasurement();
		}
		catch (const std::exception&)
		{
			this->throwError("measure()");
		}
	}
}
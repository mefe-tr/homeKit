#include "Mqtt.h"

void Mqtt::controlRunningProcess()
{
	try
	{
		std::string pythonCommand = "sudo pkill -f " + fileName;
		int res = system(pythonCommand.c_str());
	}
	catch (const std::exception&)
	{

	}
}

void Mqtt::publishData() 
{
	try
	{
		if (temperature == nullptr || humudity == nullptr) return;

		std::string pythonCommand = "python /home/pi/projects/homeKit/bin/ARM/Debug/"+fileName+" " + to_string(*temperature) + " " + to_string(*humudity);
		int res = system(pythonCommand.c_str());

		cout << "Data is published. Last publised time : " << unsigned(lastPublisedTime) << endl;
	}
	catch (const std::exception&)
	{
		
	}
}
void Mqtt::waitForNextPublishment()
{
	try
	{
		uint32_t waitTime = 1;
		
		auto startTime = chrono::steady_clock::now();
		while (1) {
			auto currentTime = chrono::steady_clock::now();

			if (chrono::duration_cast<chrono::seconds>(currentTime - startTime).count() >= PUBLISH_RELOAD_TIME)
				break;
			else
				this_thread::sleep_for(chrono::seconds(PUBLISH_WAIT_SENSITIVE));
		}
		auto endTime = chrono::steady_clock::now();
		lastPublisedTime = chrono::duration_cast<chrono::seconds>(endTime - startTime).count();

	}
	catch (const std::exception&)
	{

	}
}
void Mqtt::run() 
{
	lock_guard<mutex> lock(mRun);

	while (1)
	{
		try
		{
			controlRunningProcess();
			publishData();
			waitForNextPublishment();
		}
		catch (const std::exception&)
		{

		}
	}
}

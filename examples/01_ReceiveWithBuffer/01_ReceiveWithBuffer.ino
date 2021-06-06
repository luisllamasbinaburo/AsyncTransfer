#include "AsyncTransfer.h"

struct Data
{
	uint16_t x;
	uint16_t y;
	uint16_t z;
};

Data data;

AsyncTransfer transfer(Serial);

byte buffer[200];
uint16_t bufferLength = sizeof(buffer) / sizeof(byte);

void debug(String type)
{
	Serial.print(type);
	Serial.print(" at: ");
	Serial.print(millis());
	Serial.println("ms ");
}

void setup()
{
	while(!Serial) { ; }
	Serial.begin(9600);
	Serial.println("Starting");

	transfer.Config.AutoReset = true;
    transfer.OnSuccess = []() { debug("OK"); transfer.CopyPayloadTo(data); Serial.println(data.x); Serial.println(data.y);  Serial.println(data.z); };
	transfer.OnError = []() { debug("Error"); };
	//transfer.OnTimeout = []() { debug("Timeout"); };
	//transfer.OnByteReceived = []() { Serial.println(transfer.GetLastByteReceived()); };

	transfer.SetupForReceive(buffer, bufferLength);
	transfer.Start();
}

void loop()
{
	transfer.AsyncReceive();
}
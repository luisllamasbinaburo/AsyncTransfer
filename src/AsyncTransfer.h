#ifndef _AsyncTransfer_h
#define _AsyncTransfer_h

//#define ASYNC_TRANSFER_DEBUG

#include "Arduino.h"

#include "Constants.h"

typedef  void(*AsyncTransferCallback)();

typedef enum
{
	IDDLE,
	WAITING_START,
	RECEIVING_ID,
	RECEIVING_LENGTH,
	WAITING_DATA_START,
	RECEIVING_DATA,
	WAITING_DATA_END,
	RECEIVING_CHECKSUM,
	WAITING_END,
	MESSAGE_RECEIVED,
}  TRANSFER_STATUS;


struct AsyncTransferConfig
{
	bool AutoReset = true;
	bool SendAck = false;
	unsigned long TimeOut = 50;
};


class AsyncTransfer
{
public:
	AsyncTransferConfig Config;

	AsyncTransferCallback OnSuccess = nullptr;
	AsyncTransferCallback OnError = nullptr;
	AsyncTransferCallback OnTimeout = nullptr;
	AsyncTransferCallback OnByteReceived = nullptr;

	explicit AsyncTransfer(Stream& stream)
	{
		_stream = &stream;
		_status = TRANSFER_STATUS::IDDLE;
	}

	AsyncTransfer(Stream& stream, AsyncTransferConfig config)
	{
		_stream = &stream;
		_status = TRANSFER_STATUS::IDDLE;
		Config = config;
	}

	void SendHandShake(const bool waitEot)
	{
		_stream->write(C_STARTOFHEADING);

		if(waitEot)
		{
			_startTime = millis();
			while(!IsExpired())
			{
				while(_stream->available())
				{
					if(_stream->read() == C_ENDOFTRANSMISSION)
					{
						if(OnSuccess != nullptr) OnSuccess();
						return;
					}
				}
			}
			if(OnTimeout != nullptr) OnTimeout();
		}
		else
		{
			if(OnSuccess != nullptr) OnSuccess();
		}

	}

	template <typename T>
	void Send(const T& data, const uint16_t& dataLength = sizeof(T), uint8_t packedId = 0)
	{
		Send(data, dataLength, packedId);
	}

	void Send(const byte* data, const uint16_t dataLength, uint8_t packedId = 0)
	{
		_stream->write(C_STARTOFHEADING);
		_stream->write((byte)packedId);
		_stream->write(dataLength >> 8);
		_stream->write(dataLength & 0xFF);

		_stream->write(C_STARTOFTEXT);
		_stream->write(data, dataLength);
		_stream->write(C_ENDOFTEXT);

		_stream->write(AsyncTransfer::ChecksumFletcher16(data, dataLength));
		_stream->write(C_ENDOFTRANSMISSION);
	}

	void ReceiveHandShake(int timeOut, bool sendEot, AsyncTransferCallback okCallback, AsyncTransferCallback timeoutCallback = nullptr) const
	{
		const unsigned long startTime = millis();

		while(static_cast<unsigned long>(millis() - startTime) < timeOut)
		{
			if(_stream->read() == C_STARTOFHEADING)
			{
				if(sendEot) _stream->write(C_ENDOFTRANSMISSION);
				if(okCallback != nullptr) okCallback();
				return;
			}
		}

		if(timeoutCallback != nullptr) timeoutCallback();
	}

	template <typename T>
	void SetupForReceive(const T& data, const uint16_t& dataLength = sizeof(T))
	{
		_data_max_length = dataLength;
		_data = (byte*)&data;
	}

	void SyncReceive()
	{
		_startTime = millis();
		_status = WAITING_START;

		while(!IsExpired() && _status < MESSAGE_RECEIVED)
		{
			AsyncReceive();
		}
		if(IsExpired())
		{
			if(OnTimeout != nullptr) OnTimeout();
			Reset();
		}
	}

	void AsyncReceive()
	{
		if(_status == IDDLE) { return; }

		if(IsExpired()) Reset();

		if(_status > IDDLE && _status < MESSAGE_RECEIVED)
		{
			ReceiveAvailable();
		}
	}

	inline bool IsExpired() const
	{
		if(Config.TimeOut == 0) return false;
		return (static_cast<unsigned long>(millis() - _startTime) > Config.TimeOut);
	}

	void Start()
	{
		_status = WAITING_START;
		_data_index = 0;
		_startTime = millis();
	}

	void Stop()
	{
		_status = IDDLE;
	}

	template <typename T>
	void CopyPayloadTo(T& destin, uint16_t offset = 0)
	{
		for(auto index = 0; index < _payloadLength; index++)
		{
			((byte*)&destin)[index + offset ] = _data[index];
		}
	}

	TRANSFER_STATUS GetStatus() const
	{
		return _status;
	}

	uint8_t GetPacketId() const
	{
		return _packetId;
	}

	uint16_t GetPayloadLength() const
	{
		return _payloadLength;
	}

	byte GetLastByteReceived() const
	{
		return _lastByteReceived;
	}


private:
	Stream* _stream;
	TRANSFER_STATUS _status;
	byte _lastByteReceived;

	byte* _data;
	uint16_t _data_max_length;
	uint16_t _data_index;

	uint8_t _packetId;
	uint16_t _payloadLength;
	uint16_t _checkSum;

	unsigned long _startTime;

	void ReceiveAvailable()
	{
		while(_stream->available())
		{
			ProcessByte((byte)_stream->read());
		}
	}

	void ProcessByte(byte newData)
	{
		if(OnByteReceived != nullptr) OnByteReceived();

		switch(_status)
		{

		case WAITING_START:
		{
#ifdef ASYNC_TRANSFER_DEBUG
			Serial.print("WAITING_START ");
			Serial.print(C_STARTOFHEADING, HEX);
			Serial.print(" ");
			Serial.println(newData, HEX);
#endif

			if(newData == C_STARTOFHEADING) _status = RECEIVING_ID;
			break;
		}
		case RECEIVING_ID:
		{
#ifdef ASYNC_TRANSFER_DEBUG
			Serial.print("RECEIVING_ID ");
			Serial.println(newData, HEX);
#endif

			_packetId = newData;
			_payloadLength = 0;
			_data_index = 0;
			_status = RECEIVING_LENGTH;
			break;
		}
		case RECEIVING_LENGTH:
		{
#ifdef ASYNC_TRANSFER_DEBUG
			Serial.print("RECEIVING_LENGTH ");
			Serial.println(newData, HEX);
#endif

			_data_index++;
			if(_data_index >= 2)
			{
				_payloadLength = _lastByteReceived | newData << 8;
				if(_payloadLength <= _data_max_length) _status = WAITING_DATA_START;
				else HandleError();
			}
			break;
		}
		case WAITING_DATA_START:
		{
#ifdef ASYNC_TRANSFER_DEBUG
			Serial.print("WAITING_DATA_START ");
			Serial.print(C_STARTOFTEXT, HEX);
			Serial.print(" ");
			Serial.println(newData, HEX);
#endif

			if(newData == C_STARTOFTEXT)
			{
				_status = RECEIVING_DATA;
				_data_index = 0;
			}
			else HandleError();
			break;
		}
		case RECEIVING_DATA:
		{
#ifdef ASYNC_TRANSFER_DEBUG
			Serial.print("RECEIVING_DATA ");
			Serial.println(newData, HEX);
#endif

			_data[_data_index] = newData;
			_data_index++;
			if(_data_index >= _payloadLength) _status = WAITING_DATA_END;
			break;
		}
		case WAITING_DATA_END:
		{
#ifdef ASYNC_TRANSFER_DEBUG
			Serial.print("WAITING_DATA_END ");
			Serial.print(C_ENDOFTEXT, HEX);
			Serial.print(" ");
			Serial.println(newData, HEX);
#endif

			if(newData == C_ENDOFTEXT)
			{
				_status = RECEIVING_CHECKSUM;
				_data_index = 0;
			}
			else HandleError();
			break;
		}
		case RECEIVING_CHECKSUM:
		{
#ifdef ASYNC_TRANSFER_DEBUG
			Serial.print("RECEIVING_CHECKSUM ");
			Serial.println(newData, HEX);
#endif

			_data_index++;
			if(_data_index >= 2)
			{
				_checkSum = _lastByteReceived | newData << 8;
				_status = WAITING_END;
			}
			break;
		}
		case WAITING_END:
		{
#ifdef ASYNC_TRANSFER_DEBUG
			Serial.print("WAITING_END ");
			Serial.print(C_ENDOFTRANSMISSION, HEX);
			Serial.print(" ");
			Serial.println(newData, HEX);
#endif

			if(newData == C_ENDOFTRANSMISSION) ValidatePacket();
			else HandleError();
			break;
		}

		default:
			break;
		}

		_lastByteReceived = newData;
	}

	void ValidatePacket()
	{
		const auto isValid = ValidateChecksum();

		if(isValid)
		{
			_status = MESSAGE_RECEIVED;
			if(OnSuccess != nullptr) OnSuccess();
			if(Config.SendAck) _stream->write(C_ACKNOWLEDGE);

			Reset();
		}
		else
		{
			HandleError();
		}
	}

	inline bool ValidateChecksum()
	{
		return _checkSum == AsyncTransfer::ChecksumFletcher16(_data, _payloadLength);
	}

	static uint16_t ChecksumFletcher16(const uint8_t* buffer, size_t length)
	{
		uint8_t sum1 = 0;
		uint8_t sum2 = 0;

		for(int index = 0; index < length; ++index)
		{
			sum1 = ((uint16_t)(sum1 + buffer[index]) % 255);
			sum2 = ((uint16_t)(sum2 + sum1) % 255);
		}
		return (sum2 << 8) | sum1;
	}

	void HandleError()
	{
		if(OnError != nullptr) OnError();
		if(Config.SendAck) _stream->write(C_NEGATIVEACKNOWLEDGE);

		Reset();
	}

	void Reset()
	{
		_status = Config.AutoReset ? WAITING_START : IDDLE;
		_startTime = millis();
		_data_index = 0;
	}
};

#endif
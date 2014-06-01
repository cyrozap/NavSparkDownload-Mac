#pragma once
#include <termios.h>
#include <unistd.h>
#include <string>
unsigned GetTickCount();
class ScopeTimer
{
public:
	ScopeTimer()
	{
    startTickCount = GetTickCount();
	}

	~ScopeTimer()
	{

	}

	unsigned GetDuration()
	{
		endTickCount = GetTickCount();
//printf("s:%u, e:%u, t=%u\r\n", startTickCount, endTickCount, endTickCount - startTickCount);
		return endTickCount - startTickCount;
	}

protected:
	unsigned startTickCount;
	unsigned endTickCount;
};


// Serial.h
#define ASCII_CR        0x0D
#define ASCII_LF        0x0A

#define BINARY_HD1      0xA0
#define BINARY_HD2      0xA1

#define READ_ERROR		((DWORD)(-1))
#define ReadOK(len)			((len==READ_ERROR) ?0 :len)

class CSerial
{
public:
	CSerial();
	~CSerial();

	static int BaudrateTable[];
  static int GetBaudRateIndex(int b);

	std::string GetComPort() { return portName; }
	int GetBaudRate() { return baudRateIdx; }

	//Flow control
	bool Open(LPCSTR comPort, int bIdx);
	void Close();
	void ClearQueue();

	DWORD SendData(const void* buffer, DWORD bufferSize, bool blockTransfer = false, int delayDuration = 0);
	DWORD GetBinary(void* buffer, DWORD bufferSize, int timeout);
  	DWORD GetString(std::string& buffer, DWORD bufferSize, DWORD timeOut);
	void SetShowLog(bool b) { showLog = b; }
	bool ShowLog() { return showLog; }
protected:
	static speed_t PosixBaudrateTable[];
	static const int BaudrateTableSize;

  	int comDeviceHandle;
	bool isOpened;
  	int  baudRateIdx;
  	std::string portName;
	bool showLog;

	bool OpenByBaudrate(LPCSTR comPort, speed_t b, int baud);

};


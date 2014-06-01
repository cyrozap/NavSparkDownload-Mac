// download.cpp

#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <cstring>
#include <cstdlib>
#include <time.h>

#include <unistd.h>
#include <fcntl.h>   /* File control definitions */

#include "st_type.h"
#include "download.h"
#include "serial.h"

extern AppParameter g_appParam;
extern CSerial g_serial;
void Sleep(int ms);

class BinaryData
{
private:
	U08* dataPtr;
	int dataSize;
public:
	BinaryData() 
	{ 
		dataSize = 0; 
		dataPtr = NULL; 
	};

	BinaryData(LPCSTR filepath) 
	{ 
		dataSize = 0; 
		dataPtr = NULL;
		ReadFromFile(filepath);
	};

	BinaryData(U08 *data, int size) 
	{ 
		ReadFromMemory(data, size);
	};

	BinaryData(const BinaryData& src) 
	{ 
		dataSize = 0; 
		dataPtr = NULL;

		if(src.Ptr())
		{
			Alloc(src.Size());
			memcpy(dataPtr, src.Ptr(), dataSize);
		}
	};

	BinaryData(int size) 
	{ 
		dataSize = 0; 
		dataPtr = NULL;
		Alloc(size);
	};

	virtual ~BinaryData() 
	{ 
		Free(); 
	};

	void Free()
	{
		delete [] dataPtr;
		dataPtr = NULL;
		dataSize = 0;
	}

	void Alloc(int size)
	{
		if(dataPtr)
		{
			Free();
		}
		dataSize = size; 
		if(dataSize)
		{
			dataPtr = new U08[dataSize];
			memset(dataPtr, 0, dataSize);
		}
	}

	int ReadFromFile(LPCSTR filepath)
	{
    int fd;
    fd = open(filepath, O_RDONLY);
    if(fd == -1)
    {
      return 0;
    }

    dataSize = (int)lseek(fd, 0, SEEK_END); 
    lseek(fd, 0, SEEK_SET);

		if(dataSize > 0 && dataPtr)
		{
			Free();
		}
		dataPtr = new U08[dataSize];

		read(fd, dataPtr, dataSize);
		close(fd);
		return dataSize;
	};

	int ReadFromMemory(U08 *data, int size)
	{
		if(dataSize > 0 && dataPtr)
		{
			Free();
		}
		dataPtr = new U08[size];
		dataSize = size;
		memcpy(dataPtr, data, size);
		return dataSize;
	};

	U08 operator[](int iChar) const
	{
		return(dataPtr[iChar]);
	}

	BinaryData& operator=(const BinaryData& src)
	{
		dataSize = 0; 
		dataPtr = NULL;

		if(src.Ptr())
		{
			Alloc(src.Size());
			memcpy(dataPtr, src.Ptr(), dataSize);
		}

		return(*this);
	}

	int Size() const { return dataSize; }
	const U08* Ptr(int index = 0) const { return (dataPtr + index); }
	U08* GetBuffer(int index = 0) { return (dataPtr + index); }
	void Clear() { if(dataSize) memset(dataPtr, 0, dataSize); };
};

class BinaryCommand
{
private:
	enum { CommandExtraSize = 7, CommandHeaderSize = 4 };
	BinaryData	m_commandData;

public:
	BinaryCommand() {};
	BinaryCommand(int size) 
	{
		m_commandData.Alloc(CommandExtraSize + size);
	}
	BinaryCommand(const BinaryData& data)
	{ SetData(data); }

	void SetData(const BinaryData& data)
	{
		m_commandData.Alloc(CommandExtraSize + data.Size());
		memcpy(m_commandData.GetBuffer(CommandHeaderSize), data.Ptr(), data.Size());
	}
	U08* GetBuffer() 
	{ 
		U08 checkSum = 0;
		for(int i = 0; i < m_commandData.Size() - CommandExtraSize; ++i)
		{
			checkSum ^= *(m_commandData.Ptr(i + CommandHeaderSize));	
		}

		*m_commandData.GetBuffer(0) = 0xA0;
		*m_commandData.GetBuffer(1) = 0xA1;
		*m_commandData.GetBuffer(2) = HIBYTE(m_commandData.Size() - CommandExtraSize);
		*m_commandData.GetBuffer(3) = LOBYTE(m_commandData.Size() - CommandExtraSize);
		*m_commandData.GetBuffer(m_commandData.Size() - 3) = checkSum;
		*m_commandData.GetBuffer(m_commandData.Size() - 2) = 0x0D;
		*m_commandData.GetBuffer(m_commandData.Size() - 1) = 0x0A;

		return m_commandData.GetBuffer(0); 
	}
	void SetU08(int index, U08 data)
	{
		*m_commandData.GetBuffer(index + CommandHeaderSize - 1) = data;
	}
	void SetU16(int index, U16 data)
	{
		*m_commandData.GetBuffer(index + CommandHeaderSize - 1) = HIBYTE(data);
		*m_commandData.GetBuffer(index + CommandHeaderSize + 0) = LOBYTE(data);
	}	
	void SetU32(int index, U32 data)
	{
		*m_commandData.GetBuffer(index + CommandHeaderSize - 1) = HIBYTE(HIWORD(data));
		*m_commandData.GetBuffer(index + CommandHeaderSize + 0) = LOBYTE(HIWORD(data));
		*m_commandData.GetBuffer(index + CommandHeaderSize + 1) = HIBYTE(LOWORD(data));
		*m_commandData.GetBuffer(index + CommandHeaderSize + 2) = LOBYTE(LOWORD(data));
	}	
	int Size() const
	{ return m_commandData.Size(); }
};

Download::Download(void)
{
	fwFile = new BinaryData;
}

Download::~Download(void)
{
	delete fwFile;
}

void Download::ShowProgress(int prog, int total)
{
	if(!g_appParam.showProgress)
	{
		return;
	}
	printf("%d%%...   \r\n", 1 + (int)((double)prog / total * 98));
}

void Download::ShowStartProgress()
{
	if(!g_appParam.showProgress)
	{
		return;
	}
	printf("Start update firmware \"%s\" to %s using %d bps. \r\n", 
		g_appParam.fwPath, g_appParam.comPort, 
		CSerial::BaudrateTable[g_appParam.downloadBaudIndex]);
}

void Download::ShowEndProgress()
{
	if(!g_appParam.showProgress)
	{
		return;
	}
	printf("\r%d%%...   \r\n", 100);
}

//#define TEST_BAUD
int Download::DoDownload()
{
	ShowStartProgress();

#ifdef TEST_BAUD
	ScopeTimer t2;
	while(t2.GetDuration() < 5000)
	{
		std::string s;
	  g_serial.GetString(s, 1024, 1000);
	}
	printf("\r\nSend command.");
#endif

#ifndef TEST_BAUD
	BinaryCommand cmd(6);
	cmd.SetU08(1, 0x0B);
	cmd.SetU08(2, g_appParam.downloadBaudIndex);		//Baud Rate Index
	cmd.SetU08(3, 0);
	cmd.SetU08(4, 0);
	cmd.SetU08(5, 0);
	cmd.SetU08(6, 0);
/*
	BinaryCommand cmd(2);
	cmd.SetU08(1, 0x09);
	cmd.SetU08(2, 0x02);		//Binary message mode
*/
#else
	BinaryCommand cmd(4);
	cmd.SetU08(1, 0x05);
	cmd.SetU08(2, 0);		//Baud Rate Index
	cmd.SetU08(3, g_appParam.downloadBaudIndex);
	cmd.SetU08(4, 2);
#endif

	BinaryData ackCmd;
	int Retry = 3;
	Download::CmdErrorCode err;
	for(int i=0; i<Retry; ++i)
	{
		err =ExcuteBinaryCommand(&cmd, &ackCmd, 2000);
		if(Ack == err)
		{
			break;
		}
		if(g_appParam.showProgress)
			printf("Retry Download Command! \r\n");
	}
	if(Ack != err)
	{
		return 4;
	}

	if(g_appParam.downloadBaudIndex != g_appParam.baudRateIndex)
	{
		g_serial.Close();
		Sleep(1000);

		char sysBuf[128];
		//sprintf(sysBuf, "sudo stty -F %s %d", g_appParam.comPort, CSerial::BaudrateTable[g_appParam.downloadBaudIndex]);
		sprintf(sysBuf, "modprobe -r pl2303");
		system(sysBuf);
		printf("%s\r\n", sysBuf);
		Sleep(1000);
		sprintf(sysBuf, "modprobe pl2303");
		system(sysBuf);
		printf("%s\r\n", sysBuf);
		Sleep(1000);


		g_serial.Open(g_appParam.comPort, g_appParam.downloadBaudIndex);
		Sleep(1000);
		printf("Open %s in %d again.\r\n",g_appParam.comPort,g_appParam.downloadBaudIndex);
	}
	else
	{
		if(g_appParam.showProgress)
			printf("\r\nPass change baud rate.\r\n");
		Sleep(500);
	}

#ifdef TEST_BAUD
	ScopeTimer t;
	while(t.GetDuration() < 5000)
	{
		std::string s;
		g_serial.GetString(s, 1024, 1000);
	}
	printf("Test End\r\n");

	return 0;
#endif

	ShowProgress(0, 100);

	if(0 == fwFile->ReadFromFile(g_appParam.fwPath))
	{
		return 5;
	}


	for(int i=0; i<Retry; ++i)
	{
		err = SendBinsizeCmd();
		if(Ok == err)
		{
			break;
		}
		if(g_appParam.showProgress)
			printf("Retry BINSIZE Command! \r\n");
	}
	if(Ok != err)
	{
		return 6;
	}
	ShowProgress(1, 100);

	if(Ok != SendFwBuffer())
	{
		return 7;
	}

	std::string strAckCmd;
	if(End != GetTextAck(strAckCmd, 10000))
	{
		return 8;
	}
	ShowEndProgress();
	return 0;
}

Download::CmdErrorCode Download::SendBinsizeCmd()
{
	char strBinsizeCmd[96] = {0};
	std::string strAckCmd;
	
	U08 fwCheck = 0;
	for(int i=0; i<fwFile->Size(); ++i)
	{
		fwCheck += *(fwFile->Ptr(i));
	}

	U32 checkCode = fwFile->Size() + fwCheck;
  	sprintf(strBinsizeCmd, "BINSIZE = %d Checksum = %d %u ", fwFile->Size(), fwCheck, (unsigned int)checkCode);

	return ExcuteTextCommand(strBinsizeCmd, strAckCmd, 12000);
}


Download::CmdErrorCode Download::SendFwBuffer()
{
	const DWORD bufferSize = 8192;
	DWORD leftSize = fwFile->Size();
	DWORD sentSize = 0;
	while(leftSize)
	{ 
		DWORD sendSize = (leftSize >= bufferSize) ? bufferSize : leftSize;
		g_serial.ClearQueue();
		if(sendSize != g_serial.SendData(fwFile->Ptr(sentSize), sendSize, true))
		{
			printf("SendData fail! sendSize = %d\r\n", (int)sendSize);
			return Error;
		}

		std::string strAckCmd;
		CmdErrorCode err = GetTextAck(strAckCmd, 10000);
		if(err != Ok)
		{
			printf("GetTextAck fail! \r\n");
			return err;
		}
		sentSize += sendSize;
		leftSize -= sendSize;
		ShowProgress(sentSize, fwFile->Size());
	}
	return Ok;
}

Download::CmdErrorCode Download::ExcuteBinaryCommand(BinaryCommand* cmd, BinaryData* ackCmd, DWORD timeOut)
{
	U08* pCmd = cmd->GetBuffer();
	int inSize = cmd->Size();

	ackCmd->Alloc(1024);
	//g_serial.ClearQueue();
	g_serial.SendData(pCmd, inSize);

	ScopeTimer t;
	while(1)
	{
		if(t.GetDuration() > timeOut)
		{	//Time Out
			printf("Timeout:%u", (unsigned)timeOut);
			return Timeout;
		}

		ackCmd->Clear();
		DWORD len = g_serial.GetBinary(ackCmd->GetBuffer(), ackCmd->Size(), timeOut - t.GetDuration());

		if(len <= 0)
		{	
			continue;
		}

		DWORD cmdSize = MAKEWORD((*ackCmd)[3], (*ackCmd)[2]);
		if(cmdSize != len - 7)
		{	//Packet Size Error
			continue;
		}
		if( (*ackCmd)[0] != 0xa0 || (*ackCmd)[1] != 0xa1 ||
			(*ackCmd)[len-2] != 0x0d || (*ackCmd)[len-1] != 0x0a )
		{	//Format Error
			continue;
		}
		if( (*ackCmd)[4] == 0x83 && (*ackCmd)[5] == 0x0 )
		{	//ACK0
			continue;
		}
		if( (*ackCmd)[4] == 0x84 )
		{	//NACK
			return NACK;
		}
		if( (*ackCmd)[4] == 0x83 )
		{	//Get ACK
			return Ack;
		}
	}
	return Timeout;
}	

Download::CmdErrorCode Download::ExcuteTextCommand(LPCSTR strCmd, std::string& strAckCmd, DWORD timeOut)
{
	if(g_appParam.showProgress)
		printf("SendTextCmd:%s\r\n", strCmd);

	g_serial.ClearQueue();
	g_serial.SendData(strCmd, strlen(strCmd) + 1);
	return GetTextAck(strAckCmd, timeOut);
}	

Download::CmdErrorCode Download::GetTextAck(std::string& strAckCmd, DWORD timeOut)
{
	ScopeTimer t;
	while(1)
	{		
		if(t.GetDuration() > timeOut)
		{	//Time Out
			return Timeout;
		}

		strAckCmd.clear();
    	strAckCmd.reserve(1024);
		DWORD len = g_serial.GetString(strAckCmd, 1024, timeOut - t.GetDuration());
		//strAckCmd.ReleaseBuffer();

		if(len==0)
		{	
			continue;
		}

		if(0 == strAckCmd.compare("OK"))
		{
			return Ok;
		}
		if(0 == strAckCmd.compare("END"))
		{
			return End;
		}

		Sleep(50);
	}
	
	return Timeout;
}



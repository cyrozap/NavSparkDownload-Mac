// serial.cpp

#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <cstring>
#include <cstdlib>
#include <time.h>

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>   /* File control definitions */

#include "st_type.h"
#include "serial.h"

#ifndef CMSPAR
#define CMSPAR 0
#endif

void Sleep(int ms)
{
  usleep(ms * 1000);
}

#include <sys/time.h>
unsigned GetTickCount()
{
  struct timeval tv;
  if(gettimeofday(&tv, NULL) != 0)
    return 0;

  return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

int CSerial::BaudrateTable[] = {4800, 9600, 19200, 38400, 57600, 115200, 
#ifdef B230400
230400, 
#endif
#ifdef B460800
460800, 
#endif
#ifdef B921600
921600
#endif
};
speed_t CSerial::PosixBaudrateTable[] = {B4800, B9600, B19200, B38400, B57600, B115200, 
#ifdef B230400
B230400, 
#endif
#ifdef B460800
B460800, 
#endif
#ifdef B921600
B921600
#endif
};
const int CSerial::BaudrateTableSize = sizeof(CSerial::BaudrateTable) / sizeof(CSerial::BaudrateTable[0]);
const DWORD defaultSendUnit = 512;

int CSerial::GetBaudRateIndex(int b)
{
	for(int i = 0; i < BaudrateTableSize; ++i)
	{
		if(b == BaudrateTable[i])
		{
			return i;
		}
	}
	return -1;
}

CSerial::CSerial()
{
  	comDeviceHandle = -1;
	showLog = false;
} 

CSerial::~CSerial()
{
	Close();
}

bool CSerial::Open(LPCSTR comPort, int bIdx)
{
  	baudRateIdx = bIdx;
  	portName = comPort;
	return OpenByBaudrate(comPort, PosixBaudrateTable[baudRateIdx], BaudrateTable[baudRateIdx]);
}

bool CSerial::OpenByBaudrate(LPCSTR comPort, speed_t b, int baud)
{
/*
  char sysBuf[128];
  sprintf(sysBuf, "stty -F %s %d", comPort, baud);
  system(sysBuf);
  printf("%s\r\n", sysBuf);

  Sleep(500);
*/
	if ((comDeviceHandle = open(comPort, O_RDWR | O_NOCTTY | O_SYNC | O_NDELAY))==-1)	
  	{
    	printf("Unable to open %s \r\n", comPort);
    	return false;
	}
  	else
  	{
		if(!isatty(comDeviceHandle)) 
		{
			printf("%s is not a tty device.\r\n", comPort);
		}
		//Sleep(5000);
		termios options = {0};
		int n = tcgetattr(comDeviceHandle, &options);
		if(n < 0)
		{
			printf("tcgetattr error! %d\r\n",  n);
		}

        options.c_cflag = (options.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
        // disable IGNBRK for mismatched speed tests; otherwise receive break
        // as \000 chars
       	//options.c_iflag &= ~IGNBRK;         // disable break processing
		options.c_iflag &= ~(IGNBRK | BRKINT | ICRNL | IGNCR |
		                    INLCR | PARMRK | ISTRIP | IXON);


        options.c_lflag = 0;                // no signaling chars, no echo,
        options.c_oflag = 0;                // no remapping, no delays
        options.c_cc[VMIN]  = 0;            // read doesn't block
        options.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        options.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl


        options.c_cflag |= (HUPCL | CLOCAL | CREAD | CS8);// ignore modem controls,
#ifdef CMSPAR
        options.c_cflag &= ~(CMSPAR);;
#endif
        options.c_cflag &= ~(CRTSCTS);;
        options.c_cflag &= ~(PARENB | PARODD);      // shut off parity
        //options.c_cflag |= 8;
        options.c_cflag &= ~CSTOPB;
 
/*
		options.c_iflag &= ~(IGNBRK | BRKINT | ICRNL |
		                    INLCR | PARMRK | INPCK | ISTRIP | IXON);
		options.c_oflag = 0;
		options.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);
		options.c_cflag &= ~(CSIZE | PARENB);
		options.c_cflag |= CS8;
		options.c_cc[VMIN]  = 1;
		options.c_cc[VTIME] = 0;
*/
		n = cfsetispeed(&options, b);
		if(n  < 0)
		{
			printf("cfsetispeed error! %d\r\n",  n);
		}
		n = cfsetospeed(&options, b);
		if(n  < 0)
		{
			printf("cfsetispeed error! %d\r\n",  n);
		}

		//
		// Finally, apply the configuration
		//
		n =  tcsetattr(comDeviceHandle, TCSAFLUSH, &options);
		if(n  < 0)
		{
			 printf("tcsetattr error! %d\r\n",  n);
		}

    	isOpened = true;
	}
	if(ShowLog())
		printf("Open %s in %d bps succ.\r\n", comPort, baud);
	return true;
}

void CSerial::Close()
{
	if(!isOpened || comDeviceHandle == -1) 
	{
		return;
	}

	close(comDeviceHandle);
	comDeviceHandle = -1;
	isOpened = false;
	return;
}

DWORD CSerial::SendData(const void* buffer, DWORD bufferSize, bool blockTransfer, int delayDuration)
{
	if(!isOpened || comDeviceHandle == -1) 
	{
		return 0;
	}

	if(bufferSize>8 && ShowLog())
	{
		unsigned char* p = (unsigned char*)buffer;
	  	printf("Send Data : %02X %02X %02X %02X %02X %02X %02X %02X ... %02X %02X %02X %02X Len %d\r\n",
			p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], 
			p[bufferSize-4], p[bufferSize-3], p[bufferSize-2], p[bufferSize-1], (int)bufferSize);
	}

	if(ShowLog())
	{
		printf("Send bufferSize = %d b\r\n", (int)bufferSize);
	}
	
	int sentSize = 0;
	int sendSize = bufferSize;
	do 
	{
	  	int writeSize = write(comDeviceHandle, (char*)buffer + sentSize, sendSize);
	  	tcdrain(comDeviceHandle); 
		sentSize += writeSize;
		sendSize = bufferSize - sentSize;
		if(ShowLog())
		{
			//printf("writeSize = %d, sentSize = %d, sendSize = %d\r\n", writeSize, sentSize, sendSize);
		}
	} while(sentSize < bufferSize);

	if(ShowLog())
	{
		printf("sentSize = %d\r\n", sentSize);
	}
	return sentSize;	
}

void CSerial::ClearQueue()
{
	if(!isOpened || comDeviceHandle == -1) 
	{
		return;
	}
  tcflush(comDeviceHandle, TCIFLUSH);
  //tcflush(comDeviceHandle, TCIOFLUSH); // clear buffer
}

//Read until eos or empty.
DWORD CSerial::GetString(std::string& buffer, DWORD bufferSize, DWORD timeOut)
{ //Read a string until 0x00.
  DWORD totalSize = 0;
  ScopeTimer t;

  fcntl(comDeviceHandle, F_SETFL, FNDELAY);
	while(1)
	{ 
		if(t.GetDuration() > timeOut)
		{
			break;
		}

    	char buf[1] = {0};
		ssize_t  dwBytesDoRead = read(comDeviceHandle, buf, 1);
		if(ShowLog())
			printf("%c", *buf);

		if(dwBytesDoRead <= 0) 
		{
      Sleep(30);
			continue;
		}

		if(*buf == 0)
		{
			break;
		}

    buffer += *buf;
    if(++totalSize == bufferSize)
    {
      break;
    }
	}
	return totalSize;
}

DWORD CSerial::GetBinary(void *buffer, DWORD bufferSize, int timeout)
{	
	U08* bufferIter = (U08*)buffer;
	DWORD totalSize = 0;
  	ScopeTimer t;

  	//The FNDELAY option causes the read function to return 0 if no characters are available on the port.
  	fcntl(comDeviceHandle, F_SETFL, FNDELAY);
	while(totalSize < bufferSize - 1)
	{ 
		if(t.GetDuration() > timeout)
		{
			break;
		}
		ssize_t  dwBytesDoRead = 0;

		dwBytesDoRead = read(comDeviceHandle, bufferIter, 1);
		if(ShowLog() && dwBytesDoRead > 0)
		{
			printf(" %02X", *bufferIter);
		}

		if(dwBytesDoRead <= 0) 
		{
      		Sleep(30);
			continue;
		}
		if(totalSize == 0 && *bufferIter!=0xa0)
		{	//Binary data must start in 0xa0
			continue;
		}
		if(totalSize==0 && ShowLog())
			printf("\r\nStart Command Rec. \r\n");

		if(totalSize > 0)
		{	//not first char.
			if(*bufferIter==0xa1 && *(bufferIter-1)==0xa0)
			{
				bufferIter -= totalSize;
				*bufferIter = 0xa0; 
				++bufferIter;
				*bufferIter = 0xa1; 
				++bufferIter;
				totalSize = 2;
				continue;
			}
			else if(*bufferIter==0x0a && *(bufferIter-1)==0x0d)
			{ //End of line
				unsigned char *chk_ptr = bufferIter - totalSize;
				if (*chk_ptr == 0xa0)
				{ //Binary message
					DWORD tmp_len = *(chk_ptr + 2);
					tmp_len = tmp_len << 8 | *(chk_ptr+3);
					if (totalSize == tmp_len + 6) 
					{
						*(bufferIter+1) = 0;
						return totalSize + 1;
					}
				}
				else
				{ 
					return totalSize;
				}
			}
		}

    	++totalSize;
		if (totalSize >=  bufferSize - 1)
		{	//Buffer full
			*(bufferIter++) = 0;
			break;
		}
		++bufferIter;
	} //while(total < size - 1)
	return totalSize;
}


#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <cstring>
#include <cstdlib>
#include <time.h>

#include <unistd.h>

#include "st_type.h"
#include "download.h"
#include "serial.h"


CSerial g_serial;
AppParameter g_appParam;
void Sleep(int ms);

void ShowNavSparkHelp()
{
	printf("NavSparkDownloadCmd V1.0.3 build in %s %s\r\n", __DATE__, __TIME__);
	printf("NavSparkDownloadCmd can update image to NavSpark device.\r\n");
	printf("\r\n");
	printf("Usage: NavSparkDownloadCmd -p COMPORT -i IMAGE [OPTION]...\r\n");
	printf("\r\n");
	printf("  -p, --port       The com port for NavSpark connection, ex: /dev/ttyUSB0\r\n");
	printf("  -i, --image      The firmware image file to be updated. ex: prom.bin\r\n");
	printf("  -b, --baudrate   The connection baud rate for the NavSpark device. ex: 115200\r\n");
	printf("  -s, --speed      The baud rate to use for update. ex: 115200\r\n");
	printf("\r\n");

	printf(" Baud rate only support the follow values in --baudrate and --speed : \r\n");
	printf("  4800 bps\r\n");
	printf("  9600 bps\r\n");
	printf("  19200 bps\r\n");
	printf("  38400 bps\r\n");
	printf("  57600 bps\r\n");
	printf("  115200 bps (--speed default)\r\n");
	printf("  230400 bps\r\n");
	printf("  460800 bps\r\n");
	printf("  921600 bps\r\n");
	printf("\r\n");
	printf("Example: \r\n");
	printf("  NavSparkDownloadCmd -p COM3 -i prom.bin -b 9600\r\n");

}

void SetNavSparkParamDefault(AppParameter& p)
{
	memset(p.comPort, 0, sizeof(p.comPort));
	memset(p.fwPath, 0, sizeof(p.fwPath));
	p.baudRateIndex = 5;		// Using 115200 bps for connection.
	p.downloadBaudIndex = 5;	// Using 115200 bps for download.
	p.showProgress = 0;
}

int main (int argc, char** argv)
{
	static const struct option long_options[] =
	{
		{"port", required_argument, NULL, 'p'},
		{"baudrate", required_argument, NULL, 'b'},
		{"image", required_argument, NULL, 'i'},
		{"speed", required_argument, NULL, 's'},
		{"prompt", no_argument, NULL, 'P'},
		{ NULL, 0, NULL, 0 }
	};
	SetNavSparkParamDefault(g_appParam);

  	int iopt;
	int option_index = 0;
	while (1)
	{

    	iopt = getopt_long (argc, argv, "p:b:i:s:P", long_options, &option_index);
	  	if (iopt == EOF)
			break;
 
		switch (iopt)
		{
		case 'p':
			//_tprintf (_T("option -p with value `%s'\n"), optarg);
			strcpy(g_appParam.comPort, optarg);
			break;
			
		case 'b':
			//_tprintf (_T("option -b with value `%s'\n"), optarg);
			g_appParam.baudRateIndex = CSerial::GetBaudRateIndex(atoi(optarg));
			break;
			
		case 'i':
			//_tprintf (_T("option -i with value `%s'\n"), optarg);
			strcpy(g_appParam.fwPath, optarg);
			break;
			
		case 's':
			//_tprintf (_T("option -s with value `%s'\n"), optarg);
			g_appParam.downloadBaudIndex = CSerial::GetBaudRateIndex(atoi(optarg));
			break;

		case 'P':
			//_tprintf (_T("option -P\n"));
			g_appParam.showProgress = 1;
			break;
		
		default:
			ShowNavSparkHelp();
			return 0;
		}
	}
	g_serial.SetShowLog(g_appParam.showProgress==1);

	if(argc < 5)
	{
		ShowNavSparkHelp();
		return 0;
	}

	if(-1 == access(g_appParam.fwPath, F_OK))
	{
		printf("Can't find image file in \"%s\"\r\n", g_appParam.fwPath);
		return 1;
	}

	if(g_appParam.showProgress)
		printf("Check image %s pass.\r\n", g_appParam.fwPath);

	if(g_appParam.baudRateIndex < 0 || g_appParam.baudRateIndex > 8 || 
		g_appParam.downloadBaudIndex < 0 || g_appParam.downloadBaudIndex > 8)
	{
		printf("Unsupported baud rate for --baudrate or --speed.\r\n");
		return 2;
	}

	if(g_appParam.showProgress)
		printf("Check baud rate %d and %d pass.\r\n", CSerial::BaudrateTable[g_appParam.baudRateIndex],
        	CSerial::BaudrateTable[g_appParam.downloadBaudIndex]);

	if(!g_serial.Open(g_appParam.comPort, g_appParam.baudRateIndex))
	{
		printf("Can't open \"%s\"\r\n", g_appParam.comPort);
		return 3;
	}

	Download d;
	int n = d.DoDownload();
	g_serial.Close();
	if(n == 0)
	{
		printf("\r\nNavSpark image update finished.\r\n");
	}
	else
	{
		printf("\r\nNavSpark image failure. Error code : %d\r\n", n);
	}
	return 0;
}

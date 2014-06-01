#pragma once
#include <string>

struct AppParameter
{
	enum { ImageSize = 260 };
	enum { PortSize = 16 };

	char comPort[PortSize];
	int	baudRateIndex;
	char fwPath[ImageSize];
	int	downloadBaudIndex;
	int showProgress;
};
extern AppParameter g_appParam;

class BinaryCommand;
class BinaryData;

class Download
{
public:
	enum CmdErrorCode {
		Ack = 0,
		NACK,
		Timeout,
		Ok,
		End,
		Error,
	};

	Download(void);
	~Download(void);

	int DoDownload();

protected:
	BinaryData* fwFile;
	CmdErrorCode ExcuteBinaryCommand(BinaryCommand* cmd, BinaryData* ackCmd, DWORD timeOut = 2000);
  CmdErrorCode GetTextAck(std::string& strAckCmd, DWORD timeOut);
  CmdErrorCode ExcuteTextCommand(LPCSTR strCmd, std::string& strAckCmd, DWORD timeOut);

	CmdErrorCode SendBinsizeCmd();
	CmdErrorCode SendFwBuffer();
	void ShowProgress(int prog, int total);
	void ShowStartProgress();
	void ShowEndProgress();
};


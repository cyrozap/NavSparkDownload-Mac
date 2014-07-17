#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h> // needed for memset
#include <sys/time.h>
unsigned GetTickCount()
{
  struct timeval tv;
  if(gettimeofday(&tv, NULL) != 0)
    return 0;

  return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

#include <time.h>
void Sleep(unsigned long microseconds)
{
  struct timespec req;
  req.tv_sec  = microseconds / 1000000;
  req.tv_nsec = (microseconds * 1000) % 1000000000;
  nanosleep(&req, NULL);
}

void Flash(int fd)
{
  if (tcflush(fd, TCIFLUSH)) 
  {
    printf("Flash(%d) error!\r\n", fd);
  }
}

int Open(const char * port, int idx)
{
  int baudIdxTable[] = { B4800, B9600, B19200, B38400, B57600, B115200, B230400 };  
  int fd = -1;


  fd = open(port, O_RDWR | O_NONBLOCK);      
 
  struct termios tio = {0};
  tcgetattr(fd, &tio);

  tio.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP
                          |INLCR|IGNCR|ICRNL|IXON);
  tio.c_oflag &= ~OPOST;
  tio.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
  tio.c_cflag &= ~(CSIZE|PARENB);
  tio.c_cflag |= CS8;

  tio.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP |
                          INLCR  | IGNCR  | IXON);
  tio.c_cflag &= ~(CSIZE  | PARENB | PARODD | CSTOPB);

  tio.c_oflag = 0;
  tio.c_lflag = 0;

  tio.c_cc[VMIN]  = 1;
  tio.c_cc[VTIME] = 0;

  cfsetospeed(&tio,baudIdxTable[idx]);            // 115200 baud
  cfsetispeed(&tio,baudIdxTable[idx]);            // 115200 baud
  tcsetattr(fd, TCSANOW, &tio);
  printf("\r\nOpen %s in %d idx return %d.\r\n", port, idx, fd);

  return fd;
}

void ShowTty(int fd, int cCount)
{
  int cc=0;
  unsigned char c='D';
  fcntl(fd, F_SETFL, O_NONBLOCK); 
  printf("\r\n ShowTty(%d, %d) start.\r\n", fd, cCount);
  while (cc < cCount)
  {
    if (read(fd, &c, 1)>0)
    {
      printf("%c", c);      
      cc++;
    }
  }
  printf("\r\n ShowTty(%d, %d) exit.\r\n", fd, cCount);
}

void SendCmd(int fd, unsigned char* cmd, int len)
{
  Flash(fd);

  int rb = write(fd, cmd, len);
  tcdrain(fd); 
  printf("\r\nWrite bytes %d, change to 115200.\r\n", rb);
}

int main(int argc,char** argv)
{
  unsigned char idx = 1;
  char portName[128] = "/dev/ttyUSB0";
  const unsigned char boostSpeedIdx = 5;
  int tty_fd = -1;
  if(argc >= 3)
  { //default baud rate 9600 bps
    idx = atoi(argv[2]);
  }

  if(argc >= 2)
  {
    strcpy(portName, argv[1]);
  }
  printf("argc=%d\r\n", argc);
  printf("idx=%d\r\n", idx);
  printf("portName=%s\r\n", portName);

  tty_fd = Open(portName, idx);
  ShowTty(tty_fd, 1024);

  unsigned char cmd[11] = { 0xA0, 0xA1, 0x00, 0x04, 0x05, 0x00, (unsigned char)boostSpeedIdx, 0x02, 0x02 ,0x0D, 0x0A };
  //unsigned char cmd[11] = { 0xA0, 0xA1, 0x00, 0x04, 0x05, 0x00, 0x07, 0x00, 0x02 ,0x0D, 0x0A };
  SendCmd(tty_fd, cmd, sizeof(cmd));
  Sleep(1000);

  tcflush(tty_fd, TCIFLUSH);
  close(tty_fd);
  Sleep(1000);

  tty_fd = Open(portName, boostSpeedIdx);
  ShowTty(tty_fd, 1024);


  unsigned char cmd2[11] = { 0xA0, 0xA1, 0x00, 0x04, 0x05, 0x00, idx, 0x02, 0x04 ,0x0D, 0x0A };
  SendCmd(tty_fd, cmd2, sizeof(cmd2));
  Sleep(1000);


  tcflush(tty_fd, TCIFLUSH);
  close(tty_fd);
  Sleep(1000);

  tty_fd = Open(portName, idx);
  ShowTty(tty_fd, 1024);

  close(tty_fd);
}

#ifndef _FAM_H
#define _FAM_H

#define MAX_TCP_CONNECT         5       /* time in secs to get a connection */
#define MAX_TCP_READ            3       /* time in secs to wait for the DSO
                                           to respond to a read request */
#define BOOL                    int
#define TRUE                    1
#define FALSE                   0


//-----------------------------------------------------------------------------------

// open TCP socket
int tcpOpen(char* host, int port);

// close TCP socket
int tcpClose(int tcp_Handle);

// write to TCP socket
int tcpWrite(int tcp_Handle, char* buf, int len);

// read from TCP socket
int tcpRead(int tcp_Handle, char* buf, int len);

// write a byte to module
int NTCPwriteByte(int tcp_Handle, int addr, int data);

// write a short word(2byte) to module
int NTCPwriteShort(int tcp_Handle, int addr, int data);

// write a long word(4byte) to module
int NTCPwriteLong(int tcp_Handle, int addr, int data);

// read byte data from module(count = number of data)
int NTCPreadByte(int tcp_Handle, int addr, int count, int* data);

// read short word(2byte) data from module(count = number of data)
int NTCPreadShort(int tcp_Handle, int addr, int count, int* data);

// read long word(4byte) data from module(count = number of data)
int NTCPreadLong(int tcp_Handle, int addr, int count, int* data);

//------------------------------------------------------------------------------------

int FAMopen(char* ip);

void FAMclose(int tcp_Handle);

void FAMwriteFPGA(int tcp_Handle, int ch, int nbyte, int address, unsigned char* wdat);

void FAMreadFPGA(int tcp_Handle, int ch, int nbyte, int address, unsigned char* rdat);

void FAMsendtrig(int tcp_Handle);

void FAMwriteOFFSET(int tcp_Handle, int ch, int value);

void FAMarmFADC(int tcp_Handle);

int FAMreadFADCSTAT(int tcp_Handle);

int FAMreadPED(int tcp_Handle, int ch);

void FAMwriteFDLY(int tcp_Handle, int value);

void FAMwriteTHR(int tcp_Handle, int value);

void FAMreadFADC(int tcp_Handle, int ch, int* rdat);

void FAMwriteTOW(int tcp_Handle, int value);

//------------------------------------------------------------------------------------

#endif

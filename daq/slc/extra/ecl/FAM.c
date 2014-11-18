#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>   
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include "FAM.h"

//-----------------------------------------------------------------------------------

// connect to FAM
int FAMopen(char *ip)
{
  int tcp_Handle;
  char   gip[20];
  int    gport;
  
  strcpy(gip, ip);
  gport = 5000;

  tcp_Handle  =  tcpOpen(gip,gport);

  return tcp_Handle;
}


// disconnect from FAM
void FAMclose(int tcp_Handle)
{
  tcpClose(tcp_Handle);
}


// write to FPGA
void FAMwriteFPGA(int tcp_Handle, int ch, int nbyte, int address, unsigned char *wdat)
{
  char tcpBuf[1500];
  int count;
  int i;
	
  count = nbyte + 6;
	
  tcpBuf[0] = 1;
  tcpBuf[1] = nbyte & 0xFF;
  tcpBuf[2] = (nbyte >> 8) & 0xFF;
  tcpBuf[3] = ch & 0xFF;
  tcpBuf[4] = address & 0xFF;
  tcpBuf[5] = (address >> 8) & 0xFF;
	
  for (i = 0; i < nbyte; i++)  
    tcpBuf[i + 6] = wdat[i] & 0xFF;

  tcpWrite(tcp_Handle, tcpBuf, count);

  tcpRead(tcp_Handle, tcpBuf, 1);
}


// read from FPGA
void FAMreadFPGA(int tcp_Handle, int ch, int nbyte, int address, unsigned char *rdat)
{
  char tcpBuf[1500];
  int i;
	
  tcpBuf[0] = 2;
  tcpBuf[1] = nbyte & 0xFF;
  tcpBuf[2] = (nbyte >> 8) & 0xFF;
  tcpBuf[3] = ch & 0xFF;
  tcpBuf[4] = address & 0xFF;
  tcpBuf[5] = (address >> 8) & 0xFF;
	
  tcpWrite (tcp_Handle, tcpBuf, 6);

  tcpRead (tcp_Handle, tcpBuf, nbyte);
  
  for (i = 0; i < nbyte; i++)
    rdat[i] = tcpBuf[i] & 0xFF;
}


// send random trigger
void FAMsendtrig(int tcp_Handle)
{
  unsigned char wdat[2];	

  wdat[0] = 0;
  FAMwriteFPGA(tcp_Handle, 0, 1, 0x1, wdat);
}


// write offset DAC
void FAMwriteOFFSET(int tcp_Handle, int ch, int value)
{
  unsigned char wdat[2];	

  wdat[0] = value & 0xFF;
  wdat[1] = (value >> 8) & 0xFF;
  FAMwriteFPGA(tcp_Handle, ch - 1, 2, 0x2, wdat);
}


// arm FADC
void FAMarmFADC(int tcp_Handle)
{
  unsigned char wdat[2];	

  wdat[0] = 0;
  FAMwriteFPGA(tcp_Handle, 0, 1, 0x8, wdat);
}


// read FADC status
int FAMreadFADCSTAT(int tcp_Handle)
{
  unsigned char rdat[2];
  int value;

  FAMreadFPGA(tcp_Handle, 0, 1, 0x8, rdat);
	
  value = rdat[0] & 0xFF;
	
  return value;
}


// read pedestal
int FAMreadPED(int tcp_Handle, int ch)
{
  unsigned char wdat[2];
  unsigned char rdat[2];
  int value;

  // latch pedestal value
  wdat[0] = 0;
  FAMwriteFPGA(tcp_Handle, 0, 1, 0xB, wdat);
  
  // read pedestal
  FAMreadFPGA(tcp_Handle, ch - 1, 2, 0xA, rdat);
	
  value = rdat[1];
  value = value << 8;
  value = value + (rdat[0] & 0xFF);
  
  return value;
}


// write fadc delay
void FAMwriteFDLY(int tcp_Handle, int value)
{
  int rvalue;
  unsigned char wdat[2];	
  
  rvalue = value / 10;
  
  wdat[0] = rvalue & 0xFF;
  wdat[1] = (rvalue >> 8) & 0xFF;
  FAMwriteFPGA(tcp_Handle, 0, 2, 0xC, wdat);
}


// write discriminator threshold
void FAMwriteTHR(int tcp_Handle, int value)
{
  unsigned char wdat[2];	
  
  wdat[0] = value & 0xFF;
  wdat[1] = (value >> 8) & 0xFF;
  FAMwriteFPGA(tcp_Handle, 0, 2, 0xE, wdat);
}


// read FADC data
void FAMreadFADC(int tcp_Handle, int ch, int *rdat)
{
  unsigned char tdat[1024];
  int i;
  int j;
  
	
  for (i = 0; i < 4; i++) {
		
    FAMreadFPGA(tcp_Handle, ch - 1, 1024, i * 1024 + 0x2000, tdat);
		
    for (j = 0; j < 512; j++) {
      rdat[512 * i + j] = tdat[2 * j + 1];
      rdat[512 * i + j] = rdat[512 * i + j] << 8;
      rdat[512 * i + j] = rdat[512 * i + j] + (tdat[2 * j] & 0xFF);
    }
  }
}


// write trigger output pulse width
void FAMwriteTOW(int tcp_Handle, int value)
{
  unsigned char wdat[2];	
  
  wdat[0] = value & 0xFF;
  FAMwriteFPGA(tcp_Handle, 0, 1, 0x9, wdat);
}

//----------------------------------------------------------------------------------- 

int tcpOpen(char *host, int port)
{
  struct sockaddr_in serv_addr;
  int tcp_Handle;
  const int disable = 1;
  int rc;

  /*
   * Fill in the structure "serv_addr" with the address of the
   * server that we want to connect with.
   */
        
  //bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family      = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr(host);
  serv_addr.sin_port        = htons(port);
    
  /*
   * Open a TCP socket (an Internet stream socket).
   */
        
  if ( (tcp_Handle = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("client: can't open stream socket\n");
    return -1;
  }

  /* Bind socket with eth2 device */

  rc = setsockopt(tcp_Handle,SOL_SOCKET, SO_BINDTODEVICE,"eth2\x00",strlen("eth2\x00")+1);
  if(rc < 0){
    printf("Error in binding socket with eth2: rc= %d \n",rc);
    return -1;
  }

  /* turning off TCP NAGLE algorithm : if not, there is a delay 
     problem (up to 200ms) when packet size is small */ 

  setsockopt(tcp_Handle, IPPROTO_TCP,TCP_NODELAY,(char *) &disable, sizeof(disable)); 

  /*
   * Connect to the server.
   */

  if (connect(tcp_Handle, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    printf("client: can't connect to server\n");
    printf("ip %s , port %d \n",host,port);
    printf("error number is %d \n", connect(tcp_Handle, (struct sockaddr *) &serv_addr,sizeof(serv_addr)));

    return -2;
  } 
  
  return (tcp_Handle);
}

int tcpClose(int tcp_Handle) 
{
  close(tcp_Handle);

  return 0;
}


int tcpWrite(int tcp_Handle, char *buf,int len)
{
  int result, bytes_more, bytes_xferd;
  char *idxPtr;

  //	BOOL eoi_flag	= TRUE;

  bytes_more = len;
  idxPtr = buf;
  bytes_xferd = 0;
  while (1) {
    /* then write the rest of the block */
    idxPtr = buf + bytes_xferd;
    result=write (tcp_Handle, (char *) idxPtr, bytes_more);

    if (result<0) {
      printf("Could not write the rest of the block successfully, returned: %d\n",bytes_more);
	
      return -1;
    }
    
    bytes_xferd += result;
    bytes_more -= result;
    
    if (bytes_more <= 0)
      break;
  }

  return 0;

}


int tcpRead(int tcp_Handle, char *buf, int len)
{
  int result, accum, space_left, bytes_more, buf_count;
  //int i;
  char *idxPtr;

  fd_set rfds;
  //struct timeval tval;
  //tval.tv_sec = MAX_TCP_READ;
  //tval.tv_usec = 0;

  FD_ZERO(&rfds);
  FD_SET(tcp_Handle, &rfds);

  if (buf==NULL)
    return -1;

  //memset(buf, 0, len);

  idxPtr = buf;

  buf_count = 0;
  space_left = len;
  while (1) {
    /* block here until data is received of timeout expires */
    /*
    //      result = select((tcp_Handle+1), &rfds, NULL, NULL, &tval);
    if (result < 0) {
    printf("Read timeout\n");
    return -1;
    }

    printf("Passed Timeout  \n");
    */
    
    /* read the block */
    accum = 0;
    while (1) {
      idxPtr = buf + (buf_count + accum);
      bytes_more = space_left;
      
      if ((result = read(tcp_Handle, (char *) idxPtr, (bytes_more>2048)?2048:bytes_more)) < 0) {
	printf("Unable to receive data from the server.\n");
	return -1;
      }
      
      accum += result;
      if ((accum + buf_count) >= len)
                                break;
      /* in case data is smaller than wanted on */
      if(result<bytes_more) {
	printf("wanted %d got %d \n",bytes_more,result);
	return accum+buf_count;
      }
    }
    
    buf_count += accum;
    space_left -= accum;

    if (space_left <= 0)
      break;
  }

  return buf_count;
}


// write a byte to module
int NTCPwriteByte(int tcp_Handle, int addr, int data)
{
  char tcpBuf[4];
  int value;
	
  tcpBuf[0] = data & 0xFF;
  tcpBuf[1] = 0;
  tcpBuf[2] = addr & 0xFF;
  tcpBuf[3] = (addr >> 8) & 0x7F;

  tcpWrite (tcp_Handle, tcpBuf, 4);

  tcpRead (tcp_Handle, tcpBuf, 1);

  value = tcpBuf[0] & 0xFF;

  return value;
}


// write a short word(2byte) to module

int NTCPwriteShort(int tcp_Handle,int addr, int data)
{
  char tcpBuf[4];
  int i;
  int saddr;
  int value;
  int tmp;
	
  value = 0;

  for (i = 0; i < 2; i++) {

    saddr = addr + 1 - i;
	
    tcpBuf[0] = (data >> (8 - 8 * i)) & 0xFF;
    tcpBuf[1] = 0;
    tcpBuf[2] = saddr & 0xFF;
    tcpBuf[3] = (saddr >> 8) & 0x7F;

    tcpWrite (tcp_Handle, tcpBuf, 4);

    tcpRead (tcp_Handle, tcpBuf, 1);

    tmp = tcpBuf[0] & 0xFF;
    value = value + (tmp << (8 - 8 * i));
  }

  return value;
}


// write a long word(4byte) to module

int NTCPwriteLong(int tcp_Handle,int addr, int data)
{
  char tcpBuf[4];
  int i;
  int saddr;
  int value;
  int tmp;
	
  value = 0;

  for (i = 0; i < 4; i++) {

    saddr = addr + 3 - i;
	
    tcpBuf[0] = (data >> (24 - 8 * i)) & 0xFF;
    tcpBuf[1] = 0;
    tcpBuf[2] = saddr & 0xFF;
    tcpBuf[3] = (saddr >> 8) & 0x7F;

    tcpWrite (tcp_Handle, tcpBuf, 4);

    tcpRead (tcp_Handle, tcpBuf, 1);

    tmp = tcpBuf[0] & 0xFF;
    value = value + (tmp << (24 - 8 * i));
  }

  return value;
}


// read byte data from module(count = number of data)
int NTCPreadByte(int tcp_Handle,int addr, int count, int *data)
{
  char tcpBuf[1024];
  int chunk;
  int rem;
  int i, j;
  int saddr;

  chunk = count / 1024;
  rem = count % 1024;
  
  saddr = addr;

  // repeat for # of chunk
  for (i = 0; i < chunk; i++) {
    tcpBuf[0] = 0;
    tcpBuf[1] = 4;
    tcpBuf[2] = saddr & 0xFF;
    tcpBuf[3] = ((saddr >> 8) & 0x7F) | 0x80;

    tcpWrite (tcp_Handle, tcpBuf, 4);

    tcpRead (tcp_Handle, tcpBuf, 1024);
	
    for (j = 0; j < 1024; j++)
      data[1024 * i + j] = tcpBuf[j] & 0xFF;

    saddr = saddr + 1024;
  }

  // do it for remainings
  if (rem) {
    tcpBuf[0] = rem & 0xFF;
    tcpBuf[1] = (rem >> 8) & 0xFF;
    tcpBuf[2] = saddr & 0xFF;
    tcpBuf[3] = ((saddr >> 8) & 0x7F) | 0x80;

    tcpWrite (tcp_Handle, tcpBuf, 4);

    tcpRead (tcp_Handle, tcpBuf, rem);
	
    for (j = 0; j < rem; j++)
      data[1024 * chunk + j] = tcpBuf[j] & 0xFF;
  }

  return 0;
}


// read short word(2byte) data from module(count = number of data)

int NTCPreadShort(int tcp_Handle,int addr, int count, int *data)
{
  char tcpBuf[1024];
  int chunk;
  int rem;
  int i, j;
  int saddr;
  int tmp;

  chunk = count / 512;
  rem = count % 512;
  
  saddr = addr;

  // repeat for # of chunk
  for (i = 0; i < chunk; i++) {
    tcpBuf[0] = 0;
    tcpBuf[1] = 4;
    tcpBuf[2] = saddr & 0xFF;
    tcpBuf[3] = ((saddr >> 8) & 0x7F) | 0x80;

    tcpWrite (tcp_Handle, tcpBuf, 4);

    tcpRead (tcp_Handle, tcpBuf, 1024);
	
    for (j = 0; j < 512; j++) {
      tmp = tcpBuf[2 * j + 1] & 0xFF;
      data[512 * i + j] = tmp << 8;
      tmp = tcpBuf[2 * j] & 0xFF;
      data[512 * i + j] = data[512 * i + j] + tmp;
    }

    saddr = saddr + 1024;
  }

  // do it for remainings
  if (rem) {
    tcpBuf[0] = (rem << 1) & 0xFF;
    tcpBuf[1] = (rem >> 7) & 0xFF;
    tcpBuf[2] = saddr & 0xFF;
    tcpBuf[3] = ((saddr >> 8) & 0x7F) | 0x80;

    tcpWrite (tcp_Handle, tcpBuf, 4);

    tcpRead (tcp_Handle, tcpBuf, rem * 2);
	
    for (j = 0; j < rem; j++) {
      tmp = tcpBuf[2 * j + 1] & 0xFF;
      data[512 * chunk + j] = tmp << 8;
      tmp = tcpBuf[2 * j] & 0xFF;
      data[512 * chunk + j] = data[512 * chunk + j] + tmp;
    }
  }

  return 0;
}


// read long word(4byte) data from module(count = number of data)
int NTCPreadLong(int tcp_Handle,int addr, int count, int *data)
{
  char tcpBuf[1024];
  int chunk;
  int rem;
  int i, j;
  int saddr;
  int tmp;

  chunk = count / 256;
  rem = count % 256;
  
  saddr = addr;

  // repeat for # of chunk
  for (i = 0; i < chunk; i++) {
    tcpBuf[0] = 0;
    tcpBuf[1] = 4;
    tcpBuf[2] = saddr & 0xFF;
    tcpBuf[3] = ((saddr >> 8) & 0x7F) | 0x80;

    tcpWrite (tcp_Handle, tcpBuf, 4);

    tcpRead (tcp_Handle, tcpBuf, 1024);
	
    for (j = 0; j < 256; j++) {
      tmp = tcpBuf[4 * j + 3] & 0xFF;
      data[256 * i + j] = tmp << 24;
      tmp = tcpBuf[4 * j + 2] & 0xFF;
      data[256 * i + j] = data[256 * i + j] + (tmp << 16);
      tmp = tcpBuf[4 * j + 1] & 0xFF;
      data[256 * i + j] = data[256 * i + j] + (tmp << 8);
      tmp = tcpBuf[4 * j] & 0xFF;
      data[256 * i + j] = data[256 * i + j] + tmp;
    }

    saddr = saddr + 1024;
  }

  // do it for remainings
  if (rem) {
    tcpBuf[0] = (rem << 2) & 0xFF;
    tcpBuf[1] = (rem >> 6) & 0xFF;
    tcpBuf[2] = saddr & 0xFF;
    tcpBuf[3] = ((saddr >> 8) & 0x7F) | 0x80;

    tcpWrite (tcp_Handle, tcpBuf, 4);

    tcpRead (tcp_Handle, tcpBuf, rem * 4);
	
    for (j = 0; j < rem; j++) {
      tmp = tcpBuf[4 * j + 3] & 0xFF;
      data[256 * chunk + j] = tmp << 24;
      tmp = tcpBuf[4 * j + 2] & 0xFF;
      data[256 * chunk + j] = data[256 * chunk + j] + (tmp << 16);
      tmp = tcpBuf[4 * j + 1] & 0xFF;
      data[256 * chunk + j] = data[256 * chunk + j] + (tmp << 8);
      tmp = tcpBuf[4 * j] & 0xFF;
      data[256 * chunk + j] = data[256 * chunk + j] + tmp;
    }
  }

  return 0;
}

//----------------------------------------------------------------------------------- 


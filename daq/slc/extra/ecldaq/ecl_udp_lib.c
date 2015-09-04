#include <stdio.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "ecl_udp_lib.h"


int ecl_udp_init(void)
{
  return ecl_udp_init_ip(str_ecl_remote_host_name);
}

int ecl_udp_init_ip(const char *pzIP_ADDR)
{  
  //  struct hostent *pHostInfo;
  struct sockaddr_in addr;
 
  if( (ecl_udp_socket=socket(AF_INET, SOCK_DGRAM,0)) <0 )
    {
      ecl_udp_lib_debug?printf("socket() fault (%d)\n", errno):0;
      return 2;
    }
			     
  addr.sin_family=AF_INET;
  if( (addr.sin_addr.s_addr = inet_addr(str_ecl_local_host_name)) == INADDR_NONE)
    {
      ecl_udp_lib_debug?printf("inet_addr() fault (%d)\n", errno):0;
      return 3;
    }
  addr.sin_port=htons(ECL_COL_UDP_PORT);
  memset(&(addr.sin_zero),'\0',8);
 
  if( bind(ecl_udp_socket, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
      ecl_udp_lib_debug?printf("bind() fault (%d)\n", errno):0;
      return 4;
    }

  dest_addr.sin_family=AF_INET;
  if( (dest_addr.sin_addr.s_addr = inet_addr(pzIP_ADDR)) == INADDR_NONE)
    {
      ecl_udp_lib_debug?printf("inet_addr() fault (%d)\n", errno):0;
      return 5;
    }
  dest_addr.sin_port=htons(ECL_COL_UDP_PORT);
  memset(&(dest_addr.sin_zero),'\0',8);

  ecl_udp_lib_debug?printf("ecl_udp_init() OK\n"):0;

  ecl_udp_packet_cnt=0;
  return 0;
}

int ecl_udp_read_reg( unsigned short int reg_num, unsigned short int *reg_data)
{
  int rc;
  unsigned short int send_buf[4];
  unsigned short int recv_buf[5];

  send_buf[0]=ECL_UDP_PROTOCOL_ID;
  send_buf[1]=ecl_udp_packet_cnt++;
  send_buf[2]=0;
  send_buf[3]=reg_num;

  if((rc=sendto(ecl_udp_socket,(char*)send_buf, 8, 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr))) != 8)
    {
      ecl_udp_lib_debug?printf("sendto() fault (%d)\n", errno):0;
      return 1;
    }

  if((rc=recvfrom(ecl_udp_socket, (char*)recv_buf, 10, 0, NULL, NULL)) != 10)
    {
      ecl_udp_lib_debug?printf("recvfrom() fault (%d)\n", errno):0;
      return 2;
    }
  
  if(recv_buf[0]!=0xABCD || recv_buf[1]!=send_buf[1])
    {
      ecl_udp_lib_debug?printf("Bad received header : %04X %04X (%04X %04X must be) \n", recv_buf[0], recv_buf[1], 
			       send_buf[0], send_buf[1]):0;
      return 3;
    }

  *reg_data = recv_buf[4];

  ecl_udp_lib_debug?printf("ecl_udp_read_reg(): [%04X]=>%04X\n",reg_num,*reg_data):0;

  return 0;
}


int ecl_udp_write_reg( unsigned short int reg_num, unsigned short int reg_data)
{
  int rc;
  unsigned short int send_buf[5];
  unsigned short int recv_buf[5];

  send_buf[0]=ECL_UDP_PROTOCOL_ID;
  send_buf[1]=ecl_udp_packet_cnt++;
  send_buf[2]=1;
  send_buf[3]=reg_num;
  send_buf[4]=reg_data;

  if((rc=sendto(ecl_udp_socket,(char*)send_buf, 10, 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr))) != 10)
    {
      ecl_udp_lib_debug?printf("sendto() fault (%d)\n", errno):0;
      return 1;
    }

  if((rc=recvfrom(ecl_udp_socket, (char*)recv_buf, 10, 0, NULL, NULL)) != 10)
    {
      ecl_udp_lib_debug?printf("recvfrom() fault (%d)\n", errno):0;
      return 2;
    }

  if(recv_buf[0]!=0xABCD || recv_buf[1]!=send_buf[1])
    {
      ecl_udp_lib_debug?printf("Bad received header : %04X %04X (%04X %04X must be) \n", recv_buf[0], recv_buf[1],
                               send_buf[0], send_buf[1]):0;
      return 3;
    }

  ecl_udp_lib_debug?printf("ecl_udp_read_reg(): [%04X]<=%04X\n", reg_num, reg_data):0;

  return 0;
}

int ecl_udp_write_mem( unsigned int addr, unsigned short int length, unsigned short int *buf)
{
  int rc;
  unsigned short int send_buf[800];
  unsigned short int recv_buf[6];
  int l;

  send_buf[0]=ECL_UDP_PROTOCOL_ID;
  send_buf[1]=ecl_udp_packet_cnt++;
  send_buf[2]=3;
  *(unsigned int*)(send_buf+3)=addr;
  send_buf[5]=length;
  memcpy(send_buf+6, buf,length*2); 

  l=12+length*2;

  if((rc=sendto(ecl_udp_socket,(char*)send_buf, l, 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr))) != l)
    {
      ecl_udp_lib_debug?printf("sendto() fault (%d)\n", errno):0;
      return 1;
    }

  if((rc=recvfrom(ecl_udp_socket, (char*)recv_buf, 12, 0, NULL, NULL)) != 12)
    {
      ecl_udp_lib_debug?printf("recvfrom() fault (%d)\n", errno):0;
      return 2;
    }

  if(recv_buf[0]!=ECL_UDP_PROTOCOL_ID || recv_buf[1]!=send_buf[1])
    {
      ecl_udp_lib_debug?printf("Bad received header : %04X %04X (%04X %04X must be) \n", recv_buf[0], recv_buf[1],
                               send_buf[0], send_buf[1]):0;
      return 3;
    }

  ecl_udp_lib_debug?printf("ecl_udp_read_reg() OK\n"):0;

  return 0;
}


int ecl_udp_read_mem( unsigned int addr, unsigned short int length, unsigned short int *buf)
{
  int rc;
  unsigned short int send_buf[6];
  unsigned short int recv_buf[800];
  int l;

  send_buf[0]=ECL_UDP_PROTOCOL_ID;
  send_buf[1]=ecl_udp_packet_cnt++;
  send_buf[2]=2;
  *(unsigned int*)(send_buf+3)=addr;
  send_buf[5]=length;

  l=12+length*2;

  if((rc=sendto(ecl_udp_socket,(char*)send_buf, 12, 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr))) != 12)
    {
      ecl_udp_lib_debug?printf("sendto() fault (%d)\n", errno):0;
      return 1;
    }

  if((rc=recvfrom(ecl_udp_socket, (char*)recv_buf, l, 0, NULL, NULL)) != l)
    {
      ecl_udp_lib_debug?printf("recvfrom() fault (%d)\n", errno):0;
      return 2;
    }

  if(recv_buf[0]!=ECL_UDP_PROTOCOL_ID || recv_buf[1]!=send_buf[1])
    {
      ecl_udp_lib_debug?printf("Bad received header : %04X %04X (%04X %04X must be) \n", recv_buf[0], recv_buf[1],
                               send_buf[0], send_buf[1]):0;
      return 3;
    }

  memcpy(buf, recv_buf+6, length*2);

  ecl_udp_lib_debug?printf("ecl_udp_read_reg() OK\n"):0;

  return 0;
}

void ecl_udp_close() {
  close(ecl_udp_socket);
}

int ecl_udp_socket;
int ecl_udp_lib_debug;
char str_ecl_remote_host_name[]="192.168.1.12";
//char str_ecl_remote_host_name[]="192.168.1.13";
char str_ecl_local_host_name[]="192.168.1.61";
struct sockaddr_in dest_addr;

unsigned short int ecl_udp_packet_cnt;


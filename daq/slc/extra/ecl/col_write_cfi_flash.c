#include <stdio.h>
//#include <errno.h>
//#include <time.h>
//#include <unistd.h>
//#include "ecl_udp_lib.h"
//#include "ecl_collector.h"
//#include "ecl_collector_lib.h"

#include<string.h>  
#include<sys/socket.h>  
#include<arpa/inet.h>
#include<unistd.h>

#define ECL_TCP_STATUS_PORT (6001)

char buf[1000000];

int col_write_cfi_flash(int argn, char**argv)
{
  //  char default_ip[]="192.168.1.12";
  int bad_args_flag = 0;

  int sock;
  struct sockaddr_in server;
  char message [100], server_reply[1004];
  int recv_len=0;

  unsigned int mem_addr;
  int file_len;

  int rc, bytes_read, bytes_to_read;
  FILE *fd;

  if( argn!=4 )
   {
     printf("Incorrect number of arguments (%d)!\n", argn);
     bad_args_flag = 1;
   }

  if(sscanf(argv[2],"%x",&mem_addr)!=1)
    {
      printf("MEM_ADDR!\n");
      bad_args_flag = 1;
    }

  if(bad_args_flag)
   {
     printf("Syntax: col_write_cfi_flash IP_ADDR MEM_ADDR ECLDSP_FILE\n\n");
     return 1;
   }

  if( (fd=fopen(argv[3],"rb"))==NULL)
    {
      printf("Can not open file '%s'\n", argv[2]);
      return 1;
    }

  if((rc=fread(buf, 1, 1, fd))<=0)
    {
      printf("Can not read file '%s'\n", argv[2]);
      return 1;
    }

  if(mem_addr==0xA0000000 )
    {
      if(buf[0]=='S')
	printf("Writing to CPU boot memory...\n"
	       "File ""%s"" seems valid SREC file\n ", argv[2]);
      else
	{
	printf("Writing to CPU boot memory STOPPED:\n"
	       "File ""%s"" seems invalid SREC file\n ", argv[2]);
	return 1;
      }
    }
  
  
  fseek(fd, 0, SEEK_END);
  file_len = ftell(fd);
  fseek(fd, 0, SEEK_SET);


  if(file_len>1000000)
    {
      printf("File is too long (%d)!\n", file_len);
      return 1;
    }
  //Create socket
  sock = socket(AF_INET , SOCK_STREAM , 0);
  if (sock == -1)
    {
      printf("Fault: cound'n create socket\n");
      close(sock);
      return 1;
    }
  //  puts("Socket created");

  server.sin_addr.s_addr = inet_addr(argv[1]);
  server.sin_family = AF_INET;
  server.sin_port = htons( ECL_TCP_STATUS_PORT );

  //Connect to remote server
  if (connect(sock , (struct sockaddr *)&server, sizeof(server)) < 0)
    {
      printf("Fault: connection error\n");
      close(sock);
      return 1;
    }

  message[0]='F';
  message[1]=0;
  message[2]=0;
  message[3]=0;
  *(int*)(message+4) = htonl(mem_addr);
  *(int*)(message+8) = htonl(file_len);
  
  printf("Erasing flash memory...\n");

  if( send(sock, message, 12, 0) < 0)
    {
      puts("Fault: send error\n\n");
      close(sock);
      return 1;
    }

  bytes_to_read = file_len;
  bytes_read = 0;
  do
    {
      if((rc=fread(buf+bytes_read, 1, bytes_to_read, fd))>0)
	bytes_read += rc;
      else
	{
	  printf("read() error!\n");
	  close(sock);
	  return 1;
	}
      
    }
  while( bytes_read<bytes_to_read );

  // wait for Erase done message
  do{
    if( (recv_len=recv(sock , server_reply , 1000 , 0)) < 0)
      {
	puts("Fault: recv failed\n\n");
	close(sock);
	return 1;
      }
    server_reply[recv_len]=0;
  }
  while(  server_reply[recv_len-1]!=0 );

  //  server_reply[recv_len]=0;
  printf(server_reply);


  printf("Uploading %d bytes of data...\n", file_len);

  if( send(sock, buf, file_len, 0) < 0)
    {
      puts("Fault: send error\n\n");
      close(sock);
      return 1;
    }

  printf("Writing %d bytes to Flash memory...\n", file_len);

  // wait for Write DONE message
  do{
    if( (recv_len=recv(sock , server_reply , 1000 , 0)) < 0)
      {
	puts("Fault: recv failed\n\n");
	close(sock);
	return 1;
      }
    server_reply[recv_len]=0;
  }
  while(  server_reply[recv_len-1]!=0 );

  printf(server_reply);
  close(sock);

  return 0;
}

#ifdef EXE_MODE
int main(int argn, char**argv)
{
  return col_write_cfi_flash(argn, argv);
}
#endif

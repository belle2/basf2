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

const char* col_write_cfi_flash(const char* ip_addr, int mem_addr, const char* filename)
{
  int sock;
  struct sockaddr_in server;
  static char server_reply[1004];
  char message[100];
  int recv_len = 0;
  int file_len;
  int rc, bytes_read, bytes_to_read;
  FILE *fd;

  if ((fd = fopen(filename,"rb")) == NULL) {
    printf("Can not open file '%s'\n", filename);
    return NULL;
  }
  if ((rc = fread(buf, 1, 1, fd)) <= 0) {
    printf("Can not read file '%s'\n", filename);
    return NULL;
  }

  if (mem_addr == 0xA0000000) {
    if (buf[0]=='S') {
      printf("Writing to CPU boot memory...\n"
	     "File ""%s"" seems valid SREC file\n ", filename);
    } else {
      printf("Writing to CPU boot memory STOPPED:\n"
	     "File ""%s"" seems invalid SREC file\n ", filename);
      return NULL;
    }
  }

  fseek(fd, 0, SEEK_END);
  file_len = ftell(fd);
  fseek(fd, 0, SEEK_SET);

  if (file_len > 1000000) {
    printf("File is too long (%d)!\n", file_len);
    return NULL;
  }
  //Create socket
  sock = socket(AF_INET , SOCK_STREAM , 0);
  if (sock == -1) {
    printf("Fault: cound'n create socket\n");
    close(sock);
    return NULL;
  }
  //  puts("Socket created");

  server.sin_addr.s_addr = inet_addr(ip_addr);
  server.sin_family = AF_INET;
  server.sin_port = htons(ECL_TCP_STATUS_PORT );

  //Connect to remote server
  if (connect(sock , (struct sockaddr *)&server, sizeof(server)) < 0) {
    printf("Fault: connection error\n");
    close(sock);
    return NULL;
  }

  message[0] = 'F';
  message[1] = 0;
  message[2] = 0;
  message[3] = 0;
  *(int*)(message+4) = htonl(mem_addr);
  *(int*)(message+8) = htonl(file_len);
  
  printf("Erasing flash memory...\n");

  if (send(sock, message, 12, 0) < 0) {
    puts("Fault: send error\n\n");
    close(sock);
    return NULL;
  }

  bytes_to_read = file_len;
  bytes_read = 0;
  do {
    if ((rc = fread(buf+bytes_read, 1, bytes_to_read, fd)) > 0) {
      bytes_read += rc;
    } else {
      printf("read() error!\n");
      close(sock);
      return NULL;
    }
  }
  while (bytes_read < bytes_to_read);

  // wait for Erase done message
  memset(server_reply, 0, sizeof(server_reply));
  do {
    if ((recv_len = recv(sock , server_reply , 1000 , 0)) < 0) {
      puts("Fault: recv failed\n\n");
      close(sock);
      return NULL;
    }
    server_reply[recv_len]=0;
  }
  while (server_reply[recv_len-1] != 0);

  //  server_reply[recv_len]=0;
  printf(server_reply);

  printf("Uploading %d bytes of data...\n", file_len);

  if (send(sock, buf, file_len, 0) < 0) {
    puts("Fault: send error\n\n");
    close(sock);
    return NULL;
  }

  printf("Writing %d bytes to Flash memory...\n", file_len);

  // wait for Write DONE message
  memset(server_reply, 0, sizeof(server_reply));
  do {
    if ((recv_len = recv(sock , server_reply , 1000 , 0)) < 0) {
      puts("Fault: recv failed\n\n");
      close(sock);
      return NULL;
    }
    server_reply[recv_len]=0;
  }
  while (server_reply[recv_len-1] != 0);
  close(sock);

  printf(server_reply);

  return server_reply;
}

#ifdef EXE_MODE
int main(int argn, char**argv)
{
  char* ip_addr;
  char* filename;
  int bad_args_flag = 0;
  int mem_addr;
  if (argn != 1 && argn != 2 && argn != 3) {
    printf("Incorrect number of arguments (%d)!\n", argn);
    bad_args_flag = 1;
  }
  if (argn != 4) {
     printf("Incorrect number of arguments (%d)!\n", argn);
     bad_args_flag = 1;
   }

  if (sscanf(argv[2],"%x",&mem_addr) != 1) {
    printf("MEM_ADDR!\n");
    bad_args_flag = 1;
  }

  if (bad_args_flag) {
    printf("Syntax: col_write_cfi_flash IP_ADDR MEM_ADDR ECLDSP_FILE\n\n");
    return 1;
  }
  filename = argv[3];

  col_write_cfi_flash(ip_addr, mem_addr, filename);
  return 0;
}
#endif

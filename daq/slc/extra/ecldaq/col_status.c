#include <stdio.h>
#include<string.h>  
#include<sys/socket.h>  
#include<arpa/inet.h>

//#define ECL_TCP_STATUS_PORT (6001)

const char* col_status(const char* ip_addr)
{
  static char server_reply[10240];
  int sock;
  struct sockaddr_in server;
  char message[100];
  int recv_len = 0;
  int status_type = 0;

  //Create socket
  sock = socket(AF_INET , SOCK_STREAM , 0);
  if (sock == -1) {
    printf("Fault: cound'n create socket\n");
    return NULL;
  }

  server.sin_addr.s_addr = inet_addr(ip_addr);
  server.sin_family = AF_INET;
  int nEclTcpPort = 7000 + (((server.sin_addr.s_addr)>>24)&0xFF);
  server.sin_port = htons( nEclTcpPort );
  // server.sin_port = htons( ECL_TCP_STATUS_PORT );

  //Connect to remote server
  if (connect(sock , (struct sockaddr *)&server, sizeof(server)) < 0) {
    printf("Fault: connection error\n");
    return NULL;
  }

  message[0] = 'S';
  message[1] = (char)status_type;
  //Send some data
  if(send(sock, message, 2, 0) < 0) {
    puts("Fault: send error\n\n");
    return NULL;
  }
  memset(server_reply, 0, sizeof(server_reply));
  if ((recv_len = recv(sock , server_reply , 1000 , 0)) < 0) {
    puts("Fault: recv failed\n\n");
    return NULL;
  }
  return server_reply;
}

#ifdef EXE_MODE
int main(int argn, char** argv)
{
  char default_ip[]="192.168.1.12";
  char *ip_addr;
  int bad_args_flag = 0;
  if( argn!=1 && argn!=2 && argn!=3) {
    printf("Incorrect number of arguments (%d)!\n", argn);
    bad_args_flag = 1;
  }

  if(argn == 1) {
    ip_addr = default_ip;
  } else {
    ip_addr = argv[1];
  }
  if(argn == 3) {
    if(sscanf(argv[2],"%d",&status_type)!=1) {
      printf("Incorrect status_type (%d)!\n", argn);
      bad_args_flag = 1;
    }
  }
  if (bad_args_flag) {
    printf("Syntax: col_status [IP_ADDR [status_type]]\n");
    return NULL;
  }
  printf(col_status(ip_addr));
  return 0;
}
#endif

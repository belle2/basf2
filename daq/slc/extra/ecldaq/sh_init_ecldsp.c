#include <stdio.h>
#include <unistd.h>
#include <string.h>  
#include <sys/socket.h>  
#include <arpa/inet.h>

//#define ECL_TCP_STATUS_PORT (6001)

int sh_init_ecldsp(const char* ip_addr, int sh_num, unsigned int mem_addr)
{
  int nEclTcpPort = 6001;
  int sock;
  struct sockaddr_in server;
  char message [100], server_reply[1004];
  int recv_len=0;
  sh_num=!sh_num?16:sh_num;
  sh_num--;
  
  //Create socket
  sock = socket(AF_INET , SOCK_STREAM , 0);
  if (sock == -1) {
    printf("Fault: cound'n create socket\n");
    close(sock);
    return 1;
  }
  server.sin_addr.s_addr = inet_addr(ip_addr);
  server.sin_family = AF_INET;
  //server.sin_port = htons( ECL_TCP_STATUS_PORT );
  nEclTcpPort = 7000 + (((server.sin_addr.s_addr)>>24)&0xFF);
  server.sin_port = htons( nEclTcpPort );

  //Connect to remote server
  if (connect(sock , (struct sockaddr *)&server, sizeof(server)) < 0) {
    printf("Fault: connection error\n");
    close(sock);
    return 1;
  }

  message[0]='I';
  message[1]=sh_num;
  message[2]=200;
  message[3]=0;
  *(int*)(message+4) = htonl(mem_addr);
  printf("Initializing Shaper coefs...\n");
  if( send(sock, message, 8, 0) < 0) {
    printf("Fault: send error\n");
    close(sock);
    return 1;
  }

  // wait for Init DONE message
  do{
    if( (recv_len=recv(sock , server_reply , 1000 , 0)) < 0) {
      printf("Fault: recv failed\n\n");
      close(sock);
      return 2;
    }
    server_reply[recv_len] = 0;
  }
  while(  server_reply[recv_len-1] != 0 );
  printf(server_reply);
  close(sock);
  if (strcmp(server_reply, "DONE\n") != 0) {
    printf("Failed to init %d %s", sh_num, server_reply);
    return 3;
  }
  return 0;
}

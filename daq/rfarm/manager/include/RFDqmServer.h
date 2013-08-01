#ifndef RFDQMSERVER_H
#define RFDQMSERVER_H
//+
// File : RFDqmServer.h
// Description : DQM server for RFARM
//
// Author : Ryosuke Itoh, KEK
// Date : 14 - Jun - 2013
//-

class RFDqmServer {
public:
  RFDqmServer(char* mapfile);
  ~RFDqmServer();

  int init();
  int server();
};


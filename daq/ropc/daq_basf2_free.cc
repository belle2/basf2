#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <rawdata/dataobjects/PreRawCOPPERFormat_latest.h>
using namespace Belle2;

//namespace Belle2 {
int32_t main(int32_t argc, char** argv)
{

  PreRawCOPPERFormat_latest pre_rawcpr_fmt;
  int32_t* bufin = new int32_t[10000];
  int32_t nwords = 100;
  int32_t delete_flag = 1;
  int32_t num_events = 1;
  int32_t num_nodes = 1;

  pre_rawcpr_fmt.SetBuffer(bufin, nwords, delete_flag, num_events, num_nodes);

//    uint32_t m_node_id =
//      pre_rawcpr_fmt.FillTopBlockRawHeader();

  return 0;

}
//}

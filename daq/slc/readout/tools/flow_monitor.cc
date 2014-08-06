//+
// File : flow_monitor.cc
// Description : Readout flow rate monitor
//
// Author : Tomoyuki Konno, Tokyo Metroplitan University
// Date : 25 - Sep - 2013
//-

#include <unistd.h>
#include <cstdlib>

#include <daq/storage/BinData.h>

#include <daq/slc/nsm/NSMData.h>

#include <daq/slc/readout/RunInfoBuffer.h>
#include <daq/slc/readout/ronode_info.h>
#include <daq/slc/readout/ronode_status.h>
#include <daq/slc/readout/IOInfo.h>

#include <daq/slc/system/TCPSocket.h>
#include <daq/slc/system/TCPSocketReader.h>
#include <daq/slc/system/Time.h>
#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

#include <fstream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 2) {
    LogFile::debug("%s <node0>[ <node1> <node2>]", argv[0]);
    return 1;
  }
  std::vector<RunInfoBuffer> info_v;
  std::vector<IOInfo> ioinfo_v;
  std::vector<std::string> ip_v;
  std::vector<ronode_status> status_v;
  std::vector<unsigned long long> nbyte_v;
  for (int i = 0; i < argc - 1; i++) {
    RunInfoBuffer info;
    info.open(argv[i + 1], 1, false);
    info_v.push_back(info);
    ronode_status status;
    status.nodeid = info.getNodeId();
    status_v.push_back(status);
    nbyte_v.push_back(0);
    nbyte_v.push_back(0);
    IOInfo iinfo;
    iinfo.setLocalPort(info.getInputPort());
    iinfo.setLocalAddress(info.getInputAddress());
    std::string ip = iinfo.getLocalIP();
    LogFile::debug("in host:port=%s:%d", ip.c_str(),
                   iinfo.getLocalPort());
    ioinfo_v.push_back(iinfo);
    ip_v.push_back(ip);

    IOInfo oinfo;
    oinfo.setLocalPort(info.getOutputPort());
    oinfo.setLocalAddress(info.getOutputAddress());
    ip = oinfo.getLocalIP();
    LogFile::debug("out host:port=%s:%d", ip.c_str(),
                   oinfo.getLocalPort());
    ioinfo_v.push_back(oinfo);
    ip_v.push_back(ip);
  }
  std::ofstream fout("data.txt");
  while (true) {
    sleep(2);
    fputs("\033[2J\033[0;0H", stdout);
    rewind(stdout);
    ftruncate(1, 0);
    for (size_t i = 0; i < info_v.size(); i++) {
      ioinfo_v[2 * i].setLocalAddress(info_v[i].getInputAddress());
      ioinfo_v[2 * i].setLocalPort(info_v[i].getInputPort());
      ioinfo_v[2 * i + 1].setLocalAddress(info_v[i].getOutputAddress());
      ioinfo_v[2 * i + 1].setLocalPort(info_v[i].getOutputPort());
    }
    IOInfo::checkTCP(ioinfo_v);
    int ctime = Time().get();
    fout << ctime << " ";
    ronode_info info;
    printf("%d\n", ctime);
    for (size_t i = 0; i < info_v.size(); i++) {
      memcpy(&info, info_v[i].get(), sizeof(ronode_info));
      ronode_status& status(status_v[i]);
      float length = ctime - status.ctime;
      status.expno = info.expno;
      status.runno = info.runno;
      status.subno = info.subno;
      status.io[0].state = ioinfo_v[2 * i].getState();
      status.io[1].state = ioinfo_v[2 * i + 1].getState();
      status.ctime = ctime;
      unsigned int dcount;
      unsigned long long dnbyte;
      for (int j = 0; j < 2; j++) {
        status.io[j].freq = 0;
        status.io[j].evtsize = 0;
        status.io[j].rate = 0;
        if (j == 0) {
          status.io[j].nqueue = ioinfo_v[2 * i + j].getRXQueue();
        } else {
          status.io[j].nqueue = ioinfo_v[2 * i + j].getTXQueue();
        }
        if ((dcount = info.io[j].count - status.io[j].count) > 0) {
          dnbyte = info.io[j].nbyte - nbyte_v[2 * i + j];
          status.io[j].freq = dcount / length / 1000.;
          status.io[j].evtsize = dnbyte / dcount / 1000.;
          status.io[j].rate = dnbyte / length / 1000000.;
          status.io[j].count = info.io[j].count;
          nbyte_v[2 * i + j] = info.io[j].nbyte;
        } else {
          status.io[j].freq = 0;
          status.io[j].evtsize = 0;
          status.io[j].rate = 0;
        }
        printf("%15s:%5d %s | %7s | %6s | %7s | %12u | %6d | %2d\n",
               ip_v[i].c_str(), ioinfo_v[2 * i + j].getLocalPort(),
               ((j == 0) ? "(in) " : "(out)"),
               StringUtil::form("%4.2f", status.io[j].freq).c_str(),
               StringUtil::form("%3.2f", status.io[j].evtsize).c_str(),
               StringUtil::form("%4.2f", status.io[j].rate).c_str(),
               status.io[j].count,
               status.io[j].nqueue,
               status.io[j].state);
        fout << status.io[j].freq << " ";
        fout << status.io[j].evtsize << " ";
        fout << status.io[j].rate << " ";
        fout << status.io[j].nqueue << " ";
        fout << status.io[j].state << " ";
      }
    }
    fout << std::endl;
  }
  return 0;
}


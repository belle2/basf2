#include "daq/slc/readout/IOInfo.h"

#include <cstring>
#include <cstdlib>

#include <fstream>
#include <sstream>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

using namespace Belle2;

int IOInfo::checkTCP(std::vector<IOInfo>& info)
{
  size_t count = 0;
  for (size_t i = 0; i < info.size(); i++) {
    info[i].setState(0);
    info[i].setTXQueue(0);
    info[i].setRXQueue(0);
    if (info[i].getLocalPort() != 0) count++;
  }
  if (count == 0) return 0;
  count = 0;
  std::string line;
  std::stringstream ss;
  std::string sl, local_address, rem_address, st, queue;
  std::ifstream fin("/proc/net/tcp6");
  getline(fin, line);
  while (fin && getline(fin, line)) {
    ss.str("");
    ss << line;
    ss >> sl >> local_address >> rem_address >> st >> queue;
    unsigned int addr = strtoul(local_address.substr(24, 8).c_str(), NULL, 16);
    int port = strtoul(local_address.substr(24 + 9).c_str(), NULL, 16);
    for (size_t i = 0; i < info.size(); i++) {
      if (info[i].getLocalPort() <= 0 || info[i].getState() > 0) continue;
      if (addr == info[i].getLocalAddress() && port == info[i].getLocalPort()) {
        info[i].setRemoteAddress(strtoul(rem_address.substr(24, 8).c_str(), NULL, 16));
        info[i].setRemotePort(strtoul(rem_address.substr(24 + 9).c_str(), NULL, 16));
        info[i].setState(strtoul(st.substr(0).c_str(), NULL, 16));
        info[i].setTXQueue(strtoul(queue.substr(0, 8).c_str(), NULL, 16));
        info[i].setRXQueue(strtoul(queue.substr(9).c_str(), NULL, 16));
        count++;
      }
      if (count == info.size()) return count;
    }
  }
  fin.close();

  fin.open("/proc/net/tcp");
  getline(fin, line);
  while (fin && getline(fin, line)) {
    ss.str("");
    ss << line;
    ss >> sl >> local_address >> rem_address >> st >> queue;
    unsigned int addr = strtoul(local_address.substr(0, 8).c_str(), NULL, 16);
    int port = strtoul(local_address.substr(9).c_str(), NULL, 16);
    for (size_t i = 0; i < info.size(); i++) {
      if (info[i].getLocalPort() <= 0 || info[i].getState() > 0) continue;
      if (addr == info[i].getLocalAddress() && port == info[i].getLocalPort()) {
        info[i].setRemoteAddress(strtoul(rem_address.substr(0, 8).c_str(), NULL, 16));
        info[i].setRemotePort(strtoul(rem_address.substr(9).c_str(), NULL, 16));
        info[i].setState(strtoul(st.substr(0).c_str(), NULL, 16));
        info[i].setTXQueue(strtoul(queue.substr(0, 8).c_str(), NULL, 16));
        info[i].setRXQueue(strtoul(queue.substr(9).c_str(), NULL, 16));
        count++;
      }
      if (count == info.size()) return count;
    }
  }
  return count;
}

const char* IOInfo::getLocalIP() const
{
  sockaddr_in addr;
  addr.sin_addr.s_addr = m_local_addr;
  return inet_ntoa(addr.sin_addr);
}

const char* IOInfo::getRemoteIP() const
{
  sockaddr_in addr;
  addr.sin_addr.s_addr = m_remote_addr;
  return inet_ntoa(addr.sin_addr);
}

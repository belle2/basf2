/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/slc/readout/IOInfo.h"

#include <cstdlib>
#include <cstdio>

#include <fstream>
#include <sstream>
#include <arpa/inet.h>

using namespace Belle2;

int IOInfo::checkTCP(IOInfo& info)
{
  std::vector<IOInfo> info_v;
  info_v.push_back(info);
  int ret = checkTCP(info_v);
  info = info_v[0];
  return ret;
}

int IOInfo::checkTCP(std::vector<IOInfo>& info)
{
  size_t count = 0;
  for (size_t i = 0; i < info.size(); i++) {
    info[i].setState(0);
    info[i].setTXQueue(0);
    info[i].setRXQueue(0);
  }
  count = 0;
  std::string line;
  std::stringstream ss;
  std::string sl, local_address, rem_address, st, queue;
  std::ifstream fin("/proc/net/tcp");
  getline(fin, line);
  while (fin && getline(fin, line)) {
    ss.str("");
    ss << line;
    ss >> sl >> local_address >> rem_address >> st >> queue;
    unsigned int addr = strtoul(local_address.substr(0, 8).c_str(), NULL, 16);
    int port = strtoul(local_address.substr(9).c_str(), NULL, 16);
    unsigned int raddr = strtoul(rem_address.substr(0, 8).c_str(), NULL, 16);
    int rport = strtoul(rem_address.substr(9).c_str(), NULL, 16);
    for (size_t i = 0; i < info.size(); i++) {
      if (info[i].getState() > 0) continue;
      int sti = strtoul(st.substr(0).c_str(), NULL, 16);
      if (//addr == info[i].getLocalAddress() &&
        port == info[i].getLocalPort() && sti == 1) {
        info[i].setRemoteAddress(raddr);
        info[i].setRemotePort(rport);
        info[i].setState(sti);
        info[i].setTXQueue(strtoul(queue.substr(0, 8).c_str(), NULL, 16));
        info[i].setRXQueue(strtoul(queue.substr(9).c_str(), NULL, 16));
        count++;
      } else if (raddr == info[i].getRemoteAddress() &&
                 rport == info[i].getRemotePort() && sti == 1) {
        info[i].setLocalAddress(addr);
        info[i].setLocalPort(port);
        info[i].setState(sti);
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

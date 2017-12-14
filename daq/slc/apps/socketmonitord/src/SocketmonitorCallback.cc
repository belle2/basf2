#include "daq/slc/apps/socketmonitord/SocketmonitorCallback.h"

#include <daq/slc/system/LogFile.h>

#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>

using namespace Belle2;

SocketmonitorCallback::SocketmonitorCallback(const std::string& name, int timeout)
{
  setNode(NSMNode(name));
  setTimeout(timeout);
}

void SocketmonitorCallback::addSocket(const std::string& hostname,
                                      int port, bool islocal) throw(IOException)
{
  sockaddr_in addr;
  memset(&addr, 0, sizeof(sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(port);
  int fd;
  if ((fd = ::socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    throw (IOException("Failed to create socket"));
  }
  struct hostent* host = NULL;
  host = gethostbyname(hostname.c_str());
  if (host == NULL) {
    unsigned long ip_address = inet_addr(hostname.c_str());
    if ((signed long) ip_address < 0) {
      throw (IOException("Wrong host name or ip : %s", hostname.c_str()));
    } else {
      host = gethostbyaddr((char*)&ip_address, sizeof(ip_address), AF_INET);
    }
  }
  if (host == NULL) {
    ::close(fd);
    throw (IOException("No host %s", hostname.c_str()));
  }
  addr.sin_addr.s_addr = (*(unsigned long*) host->h_addr_list[0]);
  IOInfo info;
  if (islocal) {
    info.setLocalPort(port);
    info.setLocalAddress(addr.sin_addr.s_addr);
  } else {
    info.setRemotePort(port);
    info.setRemoteAddress(addr.sin_addr.s_addr);
  }
  m_islocal.push_back(islocal);
  m_hostname.push_back(hostname);
  m_info.push_back(info);
  ::close(fd);
}

SocketmonitorCallback::~SocketmonitorCallback() throw()
{
}

void SocketmonitorCallback::init(NSMCommunicator&) throw()
{
  add(new NSMVHandlerInt("nsockets", true, false, m_hostname.size()));
  for (size_t i = 0; i < m_hostname.size(); i++) {
    std::string vname = StringUtil::form("socket[%d].", i);
    add(new NSMVHandlerText(vname + "hostname", true, false, m_hostname[i]));
    if (m_islocal[i]) {
      add(new NSMVHandlerText(vname + "ip", true, false, m_info[i].getLocalIP()));
      add(new NSMVHandlerInt(vname + "port", true, false, m_info[i].getLocalPort()));
    } else {
      add(new NSMVHandlerText(vname + "ip", true, false, m_info[i].getRemoteIP()));
      add(new NSMVHandlerInt(vname + "port", true, false, m_info[i].getRemotePort()));
    }
    add(new NSMVHandlerInt(vname + "st", true, false, 0));
    add(new NSMVHandlerFloat(vname + "rxqueue", true, false, 0));
    add(new NSMVHandlerFloat(vname + "txqueue", true, false, 0));
  }
}

void SocketmonitorCallback::timeout(NSMCommunicator&) throw()
{
  IOInfo::checkTCP(m_info);
  static unsigned long long count = 0;
  try {
    for (size_t i = 0; i < m_hostname.size(); i++) {
      std::string vname = StringUtil::form("socket[%d].", i);
      int st = 0;
      float rxqueue = 0, txqueue = 0;
      st = m_info[i].getState();
      txqueue = m_info[i].getTXQueue();
      rxqueue = m_info[i].getRXQueue();
      txqueue /= 1024;
      rxqueue /= 1024;
      set(vname + "st", st);
      set(vname + "txqueue", txqueue);
      set(vname + "rxqueue", rxqueue);
      if (count % 2 == 0) {
        //  LogFile::debug("%s st=%d tx=%f rx=%f", m_hostname[i].c_str(), st, txqueue, rxqueue);
      }
    }
    count++;
  } catch (const IOException& e) {

  }
  count++;
}


#include "daq/slc/apps/sockmemd/SockmemCallback.h"

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/Time.h>

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <string>
#include <sstream>
#include <iostream>

using namespace Belle2;

void SockmemCallback::tx_t::update(SockmemCallback& callback, std::string state,
                                   double nrxq, double ntxq, double bytes, std::string local,
                                   std::string remote, double t, double t0)
{
  //std::cout << "update >> '"<< state << "' '" << nrxq << "' '" << ntxq << "' '" << local << "' '" << remote << "'"<< std::endl;
  m_active = (state == "ESTAB");
  if (t0 > 0) {
    double dt = t - t0;
    float rate = (bytes - m_bytes) / dt / 1024 / 1024; //MB
    m_rate = rate;
  }
  m_bytes = bytes;
  m_nrxq = nrxq;
  m_ntxq = ntxq;
}

StringList SockmemCallback::popen(const std::string& cmd)
{
  char buf[10000];
  FILE* fp;
  StringList ss;
  if ((fp = ::popen(cmd.c_str(), "r")) == NULL) {
    LogFile::error("can not exec commad");
    return ss;
    //exit(EXIT_FAILURE);
  }
  while (!feof(fp)) {
    memset(buf, 0, 1000);
    fgets(buf, sizeof(buf), fp);
    ss.push_back(buf);
  }
  pclose(fp);
  return ss;
}

SockmemCallback::SockmemCallback(const std::string& name, int timeout, ConfigFile& conf)
{
  LogFile::debug("NSM nodename = %s (timeout: %d seconds)", name.c_str(), timeout);
  setNode(NSMNode(name));
  setTimeout(timeout);
  for (int i = 0; true ; i++) {
    std::string vname = StringUtil::form("tx[%d].", i);
    std::string local = conf.get(vname + "local");
    std::string remote = conf.get(vname + "remote");
    std::string host;// = conf.get(vname + "host");
    if (local.size() > 0) {
      host = StringUtil::split(local, ':')[0];
    } else if (remote.size() > 0) {
      host = StringUtil::split(remote, ':')[0];
    } else {
      break;
    }
    tx_t tx = {host, local, remote, 0, 0, 0, 0, false};
    m_tx.insert(std::map<std::string, tx_t>::value_type((local.size() > 0 ? local : remote), tx));
  }
}

SockmemCallback::~SockmemCallback() throw()
{
}

void SockmemCallback::init(NSMCommunicator&) throw()
{
  for (std::map<std::string, tx_t>::iterator it = m_tx.begin(); it != m_tx.end(); it++) {
    tx_t& tx(it->second);
    add(new NSMVHandlerInt(tx.m_host + ".active", true, false, 0));
    add(new NSMVHandlerFloat(tx.m_host + ".ntxq", true, false, 0));
    add(new NSMVHandlerFloat(tx.m_host + ".nrxq", true, false, 0));
    add(new NSMVHandlerFloat(tx.m_host + ".bytes", true, false, 0));
    add(new NSMVHandlerFloat(tx.m_host + ".rate", true, false, 0));
  }
}

void SockmemCallback::timeout(NSMCommunicator&) throw()
{
  static double t0 = 0;
  std::string state;
  double nrxq;
  double ntxq;
  std::string local;
  std::string remote;
  for (std::map<std::string, tx_t>::iterator it = m_tx.begin(); it != m_tx.end(); it++) {
    tx_t& tx(it->second);
    tx.m_active = 0;
    tx.m_rate = 0;
  }
  double t = Time().get();
  StringList lines = popen("/usr/sbin/ss -ntr4i");
  if (lines.size() == 0) return;
  for (size_t i = 1; i < lines.size(); i++) {
    {
      std::stringstream ss;
      ss << lines[i];
      if (!(ss >> state >> nrxq >> ntxq >> local >> remote)) break;
    }
    double bytes = 0;
    std::string s;
    i++;
    if (i >= lines.size()) break;
    {
      std::stringstream ss;
      ss << lines[i];
      while (ss >> s) {
        if (s.find("bytes_received:") != std::string::npos) {
          bytes = atof(StringUtil::replace(s, "bytes_received:", "").c_str());
          break;
        }
      }
    }
    if (m_tx.find(local) != m_tx.end()) {
      tx_t& tx(m_tx[local]);
      tx.update(*this, state, nrxq, ntxq, bytes, local, remote, t, t0);
    } else if (m_tx.find(remote) != m_tx.end()) {
      tx_t& tx(m_tx[remote]);
      tx.update(*this, state, nrxq, ntxq, bytes, local, remote, t, t0);
    }
  }
  t0 = t;
  for (std::map<std::string, tx_t>::iterator it = m_tx.begin(); it != m_tx.end(); it++) {
    tx_t& tx(it->second);
    set(tx.m_host + ".active", (int)tx.m_active);
    set(tx.m_host + ".ntxq", (float)tx.m_ntxq);
    set(tx.m_host + ".nrxq", (float)tx.m_nrxq);
    set(tx.m_host + ".bytes", (float)tx.m_bytes);
    set(tx.m_host + ".rate", (float)tx.m_rate);
  }
}


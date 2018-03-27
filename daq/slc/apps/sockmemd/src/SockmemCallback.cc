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
  m_active = (state == "ESTAB");
  if (t0 > 0) {
    double dt = t - t0;
    float rate = (bytes - m_bytes) / dt / 1024 / 1024; //MB
    m_rate = rate;
  }
  m_bytes = bytes;
}

StringList SockmemCallback::popen(const std::string& cmd)
{
  char buf[10000];
  FILE* fp;
  if ((fp = ::popen(cmd.c_str(), "r")) == NULL) {
    perror("can not exec commad");
    exit(EXIT_FAILURE);
  }
  StringList ss;
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
  //m_rcnode = NSMNode(conf.get("rcnode"));
  for (int i = 0; true ; i++) {
    std::string vname = StringUtil::form("tx[%d].", i);
    std::string host = conf.get(vname + "host");
    std::string local = conf.get(vname + "local");
    std::string remote = conf.get(vname + "remote");
    std::string ip = (local.size() > 0 ? local : remote);
    if (ip.size() == 0) break;
    if (host.size() == 0) host = StringUtil::split(ip, ':')[0];
    LogFile::debug("host=" + host);
    tx_t tx = {host, conf.get(vname + ".type"), local, remote, 0, 0, 0, 0, false};
    m_tx.insert(std::map<std::string, tx_t>::value_type(ip, tx));
  }
}

SockmemCallback::~SockmemCallback() throw()
{
}

/*
   initialize() : Initialize with parameters from configuration database.
                  Called at the beginings of NSM node.
   obj          : Contains parameters in configuration from the database.
*/
void SockmemCallback::initialize(const DBObject& obj) throw(RCHandlerException)
{
  add(new NSMVHandlerText("state", true, false, "OFF"));
  for (std::map<std::string, tx_t>::iterator it = m_tx.begin(); it != m_tx.end(); it++) {
    tx_t& tx(it->second);
    add(new NSMVHandlerInt(tx.m_host + ".active", true, false, 0));
    add(new NSMVHandlerFloat(tx.m_host + ".ntxq", true, false, 0));
    add(new NSMVHandlerFloat(tx.m_host + ".nrxq", true, false, 0));
    add(new NSMVHandlerFloat(tx.m_host + ".bytes", true, false, 0));
    add(new NSMVHandlerFloat(tx.m_host + ".rate", true, false, 0));
  }
}

/*
   configure() : Read parameters from configuration database.
                 Called by RC_CONFIGURE at NOTREADY.
   obj         : Contains parameters in configuration from the database.
*/
void SockmemCallback::configure(const DBObject& obj) throw(RCHandlerException)
{
}

/*
   monitor() : Check status at each end of wainting time limit.
*/
void SockmemCallback::monitor() throw(RCHandlerException)
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
  StringList lines = popen("ss -ntr4i");
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
  bool notready = true;
  bool running = false;
  for (std::map<std::string, tx_t>::iterator it = m_tx.begin(); it != m_tx.end(); it++) {
    tx_t& tx(it->second);
    int used = 1;
    /*
    try {
      get(m_rcnode, tx.m_host+".used", used);
    } catch (const IOException& e) {
      //  break;
    }
    */
    if (tx.m_active) {
      if (tx.m_rate > 0) running = true;
      set(tx.m_host + ".active", (int)tx.m_active);
      set(tx.m_host + ".ntxq", (float)tx.m_ntxq);
      set(tx.m_host + ".nrxq", (float)tx.m_nrxq);
      set(tx.m_host + ".bytes", (float)tx.m_bytes);
      set(tx.m_host + ".rate", (float)tx.m_rate);
    } else {
      if (used) notready = true;
      set(tx.m_host + ".active", (int)0);
      set(tx.m_host + ".ntxq", (float)0);
      set(tx.m_host + ".nrxq", (float)0);
      set(tx.m_host + ".bytes", (float)0);
      set(tx.m_host + ".rate", (float)0);
    }
  }
  if (notready) set("state", "NOTREADY");
  else if (running) set("state", "RUNNING");
  else set("state", "READY");
}

/*
   load() : Perform parameter download etc to READY.
   obj    : Contains configuration read by the last initailize or configure.
            Values in obj might be modified by nsmvset.
*/
void SockmemCallback::load(const DBObject& obj) throw(RCHandlerException)
{
  m_aborted = false;
  while (!m_aborted) {
    monitor();
    bool suceeded = true;
    for (std::map<std::string, tx_t>::iterator it = m_tx.begin(); it != m_tx.end(); it++) {
      if (m_aborted) break;
      tx_t& tx(it->second);
      int used = 0;
      try {
        get(m_rcnode, tx.m_host + ".used", used);
        if (used && !tx.m_active) {
          suceeded = false;
          break;
        }
      } catch (const IOException& e) {
        break;
      }
    }
    if (suceeded) break;
    try {
      wait(1);
    } catch (const TimeoutException& e) {

    }
  }
  m_aborted = false;
  LogFile::debug("Load done");
}

/*
   start() : Start triggers to RUNNING
   expno   : Experiment number from RC parent
   runno   : Run number from RC parent
*/
void SockmemCallback::start(int expno, int runno) throw(RCHandlerException)
{
  LogFile::debug("run # = %04d.%04d.%03d", expno, runno, 0);
  monitor();
  for (std::map<std::string, tx_t>::iterator it = m_tx.begin(); it != m_tx.end(); it++) {
    tx_t& tx(it->second);
    int used = 0;
    try {
      get(m_rcnode, tx.m_host + ".used", used);
      if (used && !tx.m_active) {
        throw (RCHandlerException("link lost on " + tx.m_host));
        break;
      }
    } catch (const IOException& e) {
      break;
    }
  }
  LogFile::debug("Start done");
}

/*
   stop() : End run by stopping triggers to READY
*/
void SockmemCallback::stop() throw(RCHandlerException)
{
  LogFile::debug("Stop done");
}

/*
   resume() : Restart triggers back to RUNNING
   subno    : Sub run number for the resumed run
   returns true on success or false on failed
*/
bool SockmemCallback::resume(int subno) throw(RCHandlerException)
{
  LogFile::debug("sub run # = %03d", subno);
  LogFile::debug("Resume done");
  return true;
}

/*
   pause() : Suspend triggers to PAUSED
   returns true on success or false on failed
*/
bool SockmemCallback::pause() throw(RCHandlerException)
{
  LogFile::debug("Pause done");
  return true;
}

/*
   recover() : Recover errors to be back to READY
*/
void SockmemCallback::recover(const DBObject& obj) throw(RCHandlerException)
{
  // abort to be NOTREADY
  abort();
  // load to be READY
  load(obj);
  LogFile::debug("Recover done");
}

/*
   abort() : Discard all configuration to back to NOTREADY
*/
void SockmemCallback::abort() throw(RCHandlerException)
{
  m_aborted = true;
  monitor();
  LogFile::debug("Abort done");
}


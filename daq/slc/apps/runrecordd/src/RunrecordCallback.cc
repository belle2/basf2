#include "daq/slc/apps/runrecordd/RunrecordCallback.h"

#include <daq/slc/base/StringUtil.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/database/DBObjectLoader.h>

#include <daq/slc/psql/PostgreSQLInterface.h>

#include <iostream>
#include <sstream>

#include <stdlib.h>

using namespace Belle2;

RunrecordCallback::RunrecordCallback(const std::string& name, ConfigFile& config, int timeout)
{
  LogFile::debug("NSM nodename = %s (timeout: %d seconds)", name.c_str(), timeout);
  setNode(NSMNode(name));
  setTimeout(timeout);
  m_rcnode = NSMNode(config.get("rcnode"));
  m_objs = (DBObjectLoader::load(config.get("file"))).getObjects("vars");
  m_db = new PostgreSQLInterface(config.get("database.host"),
                                 config.get("database.dbname"),
                                 config.get("database.user"),
                                 config.get("database.password"),
                                 config.getInt("database.port"));
}

RunrecordCallback::~RunrecordCallback() throw()
{
}

void RunrecordCallback::init(NSMCommunicator& com) throw()
{
  std::string rcstate;
  int runno = 0, expno = 0;
  try {
    get(m_rcnode, "rcstate", rcstate, 10);
    LogFile::info("%s>>%s", m_rcnode.getName().c_str(), rcstate.c_str());
    get(m_rcnode, "expno", expno, 10);
    get(m_rcnode, "runno", runno, 10);
  } catch (const TimeoutException& e) {
  }
  std::stringstream ss;
  ss << "config : " << getNode().getName()
     << StringUtil::form(":%04d:%05d:", expno, runno)
     << (rcstate == "STARTING" ? "s" : "e") << std::endl;
  ss << "DAQ.Date.S :" << Date().toString() << std::endl;
  ss << "DAQ.ExpNumber.I :" << expno << std::endl;
  ss << "DAQ.RunNumber.I :" << runno << std::endl;
  for (size_t i = 0; i < m_objs.size(); i++) {
    const DBObject obj(m_objs[i]);
    obj.print();
    NSMVar var(obj.getText("var"));
    try {
      get(NSMNode(obj.getText("node")), var, 10);
      std::string name = obj.getText("name");
      ss << name << " : ";
      switch (var.getType()) {
        case NSMVar::INT:
          ss << "int(" << var.getInt() << ")" <<  std::endl;
          break;
        case NSMVar::FLOAT:
          ss << "float(" << var.getFloat() << ")" <<  std::endl;
          break;
        case NSMVar::TEXT:
          ss << "\"" << StringUtil::replace(var.getText(), "\n", "\\n") << "\"" << std::endl;
          break;
        case NSMVar::NONE:
          ss << "null" <<  std::endl;
          break;
      }
    } catch (const TimeoutException& e) {

    }
    m_vars.push_back(var);
  }
  std::cout << ss.str() << std::endl;
  m_rcstate = rcstate;
  //update(rcstate);
}

void RunrecordCallback::timeout(NSMCommunicator&) throw()
{
  update(m_rcstate);
}

void RunrecordCallback::vset(NSMCommunicator& com, const NSMVar& v) throw()
{
  if (v.getNode() == m_rcnode.getName() && v.getName() == "rcstate") {
    std::string rcstate = v.getText();
    if ((m_rcstate == "READY" && rcstate == "STARTING") ||
        ((m_rcstate == "RUNNING" || m_rcstate == "ERROR") &&
         (rcstate == "ABORTING" || rcstate == "STOPPING"))) {
      update(rcstate);
    }
    m_rcstate = rcstate;
  } else {
    for (size_t i = 0; i < m_vars.size(); i++) {
      NSMVar& var(m_vars[i]);
      if (var.getNode() == v.getNode() && var.getName() == v.getName()) {
        var = v;
        return;
      }
    }
    NSMCallback::vset(com, v);
  }
}

void RunrecordCallback::update(const std::string& rcstate)
{
  int runno = 0, expno = 0;
  try {
    get(m_rcnode, "expno", expno, 10);
  } catch (const TimeoutException& e) {
  }
  try {
    get(m_rcnode, "runno", runno, 10);
  } catch (const TimeoutException& e) {
  }
  std::stringstream ss;
  ss << "config : " << getNode().getName()
     << StringUtil::form(":%04d:%05d:", expno, runno)
     << (rcstate == "STARTING" ? "s" : "e") << std::endl;
  ss << "DAQ.Date.S :" << Date().toString() << std::endl;
  ss << "DAQ.ExpNumber.I :" << expno << std::endl;
  ss << "DAQ.RunNumber.I :" << runno << std::endl;
  for (size_t i = 0; i < m_objs.size(); i++) {
    const DBObject obj(m_objs[i]);
    NSMVar& var(m_vars[i]);
    // get(NSMNode(obj.getText("node")), var);
    ss << obj.getText("name") << " : ";
    switch (var.getType()) {
      case NSMVar::INT:
        ss << "int(" << var.getInt() << ")" <<  std::endl;
        break;
      case NSMVar::FLOAT:
        ss << "float(" << var.getFloat() << ")" <<  std::endl;
        break;
      case NSMVar::TEXT:
        ss << "\"" << StringUtil::replace(StringUtil::replace(var.getText(), "\n", "\\n") , "%", "\%") << "\"" << std::endl;
        break;
      case NSMVar::NONE:
        ss << "null" <<  std::endl;
        break;
    }
  }
  std::string s = ss.str();
  //LogFile::info(s);
  std::cout << s << std::endl;
  ConfigFile conf(ss);
  DBObject obj = DBObjectLoader::load(conf);
  try {
    DBObjectLoader::createDB(*m_db, "runrecord", obj);
  } catch (const DBHandlerException& e) {
  }
}

#include "daq/slc/runcontrol/RCNodeDaemon.h"

#include <daq/slc/database/PostgreSQLInterface.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/Daemon.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

RCNodeDaemon::RCNodeDaemon(ConfigFile& config,
                           RCCallback* callback,
                           RCCallback* callback2)
{
  std::string host = config.get("nsm.host");
  if (host.size() == 0) {
    LogFile::error("nsm.host is empty");
    exit(1);
  }
  int port = config.getInt("nsm.port");
  if (port < 0) {
    LogFile::error("nsm.port is not a positive integer");
    exit(1);
  }
  std::string name = config.get("nsm.nodename");
  if (name.size() == 0) {
    LogFile::error("nsm.nodename is empty");
    exit(1);
  }
  callback->setNode(NSMNode(name));
  std::string runtype = config.get("runtype");
  if (runtype.size() > 0) {
    callback->setRuntype(runtype);
  } else {
    LogFile::warning("runtype is empty");
  }
  std::string dbtable = config.get("dbtable");
  if (dbtable.size() > 0) {
    LogFile::debug("database.use=%s", config.getBool("database.use") ? "TRUE" : "FALSE");
    if (config.getBool("database.use")) {
      callback->setDB(new PostgreSQLInterface(config.get("database.host"),
                                              config.get("database.dbname"),
                                              config.get("database.user"),
                                              config.get("database.password"),
                                              config.getInt("database.port")),
                      dbtable);
    } else {
      callback->setDBTable(dbtable);
      callback->setProvider(config.get("provider.host"),
                            config.getInt("provider.port"));
    }
  } else {
    LogFile::warning("dbtable is empty");
  }
  m_daemon.add(callback, host, port);
  host = config.get("nsm.global.host");
  port = config.getInt("nsm.global.port");
  if (callback2 != NULL && host.size() > 0 && port > 0) {
    callback2->setNode(NSMNode(name));
    m_daemon.add(callback2, host, port);
  }
}

void RCNodeDaemon::run()
{
  m_daemon.run();
}

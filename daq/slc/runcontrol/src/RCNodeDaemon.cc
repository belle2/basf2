#include "daq/slc/runcontrol/RCNodeDaemon.h"

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/Daemon.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

RCNodeDaemon::RCNodeDaemon(ConfigFile& config,
                           RCCallback* callback,
                           RCCallback* callback2,
                           DBInterface* db)
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
  std::string rcconfig = config.get("rcconfig");
  if (rcconfig.size() > 0) {
    callback->setRCConfig(rcconfig);
  } else {
    LogFile::notice("rcconfig is empty");
  }
  int timeout = config.getInt("timeout");
  if (timeout > 0) {
    callback->setTimeout(timeout);
  }
  std::string file = config.get("file");
  std::string dbtable = config.get("dbtable");
  if (file.size() > 0) {
    LogFile::info("read file %s", file.c_str());
    callback->setDBFile(file);
  } else if (dbtable.size() > 0) {
    LogFile::debug("database.use=%s", config.getBool("database.use") ? "TRUE" : "FALSE");
    if (config.getBool("database.use") && db != NULL) {
      callback->setDB(db, dbtable);
    } else {
      callback->setDBTable(dbtable);
      callback->setProvider(config.get("provider.host"),
                            config.getInt("provider.port"));
    }
  } else if (file.size() > 0) {

  } else {
    LogFile::notice("dbtable is empty");
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

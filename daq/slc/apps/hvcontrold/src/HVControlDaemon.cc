#include "daq/slc/apps/hvcontrold/HVControlDaemon.h"
#include "daq/slc/apps/hvcontrold/HVControlCallback.h"

#include <daq/slc/database/PostgreSQLInterface.h>

#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

void HVControlDaemon::run()
{
  ConfigFile config("slowcontrol", "hv");
  DBInterface* db = new PostgreSQLInterface(config.get("database.host"),
                                            config.get("database.dbname"),
                                            config.get("database.user"),
                                            config.get("database.password"),
                                            config.getInt("database.port"));
  m_callback->setDB(db);
  const std::string nodename = m_callback->getNode()->getName();
  const int configid = config.getInt(nodename + ".config");
  HVNodeInfo& info(m_callback->getInfo());
  info.setConfigId(configid);
  info.readFile(config.get(nodename + ".include"));
  LogFile::open("hvcontrold." + nodename);
  NSMNodeDaemon* daemon = new NSMNodeDaemon(m_callback,
                                            config.get("nsm.local.host"),
                                            config.getInt("nsm.local.port"));
  daemon->run();
}

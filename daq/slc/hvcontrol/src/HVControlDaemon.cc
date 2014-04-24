#include "daq/slc/hvcontrol/HVControlDaemon.h"
#include "daq/slc/hvcontrol/HVControlCallback.h"

#include <daq/slc/database/PostgreSQLInterface.h>

#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

void HVControlDaemon::run()
{
  ConfigFile config("slowcontrol");
  DBInterface* db = new PostgreSQLInterface(config.get("database.host"),
                                            config.get("database.dbname"),
                                            config.get("database.user"),
                                            config.get("database.password"),
                                            config.getInt("database.port"));
  m_callback->setDB(db);
  const std::string nodename = m_callback->getNode().getName();
  LogFile::open("hvcontrold." + nodename);
  /*  */
  NSMNodeDaemon* daemon = new NSMNodeDaemon(m_callback,
                                            config.get("nsm.local.host"),
                                            config.getInt("nsm.local.port"));
  daemon->run();
}

#include "daq/slc/hvcontrol/HVControlDaemon.h"
#include "daq/slc/hvcontrol/HVControlCallback.h"

#include <daq/slc/psql/PostgreSQLInterface.h>

#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

void HVControlDaemon::run()
{
  ConfigFile config("slowcontrol", "hvcontrol/" + m_filename);
  PostgreSQLInterface db(config.get("database.host"),
                         config.get("database.dbname"),
                         config.get("database.user"),
                         config.get("database.password"),
                         config.getInt("database.port"));
  const std::string nodename = config.get("nsm.nodename");
  LogFile::open("hvcontrold/" + nodename, LogFile::DEBUG);
  m_callback->getNode().setName(nodename);
  m_callback->setDB(config.get("hv.tablename"), &db);
  m_callback->setTimeout(config.getInt("hv.interval"));
  LogFile::info("config.standby : " + config.get("hv.config.standby"));
  m_callback->setStandbyConfig(config.get("hv.config.standby"));
  m_callback->setShoulderConfig(config.get("hv.config.shoulder"));
  m_callback->setPeakConfig(config.get("hv.config.peak"));
  NSMNodeDaemon(m_callback, config.get("nsm.host"),
                config.getInt("nsm.port")).run();
}

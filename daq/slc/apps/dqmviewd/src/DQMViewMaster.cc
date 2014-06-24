#include "daq/slc/apps/dqmviewd/DQMViewMaster.h"

#include <daq/slc/system/Fork.h>
#include <daq/slc/system/Executor.h>
#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

void DQMViewMaster::init()
{
  for (size_t i = 0; i < m_reader_v.size(); i++) {
    //m_con_v.push_back(ProcessController());
  }
  for (size_t i = 0; i < m_reader_v.size(); i++) {
    const std::string name = m_reader_v[i].getName();
    //m_con_v[i].setCallback(m_callback);
    //m_con_v[i].init("hserver_" + name);
  }
}

bool DQMViewMaster::boot()
{
  /*
  ConfigFile config("dqm");
  const std::string dumppath = config.get("DQM_DUMP_PATH");
  const std::string mappath = config.get("DQM_MAP_PATH");
  LogFile::debug("DQM_DUMP_PATH=%s", dumppath.c_str());
  LogFile::debug("DQM_MAP_PATH=%s", mappath.c_str());
  for (size_t i = 0; i < m_reader_v.size(); i++) {
    const std::string filename = m_reader_v[i].getFileName();
    LogFile::debug("booting hserver %d %s", m_port_v[i], filename.c_str());
    m_con_v[i].clearArguments();
    m_con_v[i].setExecutable("hserver");
    m_con_v[i].addArgument(StringUtil::form("%d", m_port_v[i]));
    m_con_v[i].addArgument(filename);
    m_con_v[i].load(-1);
  }
  */
  return true;
}

bool DQMViewMaster::abort()
{
  for (size_t i = 0; i < m_reader_v.size(); i++) {
    //m_con_v[i].abort();
  }
  return true;
}


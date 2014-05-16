#include "daq/slc/apps/nsm2socket/DB2NSM.h"

#include "daq/slc/database/ConfigObjectTable.h"
#include "daq/slc/database/FieldInfoTable.h"
#include "daq/slc/database/ConfigInfoTable.h"

#include <daq/slc/nsm/NSMCommand.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

bool DB2NSM::set(const NSMMessage& msg_in, NSMMessage& msg_out)
{
  try {
    ConfigObject obj;
    StringList str_v = StringUtil::split(msg_in.getData(), ' ', 5);
    m_db->connect();
    if (str_v[0] == "configlist") {
      ConfigInfoList info_v = ConfigInfoTable(m_db).getList(str_v[1]);
      for (size_t i = 0; i < info_v.size(); i++) {
        ConfigInfo& info(info_v[i]);
        ConfigObject objc;
        objc.addText("name", info.getName());
        objc.addText("node", info.getNode());
        objc.addText("table", info.getTable());
        obj.addObject("config", objc);
      }
    } else if (str_v[0] == "config") {
      std::string nodename = str_v[1];
      std::string configname = str_v[2];
      if (msg_in.getNParams() > 0 && msg_in.getParam(0) > 0) {
        configname = ConfigInfoTable(m_db).get(msg_in.getParam(0)).getName();
      }
      obj = ConfigObjectTable(m_db).get(configname, nodename);
    }
    m_db->close();
    msg_out.setRequestName(NSMCommand::DBSET);
    msg_out.setData(obj);
    return true;
  } catch (const DBHandlerException& e) {
    LogFile::error(e.what());
    m_db->close();
  }
  return false;
}

bool DB2NSM::get(const NSMMessage& msg_in)
{
  try {
    ConfigObject obj;
    if (msg_in.getLength() > 0) {
      msg_in.getData(obj);
      m_db->connect();
      ConfigObjectTable(m_db).add(obj);
      m_db->close();
      return true;
    }
  } catch (const DBHandlerException& e) {
    m_db->close();
  }
  return false;

}

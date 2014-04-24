#include "daq/slc/apps/nsm2socket/DB2NSM.h"

#include "daq/slc/database/ConfigObjectTable.h"
#include "daq/slc/database/FieldInfoTable.h"
#include "daq/slc/database/ConfigInfoTable.h"

#include <daq/slc/nsm/NSMCommand.h>

#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

bool DB2NSM::set(const NSMMessage& msg_in, NSMMessage& msg_out)
{
  try {
    ConfigObject obj;
    StringList str_v = StringUtil::split(msg_in.getData(), ' ', 5);
    if (str_v[0] == "field") {
      m_db->connect();
      FieldInfoList info_v = FieldInfoTable(m_db).getList();
      m_db->close();
      for (size_t i = 0; i < info_v.size(); i++) {
        FieldInfo& info(info_v[i]);
        ConfigObject objc;
        objc.addText("name", info.getName());
        objc.addInt("type", info.getType());
        objc.addText("table", info.getTable());
        obj.addObject("fieldinfo", objc);
      }
    } else if (str_v[0] == "config") {
      m_db->connect();
      ConfigInfoList info_v =
        ConfigInfoTable(m_db).getList(str_v[1]);
      m_db->close();
      for (size_t i = 0; i < info_v.size(); i++) {
        ConfigInfo& info(info_v[i]);
        ConfigObject objc;
        objc.addText("name", info.getName());
        objc.addText("node", info.getNode());
        objc.addText("table", info.getTable());
        obj.addObject("config", objc);
      }
    } else if (str_v[0] == "object") {
      std::string configname = str_v[2];
      std::string nodename = str_v[1];
      m_db->connect();
      obj = ConfigObjectTable(m_db).get(configname, nodename);
      m_db->close();
    }
    msg_out.init();
    msg_out.setRequestName(NSMCommand::DBSET);
    msg_out.setData(obj);
    return true;
  } catch (const DBHandlerException& e) {
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

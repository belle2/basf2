#include <daq/slc/database/ConfigObjectTable.h>
#include <daq/slc/database/ConfigInfoTable.h>
#include <daq/slc/database/FieldInfoTable.h>
#include <daq/slc/database/NodeInfoTable.h>
#include <daq/slc/database/TableInfoTable.h>
#include <daq/slc/database/PostgreSQLInterface.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

#include <iostream>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 4) {
    LogFile::debug("usage: dbtest5 <filepath> <tablename> <groupname> [<configname>]");
    return 1;
  }
  const std::string filename = argv[1];
  const std::string tablename = argv[2];
  const std::string groupname = argv[3];
  const std::string configname = (argc > 4) ? argv[4] : "default";

  ConfigFile dbconfig("slowcontrol");
  DBInterface* db = new PostgreSQLInterface(dbconfig.get("database.host"),
                                            dbconfig.get("database.dbname"),
                                            dbconfig.get("database.user"),
                                            dbconfig.get("database.password"),
                                            dbconfig.getInt("database.port"));
  ConfigFile config(filename);
  const std::string nodename = config.get("nodename");
  int revision = config.getInt("revision");
  if (revision <= 0) revision = 1;
  config.cd(configname);
  db->connect();
  ConfigObject m_obj;
  m_obj.setName(configname);
  m_obj.setNode(nodename);
  m_obj.setTable(tablename);
  m_obj.setRevision(revision);

  NodeInfoTable ntable(db);
  NodeInfoList node_v = ntable.getNodeInfos();
  ConfigInfoList config_v = ConfigInfoTable(db).getList();
  ConfigObjectTable dbtable(db);
  ConfigObjectList obj_v;
  for (StringList::iterator it = config.getLabels().begin();
       it != config.getLabels().end(); it++) {
    std::string label = *it;
    if (label.find(".type") != std::string::npos) {
      config.cd();
      std::string cnode = StringUtil::replace(label, ".type", "");
      ConfigObject obj;
      obj.setName(configname);
      obj.setTable(tablename + ".node");
      obj.setRevision(revision);
      obj.setNode(nodename);
      std::string value = StringUtil::tolower(config.get(cnode + ".used"));
      obj.addBool("used", value != "false");
      value = StringUtil::tolower(config.get(cnode + ".sequential"));
      obj.addBool("sequential", value != "false");
      config.cd(configname);
      StringList str_v = StringUtil::split(config.get(cnode + ".runtype"), ',');
      ConfigObject cobj;
      cobj.setNode(cnode);
      cobj.setTable(str_v[0]);
      cobj.setRevision(atoi(str_v[1].c_str()));
      cobj.setName(str_v[2]);
      obj.addObject("runtype", cobj);
      config.cd();
      obj.addEnum("type", config.get(cnode + ".type"));
      std::string dbmode = config.get(cnode + ".dbmode");
      if (dbmode.size() == 0) dbmode = "set";
      obj.addEnum("dbmode", dbmode);
      obj.setIndex(obj_v.size());
      obj_v.push_back(obj);
    }
  }
  m_obj.addObjects("node", obj_v);
  NodeInfoTable(db).addNode(NodeInfo(nodename, groupname));
  ConfigInfoTable(db).add(ConfigInfo(configname, nodename,
                                     tablename, revision));
  dbtable.add(obj_v, false);
  dbtable.add(m_obj, true);
  db->close();
  return 0;
}

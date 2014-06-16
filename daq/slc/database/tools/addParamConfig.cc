#include "daq/slc/database/FieldInfoTable.h"
#include "daq/slc/database/ConfigObjectTable.h"
#include "daq/slc/database/NodeInfoTable.h"
#include "daq/slc/database/ConfigInfoTable.h"
#include "daq/slc/database/TableInfoTable.h"

#include <daq/slc/database/PostgreSQLInterface.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

#include <fstream>
#include <iostream>
#include <cstdlib>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 4) {
    LogFile::debug("usage: %s <groupname> <tablename> <csvfile>", argv[0]);
    return 1;
  }
  std::string groupname = argv[1];
  std::string tablename = argv[2];
  std::string path = argv[3];
  ConfigFile dbconfig("slowcontrol");
  DBInterface* db = new PostgreSQLInterface(dbconfig.get("database.host"),
                                            dbconfig.get("database.dbname"),
                                            dbconfig.get("database.user"),
                                            dbconfig.get("database.password"),
                                            dbconfig.getInt("database.port"));
  db->connect();
  int revision = 1;
  TableInfo tinfo = TableInfoTable(db).get(tablename, revision);
  FieldInfoTable table(db);
  FieldInfoList info_v = table.getList(tablename, revision);
  std::map<std::string, FieldInfo> info_m;
  for (size_t i = 0; i < info_v.size(); i++) {
    info_m.insert(std::map<std::string, FieldInfo>::value_type(info_v[i].getName(),
                                                               info_v[i]));
  }

  std::ifstream fin(path.c_str());
  std::string line;
  getline(fin, line);
  line = StringUtil::replace(line, " ", "");
  std::vector<std::string> name_v = StringUtil::split(line, ',');
  std::string nodename, configname;
  int index = 0;
  while (fin && getline(fin, line)) {
    line = StringUtil::replace(line, " ", "");
    std::vector<std::string> value_v = StringUtil::split(line, ',');
    ConfigObject obj;
    for (size_t i = 0; i < value_v.size(); i++) {
      std::string name = name_v[i];
      std::string value = value_v[i];
      if (name == "nodename") {
        if (nodename != value) index = 0;
        nodename = value;
        NodeInfoTable ntable(db);
        ntable.addNode(NodeInfo(nodename, groupname));
        obj.setNode(nodename);
        obj.setTable(tablename);
        obj.setRevision(revision);
        continue;
      } else if (name == "config") {
        if (configname != value) index = 0;
        configname = value;
        obj.setName(configname);
        continue;
      }
      FieldInfo& info(info_m[name]);
      switch (info.getType()) {
        case FieldInfo::BOOL:   obj.addBool(name, value != "false"); break;
        case FieldInfo::SHORT:    {
          if (value.find("0x") != std::string::npos) {
            obj.addShort(name, strtol(value.c_str(), NULL, 0)); break;
          } else {
            obj.addShort(name, atoi(value.c_str())); break;
          }
        }
        case FieldInfo::INT:    {
          if (value.find("0x") != std::string::npos) {
            obj.addInt(name, strtol(value.c_str(), NULL, 0)); break;
          } else {
            obj.addInt(name, atoi(value.c_str())); break;
          }
        }
        case FieldInfo::LONG:    {
          if (value.find("0x") != std::string::npos) {
            obj.addLong(name, strtol(value.c_str(), NULL, 0)); break;
          } else {
            obj.addLong(name, atoi(value.c_str())); break;
          }
        }
        case FieldInfo::FLOAT:  obj.addFloat(name, atof(value.c_str())); break;
        case FieldInfo::TEXT:   obj.addText(name, value); break;
        case FieldInfo::OBJECT:  {
          ConfigObject cobj;
          StringList str_v = StringUtil::split(value, '/');
          std::string cnodename = nodename;
          std::string ctablename = tablename + "." + name;
          int crevision = revision;
          if (str_v.size() > 3) {
            cnodename = str_v[0];
            ctablename = str_v[1];
            crevision = atoi(str_v[2].c_str());
            value = str_v[3];
          }
          cobj.setName(value);
          cobj.setNode(cnodename);
          cobj.setTable(ctablename);
          cobj.setRevision(crevision);
          obj.addObject(name, cobj);
        } break;
        case FieldInfo::ENUM:
          obj.addEnum(name, value); break;
        default: break;
      }
    }
    obj.setIndex(index);
    ConfigObjectTable(db).add(obj, tinfo.isRoot());
    index++;
  }
  db->close();
  return 0;
}

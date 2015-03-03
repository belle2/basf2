#include <daq/slc/database/PostgreSQLInterface.h>
#include <daq/slc/database/DBObjectLoader.h>

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
    LogFile::debug("usage: %s <tablename> <conffile> <csvfile> [<parname>]", argv[0]);
    return 1;
  }
  ConfigFile config("slowcontrol");
  PostgreSQLInterface db(config.get("database.host"),
                         config.get("database.dbname"),
                         config.get("database.user"),
                         config.get("database.password"),
                         config.getInt("database.port"));

  std::string tablename = argv[1];
  std::string cfilename = argv[2];
  std::string csvfilename = argv[3];
  std::string parname = (argc > 4) ? argv[4] : "";
  const DBObject obj(DBObjectLoader::load(cfilename));
  std::ifstream fin(csvfilename.c_str());
  std::string line;
  getline(fin, line);
  line = StringUtil::replace(line, " ", "");
  std::vector<std::string> name_v = StringUtil::split(line, ',');
  std::string nodename, configname;
  int index = 0;
  DBObject obj_out;
  while (fin && getline(fin, line)) {
    line = StringUtil::replace(line, " ", "");
    std::vector<std::string> value_v = StringUtil::split(line, ',');
    DBObject cobj(obj);
    std::string nodename_tmp = value_v[0];
    std::string configname_tmp = value_v[1];
    if (obj_out.getNObjects(parname) > 0 &&
        (nodename_tmp != nodename || configname_tmp != configname)) {
      if (argc > 4) {
        DBObjectLoader::createDB(db, tablename, obj_out);
      }
      obj_out = DBObject();
    }
    nodename = nodename_tmp;
    configname = configname_tmp;
    if (argc > 4) {
      cobj.setName(parname);
    } else {
      cobj.setName(nodename + "@" + configname);
    }
    obj_out.setName(nodename + "@" + configname);
    for (size_t i = 2; i < value_v.size(); i++) {
      std::string name = name_v[i];
      std::string value = value_v[i];
      if (obj.getProperty(name).getType() == DBField::OBJECT) {
        cobj.getObject(name).setPath(value);
      } else {
        cobj.setValueText(name, value);
      }
    }
    if (argc > 4) {
      cobj.setIndex(index);
      obj_out.addObject(parname, cobj);
    } else {
      DBObjectLoader::createDB(db, tablename, cobj);
    }
    index++;
  }
  if (argc > 4) {
    DBObjectLoader::createDB(db, tablename, obj_out);
  }
  return 0;
}

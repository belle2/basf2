#include "daq/slc/database/DBObjectLoader.h"

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/ConfigFile.h>

#include <algorithm>
#include <cstdlib>
#include <iostream>

using namespace Belle2;

ConfigObject DBObjectLoader::load(const std::string& path,
                                  const std::string& tablename,
                                  bool search)
{
  std::string filename = (path.size() == 0) ? tablename : path + "/" + tablename + ".conf";
  std::cout << filename << std::endl;
  ConfigFile config(filename);
  int revision = config.getInt("revision");
  if (revision <= 0) revision = 1;
  ConfigObject obj;
  obj.setTable(tablename);
  obj.setRevision(revision);
  for (std::vector<std::string>::iterator it = config.getLabels().begin();
       it != config.getLabels().end(); it++) {
    std::string name = *it;
    if (name == "revision") continue;
    std::string value = config.get(name);
    std::string type_s = "text";
    std::string::size_type pos = value.find_first_of("(");
    FieldInfo::Type type = FieldInfo::INT;
    if (pos != std::string::npos) {
      type_s = value.substr(0, pos);
      if (type_s == "bool") { obj.addBool(name, false); }
      else if (type_s == "char") { obj.addChar(name, 0); }
      else if (type_s == "short") { obj.addShort(name, 0); }
      else if (type_s == "int") { obj.addInt(name, 0); }
      else if (type_s == "long") { obj.addLong(name, 0); }
      else if (type_s == "float") { obj.addFloat(name, 0); }
      else if (type_s == "double") { obj.addDouble(name, 0); }
      else if (type_s == "object") {
        ConfigObject cobj;
        if (search) {
          cobj = load(path, tablename + "." + name, search);
        }
        obj.addObject(name, cobj);
        type = FieldInfo::OBJECT;
      }
      if (type != FieldInfo::TEXT && type != FieldInfo::OBJECT) {
        value = StringUtil::replace(value.substr(pos + 1), ")", "");
        std::cout << name << " " << value << " " << type_s << std::endl;
      }
      obj.setValueText(name, value);
    } else {
      obj.addText(name, value);
    }
  }
  return obj;
}

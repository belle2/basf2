#include "daq/slc/database/DBObjectLoader.h"

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/ConfigFile.h>

#include <algorithm>
#include <cstdlib>

using namespace Belle2;

ConfigObject DBObjectLoader::load(const std::string& path,
                                  const std::string& tablename)
{
  std::string filename = (path.size() == 0) ? tablename : path + "/" + tablename + ".conf";
  ConfigFile config(filename);
  int revision = config.getInt("revision");
  if (revision <= 0) revision = 1;
  std::vector<std::string>& label_v(config.getDirectories());
  if (find(label_v.begin(), label_v.end(), "default") == label_v.end()) {
    LogFile::debug("Faield to find label 'default'");
    exit(1);
  }
  config.cd();
  config.cd("default");
  ConfigObject obj;
  obj.setTable(tablename);
  obj.setRevision(revision);
  for (std::vector<std::string>::iterator it = config.getLabels().begin();
       it != config.getLabels().end(); it++) {
    std::string name = *it;
    if (name.find("default.") == 0) {
      name = StringUtil::replace(name, "default.", "");
      std::string value = config.get(name);
      std::string type_s = "text";
      std::string::size_type pos = value.find_first_of("(");
      FieldInfo::Type type = FieldInfo::TEXT;
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
          /*
          StringList str_v = StringUtil::split(value, ',');
          std::string ctablename = tablename + "." + name;
          if (str_v.size() > 1) {
            ctablename = str_v[0];
            value = str_v[1];
          }
          obj.addObject(name, load(path, ctablename));
          */
          obj.addObject(name, ConfigObject());
        }
        if (type != FieldInfo::TEXT && type != FieldInfo::OBJECT) {
          value = StringUtil::replace(value.substr(pos + 1), ")", "");
        }
        obj.setValueText(name, value);
      } else {
        obj.addText(name, value);
      }
    }
  }
  config.cd();
  return obj;
}

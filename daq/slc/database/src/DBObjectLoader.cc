#include "daq/slc/database/DBObjectLoader.h"

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/ConfigFile.h>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>

using namespace Belle2;

DBObject DBObjectLoader::load(const std::string& filename)
{
  ConfigFile config(filename);
  return load(config);
}

DBObject DBObjectLoader::load(ConfigFile& config)
{
  const std::string nodename = config.get("nodename");
  const std::string configname = config.get("config");
  DBObject obj;
  if (nodename.size() > 0)
    obj.setName(nodename + "@" + configname);
  else
    obj.setName(configname);
  for (StringList::iterator it = config.getLabels().begin();
       it != config.getLabels().end(); it++) {
    const std::string name = *it;
    StringList str = StringUtil::split(name, '.');
    if (str[0] == "config") continue;
    if (str[0] == "nodename") continue;
    std::string value = config.get(name);
    std::string::size_type pos = value.find_first_of("(");
    DBField::Type type = DBField::TEXT;
    if (pos != std::string::npos) {
      std::string type_s = value.substr(0, pos);
      if (type_s == "bool") type = DBField::BOOL;
      else if (type_s == "char") type = DBField::CHAR;
      else if (type_s == "short") type = DBField::SHORT;
      else if (type_s == "int") type = DBField::INT;
      else if (type_s == "long") type = DBField::LONG;
      else if (type_s == "float") type = DBField::FLOAT;
      else if (type_s == "double") type = DBField::DOUBLE;
      else if (type_s == "object") type = DBField::OBJECT;
      if (type != DBField::TEXT) {
        value = StringUtil::replace(value.substr(pos + 1), ")", "");
      }
    } else {
      float vf;
      if (StringUtil::tolower(value) == "true") {
        type = DBField::BOOL;
      } else if (StringUtil::tolower(value) == "false") {
        type = DBField::BOOL;
      } else if (StringUtil::isdigit(value)) {
        type = DBField::INT;
      } else if (StringUtil::split(value, '.').size() < 3 &&
                 StringUtil::isdigit(StringUtil::replace(value, ".", "")) &&
                 sscanf(value.c_str(), "%f", &vf) == 1) {
        type = DBField::FLOAT;
      } else {
        type = DBField::TEXT;
      }
    }
    if (!setObject(obj, str, type, value)) {
      LogFile::error("error : %s : %s", name.c_str(), value.c_str());
    }
  }
  return obj;
}

DBObject DBObjectLoader::load(DBInterface& db,
                              const std::string& tablename,
                              const std::string& configname, bool isfull)
{
  const std::string rootnode = "." + configname;
  if (!db.isConnected()) {
    db.connect();
  }
  db.execute("select * from %s p where p.path like '%s._%%' and not exists "
             "(select * from (select * from %s where path like '%s._%%') c where c.path "
             "like p.path || '_%%') order by id",
             tablename.c_str(), rootnode.c_str(),
             tablename.c_str(), rootnode.c_str());
  DBRecordList record_v(db.loadRecords());
  DBObject obj;
  obj.setName(configname);
  for (size_t i = 0; i < record_v.size(); i++) {
    DBRecord& record(record_v[i]);
    std::string value;
    DBField::Type type = DBField::OBJECT;
    std::string table_in = tablename;
    std::string config_in;
    if (record.hasField("value_b")) {
      value = record.get("value_b");
      type = DBField::BOOL;
    } else if (record.hasField("value_c")) {
      value = record.get("value_c");
      type = DBField::CHAR;
    } else if (record.hasField("value_s")) {
      value = record.get("value_s");
      type = DBField::SHORT;
    } else if (record.hasField("value_i")) {
      value = record.get("value_i");
      type = DBField::INT;
    } else if (record.hasField("value_f")) {
      value = record.get("value_f");
      type = DBField::FLOAT;
    } else if (record.hasField("value_d")) {
      value = record.get("value_d");
      type = DBField::DOUBLE;
    } else if (record.hasField("value_t")) {
      value = record.get("value_t");
      type = DBField::TEXT;
    } else if (record.hasField("value_o")) {
      value = record.get("value_o");
      type = DBField::OBJECT;
      std::string name = record.get("name");
      table_in = tablename;
      config_in = value;
      StringList ss = StringUtil::split(value, '/');
      if (ss.size() > 1) {
        table_in = ss[0];
        config_in = ss[1];
      }
      //if (config_in.find("@") == std::string::npos) {
      //  const std::string nodename = StringUtil::split(configname, '@')[0];
      //  config_in = nodename + "@" + config_in;
      //}
      if (table_in == tablename && config_in == configname) {
        LogFile::error("recursive call of %s/%s",
                       table_in.c_str(), config_in.c_str());
      }
    }
    std::string path = record.get("path");
    path = path.substr(0, path.size() - 1);
    path = StringUtil::replace(path, rootnode, "");
    path = path.substr(1);
    StringList str = StringUtil::split(path, '.');
    if (!DBObjectLoader::setObject(obj, str, type, value, table_in, config_in,
                                   (isfull ? &db : NULL))) {
      LogFile::error("error : %s : %s", record.get("path").c_str(), value.c_str());
    }
  }
  return obj;
}

bool DBObjectLoader::add(DBObject& obj, StringList& str,
                         const std::string& name_in, const DBObject& cobj)
{
  std::string name = str[0];
  int index = 0;
  StringList sstr = StringUtil::split(str[0], '[');
  if (sstr.size() > 1) {
    index = atoi(sstr[1].c_str());
    name = sstr[0];
  } else {
    name = str[0];
    index = 0;
  }
  if (str.size() > 1) {
    str.erase(str.begin());
    if (obj.hasObject(name)) {
      return add(obj.getObject(name, index), str, name_in, cobj);
    }
    throw (std::out_of_range(StringUtil::form("%s:%d %s", __FILE__, __LINE__,
                                              name.c_str())));
  } else {
    if (obj.hasObject(name) && obj.getNObjects(name) > index) {
      obj.getObject(name, index).addObject(name_in, cobj);
      return true;
    }
    obj.addObject(name, cobj);
    return true;
  }
  throw (std::out_of_range(StringUtil::form("%s:%d %s", __FILE__, __LINE__,
                                            StringUtil::join(str, ".").c_str())));
}

bool DBObjectLoader::setObject(DBObject& obj, StringList& str,
                               DBField::Type type, const std::string& value,
                               const std::string& table_in, const std::string& config_in,
                               DBInterface* db)
{
  std::string name;
  int index = 0;
  if (str.size() > 0) {
    StringList sstr = StringUtil::split(str[0], '[');
    if (sstr.size() > 1) {
      index = atoi(sstr[1].c_str());
      name = sstr[0];
    } else {
      name = str[0];
      index = 0;
    }
  } else {
    return false;
  }
  if (str.size() > 1) {
    str.erase(str.begin());
    bool found = obj.hasObject(name);
    if (found) {
      DBObjectList& objs(obj.getObjects(name));
      for (DBObjectList::iterator it = objs.begin();
           it != objs.end(); it++) {
        if (it->getIndex() == index) {
          found = false;
          break;
        }
      }
      found = !found;
    }
    if (!found) {
      DBObject cobj;
      cobj.setName(name);
      cobj.setIndex(index);
      obj.addObject(name, cobj);
    }
    DBObjectList& objs(obj.getObjects(name));
    for (DBObjectList::iterator it = objs.begin();
         it != objs.end(); it++) {
      DBObject& cobj(*it);
      if (cobj.getIndex() == index) {
        return setObject(cobj, str, type, value, table_in, config_in, db);
      }
    }
  } else {
    switch (type) {
      case DBField::BOOL:   obj.addBool(name, false); break;
      case DBField::CHAR:   obj.addChar(name, 0); break;
      case DBField::SHORT:  obj.addShort(name, 0); break;
      case DBField::INT:    obj.addInt(name, 0); break;
      case DBField::LONG:   obj.addLong(name, 0); break;
      case DBField::FLOAT:  obj.addFloat(name, 0); break;
      case DBField::DOUBLE: obj.addDouble(name, 0); break;
      case DBField::TEXT:   obj.addText(name, value); break;
      case DBField::OBJECT: {
        DBObject cobj(value);
        if (db) {
          cobj = DBObjectLoader::load(*db, table_in, config_in, true);
        }
        if (cobj.hasObject(name)) {
          cobj.getObject(name).setPath(table_in + "/" + config_in);
          obj.addObjects(name, cobj.getObjects(name));
        } else {
          cobj.setName(name);
          obj.addObject(name, cobj);
        }
      } break;
      default : return false;
    }
    obj.setValueText(name, value);
    return true;
  }
  return false;
}

bool DBObjectLoader::createDB(DBInterface& db,
                              const std::string& tablename,
                              const DBObject& obj)
{
  std::stringstream ss;
  obj.printSQL(tablename, ss);
  try {
    if (!db.isConnected()) db.connect();
  } catch (const DBHandlerException& e) {
    LogFile::error(e.what());
    return false;
  }
  try {
    db.execute("create table %s \n"
               "(name  varchar(64) \n"
               "check (replace(name, '.', '') = name) not null, \n"
               "path varchar(256) not null, \n"
               "id bigserial, \n"
               "record_time timestamp default current_timestamp, \n"
               "value_b boolean default NULL, \n"
               "value_c char default NULL, \n"
               "value_s smallint default NULL, \n"
               "value_i int default NULL, \n"
               "value_l bigint default NULL, \n"
               "value_f float default NULL, \n"
               "value_d double precision default NULL, \n"
               "value_t text default NULL, \n"
               "value_o varchar(256) default NULL, \n"
               "UNIQUE (path)); ", tablename.c_str());
    db.execute("create index %s_id_index on %s(id);",
               tablename.c_str(), tablename.c_str());
  } catch (const DBHandlerException& e) {
  }
  try {
    db.execute(ss.str().c_str());
  } catch (const DBHandlerException& e) {
    LogFile::error(e.what());
    return false;
  }
  return true;
}

StringList DBObjectLoader::getDBlist(DBInterface& db,
                                     const std::string& tablename,
                                     const std::string& nodename,
                                     const std::string& grep)
{
  StringList str;
  try {
    if (!db.isConnected()) db.connect();
    if (nodename.size() > 0) {
      if (grep.size() > 0) {
        db.execute("select name from %s where name = REPLACE(path, '.', '') and name like '%s@%s_%';",
                   tablename.c_str(), nodename.c_str(), grep.c_str());
      } else {
        db.execute("select name from %s where name = REPLACE(path, '.', '') and name like '%s@_%';",
                   tablename.c_str(), nodename.c_str());
      }
    } else {
      if (grep.size() > 0) {
        db.execute("select name from %s where name = REPLACE(path, '.', '') and name like '_%%@%s_%';",
                   tablename.c_str(), grep.c_str());
      } else {
        db.execute("select name from %s where name = REPLACE(path, '.', '');",
                   tablename.c_str());
      }
    }
    DBRecordList record_v(db.loadRecords());
    for (size_t i = 0; i < record_v.size(); i++) {
      DBRecord& record(record_v[i]);
      str.push_back(record.get("name"));
    }
  } catch (const DBHandlerException& e) {

  }
  return str;
}

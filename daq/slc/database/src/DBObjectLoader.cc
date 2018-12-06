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
      else if (type_s == "int") type = DBField::INT;
      else if (type_s == "float") type = DBField::FLOAT;
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
      } else if ((value.size() >= 2 && value.at(0) == '0' && value.at(1) == 'x') ||
                 StringUtil::isdigit(value)) {
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
                              const std::string& config_in, bool isfull)
{
  std::string configname = config_in;
  if (!db.isConnected()) {
    db.connect();
  }
  DBObject obj;
  StringList list = DBObjectLoader::getDBlist(db, tablename, configname);
  if (list.size() == 0) return obj;
  StringList s = StringUtil::split(list[0], ',');
  std::stringstream ss;
  ss << "select * from " << s[1] << " where pid =" << s[2] << " order by id";
  db.execute(ss.str());
  DBRecordList record_v(db.loadRecords());
  if (list.size() > 0) {
    configname = s[0];
  }
  ss.str("");
  ss << " config : " << configname << std::endl;
  for (size_t i = 0; i < record_v.size(); i++) {
    DBRecord& record(record_v[i]);
    if (record.hasField("value_b")) {
      ss << record.get("name") <<  " : " <<
         (record.getBool("value_b") ? "true" : "false") << std::endl;
    } else if (record.hasField("value_i")) {
      ss << record.get("name") <<  " : int(" << record.get("value_i") << ")" << std::endl;
    } else if (record.hasField("value_f")) {
      ss << record.get("name") <<  " : float(" << record.get("value_f") << ")" << std::endl;
    } else if (record.hasField("value_t")) {
      ss << record.get("name") <<  " : \"" << record.get("value_t") << "\"" << std::endl;
    }
  }
  ConfigFile conf(ss);
  return DBObjectLoader::load(conf);
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
      case DBField::INT:    obj.addInt(name, 0); break;
      case DBField::FLOAT:  obj.addFloat(name, 0); break;
      case DBField::TEXT:   obj.addText(name, value); break;
      case DBField::OBJECT: {
        DBObject cobj(value);
        std::string config_out = config_in;
        //LogFile::notice(config_out);
        if (db) {
          StringList list = DBObjectLoader::getDBlist(*db, table_in, config_out);
          if (list.size() > 0) {
            //LogFile::notice("%s:%d", __FILE__, __LINE__);
            bool found = false;
            for (size_t i = 0; i < list.size(); i++) {
              if (config_out == list[0]) {
                found = true;
                break;
              }
            }
            if (found) {
              cobj = DBObjectLoader::load(*db, table_in, config_out, true);
            } else {
              if (list.size() > 0) {
                config_out = list[0];
                cobj = DBObjectLoader::load(*db, table_in, config_out, true);
              }
            }
          }
        }
        if (cobj.hasObject(name)) {
          cobj.getObject(name).setPath(table_in + "/" + config_out);
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
  std::string tablename_date = tablename + "_" + Date().toString("%Y");
  std::string tablename_id = tablename + "_id";
  try {
    if (!db.isConnected()) db.connect();
  } catch (const DBHandlerException& e) {
    LogFile::error(e.what());
    return false;
  }
  try {
    if (obj.getName().size() == 0) {
      LogFile::error("Configname is null. createDB canceled.");
      return false;
    }
    if (!db.checkTable("daqconfig")) {
      db.execute("create table daqconfig \n"
                 "(name text not null, \n"
                 "id bigserial, lastupdate timestamp, \n"
                 "UNIQUE(name));");
      db.execute("create index daqconfig_id_index on daqconfig(id);",
                 tablename.c_str(), tablename.c_str());
    }
    if (!db.checkTable(tablename_id)) {
      try {
        db.execute("insert into daqconfig (name, lastupdate) values "
                   "('%s', current_timestamp);", tablename_id.c_str());
      } catch (const std::exception& e) {
        db.execute("update daqconfig set lastupdate = current_timestamp where name = '%s';",
                   tablename_id.c_str());
      }
      db.execute("create table %s \n"
                 "(name text not null \n"
                 "check (replace(name, '.', '') = name) not null, \n"
                 "id bigserial, \n"
                 "content text, \n"
                 "record_time timestamp with time zone default current_timestamp, \n"
                 "UNIQUE (name)); ", tablename_id.c_str());
    }
    if (!db.checkTable(tablename_date)) {
      try {
        db.execute("insert into daqconfig (name, lastupdate) values "
                   "('%s', current_timestamp);", tablename_date.c_str());
      } catch (const std::exception& e) {
        db.execute("update daqconfig set lastupdate = current_timestamp where name = '%s';",
                   tablename_date.c_str());
      }
      db.execute("create table %s \n"
                 "(name text not null,\n"
                 "id bigserial, \n"
                 "pid bigint, \n"
                 "value_b boolean default NULL, \n"
                 "value_i int default NULL, \n"
                 "value_f float default NULL, \n"
                 "value_t text default NULL \n"
                 "); ", tablename_date.c_str());
      db.execute("create index %s_index on %s(id);",
                 tablename_date.c_str(), tablename_date.c_str());
    }
  } catch (const DBHandlerException& e) {
    LogFile::warning(e.what());
  }
  try {
    db.execute("select max(id) as id from %s;", tablename_id.c_str());
    int id = 1;
    DBRecordList record(db.loadRecords());
    if (record.size() > 0) {
      id = record[0].getInt("id") + 1;
    }
    db.execute("insert into %s (name, id, content) values ('%s', %d, '%s');",
               tablename_id.c_str(), obj.getName().c_str(), id, tablename_date.c_str());
    std::string s = obj.printSQL(tablename_date, id);
    db.execute(s);
  } catch (const DBHandlerException& e) {
    LogFile::error(e.what());
    return false;
  }
  return true;
}

StringList DBObjectLoader::getDBlist(DBInterface& db,
                                     const std::string& tablename,
                                     const std::string& grep, int max)
{
  StringList str;
  try {
    if (!db.isConnected()) db.connect();
    if (grep.size() > 0) {
      const char* prefix = grep.c_str();
      std::stringstream ss;
      ss << "select id,name,content,to_char(record_time,'DD/MM HH24:MI:SS') as tdate from " << tablename << "_id where "
         << "(name like '" << prefix << "_%' or "
         << "name like '" << prefix << "') order by id desc";
      if (max > 0) ss << " limit " << max;
      ss << ";";
      db.execute(ss.str());
    } else {
      std::stringstream ss;
      ss << "select id,name,content,to_char(record_time,'DD/MM HH24:MI:SS') as tdate from " << tablename << "_id order by id desc";
      if (max > 0) ss << " limit " << max;
      ss << ";";
      db.execute(ss.str());
    }
    DBRecordList record_v(db.loadRecords());
    for (size_t i = 0; i < record_v.size(); i++) {
      DBRecord& record(record_v[i]);
      str.push_back(record.get("name") + "," + record.get("content") + "," +
                    record.get("id") + "," + record.get("tdate"));
    }
  } catch (const DBHandlerException& e) {
    LogFile::error(e.what());
  }
  return str;
}

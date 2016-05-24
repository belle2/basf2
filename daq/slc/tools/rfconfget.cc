#include <daq/slc/base/StringUtil.h>

#include <daq/slc/psql/PostgreSQLInterface.h>
#include <daq/slc/database/DBObjectLoader.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/ConfigFile.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <cstdlib>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 3) {
    std::cout << "Usage : " << argv[0] << " <table> <confname> [<outputfile>]" << std::endl;
    return 1;
  }
  std::string table = argv[1];
  std::string confname = argv[2];
  ConfigFile config("slowcontrol");
  PostgreSQLInterface db(config.get("database.host"),
                         config.get("database.dbname"),
                         config.get("database.user"),
                         config.get("database.password"),
                         config.getInt("database.port"));
  DBObject obj = DBObjectLoader::load(db, table, confname);
  db.close();

  std::stringstream sin;
  DBObject::NameValueList map;
  obj.search(map);
  size_t length = 0;
  for (DBObject::NameValueList::iterator it = map.begin();
       it != map.end(); it++) {
    if (it->name.size() > length) length = it->name.size();
  }
  length++;
  for (DBObject::NameValueList::iterator it = map.begin();
       it != map.end(); it++) {
    if (it->name.size() > 0 && it->name.at(0) == '$') {
      sin << StringUtil::form("%s:%s", it->name.c_str(), it->value.c_str()) << std::endl;
    } else {
      sin << StringUtil::form(StringUtil::form("%%-%ds  %%s", length).c_str(),
                              (it->name + ":").c_str(), it->value.c_str()) << std::endl;
    }
  }
  std::string s;
  std::vector<std::string> s_out;
  std::map<int, std::string> is_out;
  while (sin && getline(sin, s)) {
    if (s.size() == 0) continue;
    if (s.at(0) == '#') continue;
    if (s.at(0) == '$') {
      if (s.find("$$") == std::string::npos) {
        int i = atoi(StringUtil::replace(s, "$", "").c_str());
        StringList ss = StringUtil::split(s, ':', 2);
        s = StringUtil::join(ss, ":", 1);
        is_out.insert(std::map<int, std::string>::value_type(i, s));
      } else {
        int i = atoi(StringUtil::replace(s, "$", "").c_str());
        StringList ss = StringUtil::split(s, ':', 2);
        s = "$" + StringUtil::join(ss, ":", 1);
        is_out.insert(std::map<int, std::string>::value_type(i, s));
      }
    } else {
      s_out.push_back(s);
    }
  }
  for (std::map<int, std::string>::iterator it = is_out.begin();
       it != is_out.end(); it++) {
    int i = it->first;
    std::string s = it->second;
    if (s.size() == 0 || s.at(0) != '$') {
      s_out.insert(s_out.begin() + i, "#" + s);
    }
  }
  int i = 0;
  std::ofstream fout;
  if (argc > 3) fout.open(argv[3]);
  std::ostream& os((argc > 3) ? fout : std::cout);
  int count = 0;
  for (std::vector<std::string>::iterator it = s_out.begin();
       it != s_out.end(); it++) {
    if (is_out.find(i) != is_out.end()) {
      if (is_out[i].size() > 0 && is_out[i].at(0) == '$') {
        os << *it << "   # " << is_out[i].substr(1) << std::endl;
        i++;
        continue;
      }
    }
    if (it->size() > 0 && it->at(0) == '#') {
      if (i > 0 && count == 0)
        os << std::endl;
      count++;
    } else {
      count = 0;
    }
    os << *it << std::endl;
    i++;
  }
}

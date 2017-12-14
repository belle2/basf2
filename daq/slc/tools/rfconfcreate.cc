#include <daq/slc/base/StringUtil.h>

#include <daq/slc/psql/PostgreSQLInterface.h>
#include <daq/slc/database/DBObjectLoader.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

#include <iostream>
#include <fstream>
#include <sstream>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 2) {
    std::cout << "Usage : " << argv[0] << " <rfconf> <table>" << std::endl;
  }
  std::fstream fin(argv[1]);
  std::string table = argv[2];
  std::string s;
  int count1 = 0;
  std::stringstream sout;
  while (fin && getline(fin, s)) {
    if (s.size() == 0) continue;
    if (s.at(0) == '#') {
      sout << StringUtil::form("$%d:", count1) << "\"" << s.substr(1) << "\"" << std::endl;
    } else if (s.find("#") > 2) {
      StringList ss = StringUtil::split(s, '#');
      sout << ss[0] << std::endl;
      if (ss.size() > 1) {
        sout << StringUtil::form("$$%d:", count1) << "\"" << ss[1] << "\"" << std::endl;
      }
    } else {
      sout << s << std::endl;
    }
    s = StringUtil::replace(StringUtil::split(s, ':')[0], " ", "");
    if (!(s == "config" || s == "nodename")) {
      count1++;
    }
  }
  std::cout << sout.str();
  ConfigFile config("slowcontrol");
  PostgreSQLInterface db(config.get("database.host"),
                         config.get("database.dbname"),
                         config.get("database.user"),
                         config.get("database.password"),
                         config.getInt("database.port"));
  ConfigFile rfconf(sout);
  DBObject obj = DBObjectLoader::load(rfconf);
  DBObjectLoader::createDB(db, table, obj);
  db.close();
  printf("new rfconf %s %s stored\n", table.c_str(), obj.getName().c_str());
  return 0;
}

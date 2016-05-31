#include <daq/slc/psql/PostgreSQLInterface.h>
#include <daq/slc/database/DBObjectLoader.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

#include <iostream>
#include <cstring>
#include <cstdlib>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 2) {
    LogFile::debug("usage: %s <tablename> [-e expno] [-r runno] [-n <node>] [--html]", argv[0]);
    return 1;
  }

  const std::string tablename = argv[1];
  std::string nodename;
  int expno = -1;
  int runno = -1;
  int max = -1;
  std::string prefix = "";
  bool htmlmode = false;
  for (int i = 2; i < argc; i++) {
    prefix = argv[i];
  }

  ConfigFile config("slowcontrol");
  PostgreSQLInterface db(config.get("database.host"),
                         config.get("database.dbname"),
                         config.get("database.user"),
                         config.get("database.password"),
                         config.getInt("database.port"));
  StringList list = DBObjectLoader::getDBlist(db, tablename, prefix, max);
  if (htmlmode) {
    printf("<table>\n");
    //printf("<caption><strong>%s</strong></caption>\n", getName().c_str());
    // printf("<thead><tr><th>Name</th><th>Value</th>\n</tr></thead>\n");
    printf("<tbody>\n");
    for (size_t i = 0; i < list.size(); i++) {
      printf("<tr><td><a href=\"./daqconfig.php?db=%s&config=%s\">%s</a></td>\n",
             tablename.c_str(), list[i].c_str(), list[i].c_str());
    }
    printf("</tbody>\n");
    printf("</table>\n");
  } else {
    for (size_t i = 0; i < list.size(); i++) {
      std::cout << list[i] << std::endl;
    }
  }
  return 0;
}


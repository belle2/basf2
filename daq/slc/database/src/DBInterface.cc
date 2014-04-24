#include "daq/slc/database/DBInterface.h"

#include <daq/slc/system/LogFile.h>

#include <cstdarg>
#include <cstdio>

using namespace Belle2;

DBInterface::DBInterface(const std::string& host,
                         const std::string& database,
                         const std::string& user,
                         const std::string& password, int port) throw()
{
  _host = host;
  _database = database;
  _user = user;
  _password = password;
  _port = port;
}

DBInterface::~DBInterface() throw()
{

}

void DBInterface::execute(const std::string& text, ...) throw(DBHandlerException)
{
  va_list ap;
  static char ss[1024 * 100];
  va_start(ap, text);
  vsprintf(ss, text.c_str(), ap);
  va_end(ap);
  //LogFile::debug(ss);
  execute_imp(ss);
}

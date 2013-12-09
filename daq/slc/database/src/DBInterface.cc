#include "daq/slc/database/DBInterface.h"

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

#include "daq/slc/database/DBInterface.h"

#include <daq/slc/system/LogFile.h>

#include <cstdarg>
#include <cstdio>
//#include <iostream>

using namespace Belle2;

DBInterface::DBInterface(const std::string& host,
                         const std::string& database,
                         const std::string& user,
                         const std::string& password, int port) throw()
{
  m_host = host;
  m_database = database;
  m_user = user;
  m_password = password;
  m_port = port;
}

DBInterface::~DBInterface() throw()
{

}

void DBInterface::execute(const char* text, ...) throw(DBHandlerException)
{
  va_list ap;
  va_start(ap, text);
  vsprintf(m_buf, text, ap);
  va_end(ap);
  //LogFile::debug(m_buf);
  //std::cout << m_buf << std::endl;
  execute_imp(m_buf);
}

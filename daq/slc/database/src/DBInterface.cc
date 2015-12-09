#include "daq/slc/database/DBInterface.h"

#include <daq/slc/system/LogFile.h>

#include <cstdarg>
#include <cstdio>
#include <iostream>

using namespace Belle2;

DBInterface::DBInterface(const std::string& host,
                         const std::string& database,
                         const std::string& user,
                         const std::string& password, int port) throw()
{
  init(host, database, user, password, port);
}

DBInterface::~DBInterface() throw()
{
  delete [] m_buf;
}

void DBInterface::init(const std::string& host,
                       const std::string& database,
                       const std::string& user,
                       const std::string& password, int port) throw()
{
  m_host = host;
  m_database = database;
  m_user = user;
  m_password = password;
  m_port = port;
  m_buf = new char[1024000];
}

void DBInterface::execute(const char* text, ...) throw(DBHandlerException)
{
  va_list ap;
  va_start(ap, text);
  vsprintf(m_buf, text, ap);
  va_end(ap);
  //LogFile::debug(m_buf);
  execute_imp(m_buf);
}

void DBInterface::execute(const std::string& text) throw(DBHandlerException)
{
  execute_imp(text.c_str());
}


/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/slc/database/DBInterface.h"

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/system/LockGuard.h>
#include <daq/slc/system/LogFile.h>

#include <cstdarg>
#include <cstdio>

using namespace Belle2;

DBInterface::DBInterface(const std::string& host,
                         const std::string& database,
                         const std::string& user,
                         const std::string& password, int port)
{
  init(host, database, user, password, port);
}

DBInterface::~DBInterface()
{
  delete [] m_buf;
}

void DBInterface::init(const std::string& host,
                       const std::string& database,
                       const std::string& user,
                       const std::string& password, int port)
{
  m_host = host;
  m_database = database;
  m_user = user;
  m_password = password;
  m_port = port;
  m_buf = new char[m_buf_size];
}

void DBInterface::execute(const char* text, ...)
{
  StringList s;

  {
    LockGuard lockGuard(m_mutex);

    va_list ap;
    va_start(ap, text);
    vsnprintf(m_buf, m_buf_size, text, ap);
    va_end(ap);
    //std::cout << m_buf << std::endl;
    //execute_imp(m_buf);
    s = StringUtil::split(m_buf, ';');
  }

  for (size_t i = 0; i < s.size(); i++)
    execute_imp(s[i].c_str());
}

void DBInterface::execute(const std::string& text)
{
  //std::cout << text << std::endl;
  StringList s = StringUtil::split(text, ';');
  for (size_t i = 0; i < s.size(); i++)
    execute_imp(s[i].c_str());
}


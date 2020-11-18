#include "daq/slc/psql/PostgreSQLInterface.h"
#include "daq/slc/base/ConfigFile.h"

#ifndef NOT_USE_PSQL
#include <pgsql/libpq-fe.h>
#endif

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/database/DBHandlerException.h>

#include <daq/slc/system/LockGuard.h>

using namespace Belle2;

PostgreSQLInterface::PostgreSQLInterface(const std::string& host,
                                         const std::string& database,
                                         const std::string& user,
                                         const std::string& password,
                                         int port)
{
  init(host, database, user, password, port);
  m_sq_conn = NULL;
  m_sq_result = NULL;
}

PostgreSQLInterface::PostgreSQLInterface()
{
  ConfigFile config("slowcontrol");
  init(config.get("database.host"),
       config.get("database.dbname"),
       config.get("database.user"),
       config.get("database.password"),
       config.getInt("database.port"));
  m_sq_conn = NULL;
  m_sq_result = NULL;
}

void PostgreSQLInterface::connect()
{
#ifndef NOT_USE_PSQL
  if (isConnected()) return;

  {
    LockGuard lockGuard(m_mutex);
    m_sq_conn = PQconnectdb(StringUtil::form("host=%s dbname=%s user=%s password=%s port=%d",
                                             m_host.c_str(), m_database.c_str(),
                                             m_user.c_str(), m_password.c_str(),
                                             m_port).c_str());
  }

  if (PQstatus(m_sq_conn) == CONNECTION_BAD) {
    DBHandlerException exception("Failed to connect to the database : %s",
                                 PQerrorMessage(m_sq_conn));
    close();
    throw exception;
  }
#else
  throw (DBHandlerException("PGLIB is not available"));
#endif
}

bool PostgreSQLInterface::isConnected()
{
#ifndef NOT_USE_PSQL
  LockGuard lockGuard(m_mutex);
  bool connected = m_sq_conn != NULL && PQstatus(m_sq_conn) == CONNECTION_OK;
  return connected;
#else
  return false;
#endif
}

void PostgreSQLInterface::execute_imp(const char* command)
{
  clear();
#ifndef NOT_USE_PSQL
  LockGuard lockGuard(m_mutex);
  m_sq_result = PQexec(m_sq_conn, command);
  ExecStatusType status = PQresultStatus(m_sq_result);
  if (status == PGRES_FATAL_ERROR) {
    // Need to pre-generate exception to avoid data race
    // on PQerrorMessage.
    DBHandlerException exception("Failed to execute command : %s (%s)",
                                 command, PQerrorMessage(m_sq_conn));
    throw (exception);
  }
#else
  throw (DBHandlerException("libpg is not available"));
#endif
}

DBRecordList PostgreSQLInterface::loadRecords()
{
#ifndef NOT_USE_PSQL
  LockGuard lockGuard(m_mutex);
  if (PQresultStatus(m_sq_result) != PGRES_TUPLES_OK) {
    throw (DBHandlerException("DB records are not ready for reading"));
  }
  const size_t nrecords = PQntuples(m_sq_result);
  const size_t nfields = PQnfields(m_sq_result);
  m_record_v = DBRecordList();
  std::vector<std::string> name_v;
  for (size_t ifield = 0; ifield < nfields; ifield++) {
    const char* name = PQfname(m_sq_result, ifield);
    if (name != NULL) name_v.push_back(name);
  }
  for (size_t irecord = 0; irecord < nrecords; irecord++) {
    DBRecord record;
    for (size_t ifield = 0; ifield < nfields; ifield++) {
      if (!PQgetisnull(m_sq_result, irecord, ifield)) {
        const char* value = PQgetvalue(m_sq_result, irecord, ifield);
        if (value != NULL) {
          record.add(name_v[ifield], value);
        }
      }
    }
    m_record_v.push_back(record);
  }

  // Vector copy must be done before mutex release,
  // otherwise data race is introduced.
  DBRecordList ret(m_record_v);

  return ret;
#else
  throw (DBHandlerException("libpg is not available"));
#endif
}

void PostgreSQLInterface::clear()
{
#ifndef NOT_USE_PSQL
  LockGuard lockGuard(m_mutex);
  if (m_sq_result != NULL) {
    PQclear(m_sq_result);
    m_sq_result = NULL;
  }
#else
  throw (DBHandlerException("libpg is not available"));
#endif
}

void PostgreSQLInterface::close()
{
  clear();
#ifndef NOT_USE_PSQL
  LockGuard lockGuard(m_mutex);
  if (m_sq_conn != NULL) {
    PQfinish(m_sq_conn);
    m_sq_conn = NULL;
  }
#else
  throw (DBHandlerException("libpg is not available"));
#endif
}

bool PostgreSQLInterface::checkTable(const std::string& tablename)
{
#ifndef NOT_USE_PSQL
  execute("select relname from pg_stat_user_tables where relname='%s';",
          tablename.c_str());
  DBRecordList ret(loadRecords());
  return ret.size() > 0;
#else
  throw (DBHandlerException("libpg is not available"));
#endif
}

DBFieldTypeList PostgreSQLInterface::getTableContents(const std::string& tablename)
{
#ifndef NOT_USE_PSQL
  DBFieldTypeList name_m;
  execute("select attname, typname from pg_class, pg_attribute, pg_type "
          "where relkind ='r'and relname = '%s' and attrelid = relfilenode "
          "and attnum > 0 and pg_type.oid = atttypid;", tablename.c_str());
  DBRecordList ret(loadRecords());
  for (size_t i = 0; i < ret.size(); i++) {
    name_m.insert(DBFieldTypeList::value_type(ret[i].get("attname"),
                                              ret[i].get("typname")));
  }
  return name_m;
#else
  throw (DBHandlerException("libpg is not available"));
#endif
}

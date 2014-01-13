#include "daq/slc/database/PostgreSQLInterface.h"

#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

PostgreSQLInterface::PostgreSQLInterface(const std::string& host,
                                         const std::string& database,
                                         const std::string& user,
                                         const std::string& password,
                                         int port) throw()
  : DBInterface(host, database, user, password, port),
    _sq_conn(NULL), _sq_result(NULL) {}

void PostgreSQLInterface::connect() throw(DBHandlerException)
{
  _sq_conn = PQconnectdb(Belle2::form("host=%s dbname=%s user=%s password=%s",
                                      _host.c_str(), _database.c_str(),
                                      _user.c_str(), _password.c_str()).c_str());
  if (PQstatus(_sq_conn) == CONNECTION_BAD) {
    throw (DBHandlerException(__FILE__, __LINE__,
                              Belle2::form("Failed to connect to the database : (%s)",
                                           PQerrorMessage(_sq_conn))));
  }
}

void PostgreSQLInterface::execute(const std::string& command)
throw(DBHandlerException)
{
  clear();
  _sq_result = PQexec(_sq_conn, command.c_str());
  ExecStatusType status = PQresultStatus(_sq_result);
  if (status == PGRES_FATAL_ERROR) {
    throw (DBHandlerException(__FILE__, __LINE__,
                              Belle2::form("Failed to execute command : %s (%s)",
                                           command.c_str(), PQerrorMessage(_sq_conn))));
  }
}

DBRecordList& PostgreSQLInterface::loadRecords() throw(DBHandlerException)
{
  if (PQresultStatus(_sq_result) != PGRES_TUPLES_OK) {
    throw (DBHandlerException(__FILE__, __LINE__, "Failed to get records"));
  }
  const size_t nrecords = PQntuples(_sq_result);
  const size_t nfields = PQnfields(_sq_result);
  _record_v = DBRecordList();
  std::vector<std::string> name_v;
  for (size_t ifield = 0; ifield < nfields; ifield++) {
    const char* name = PQfname(_sq_result, ifield);
    if (name != NULL) name_v.push_back(name);
  }
  for (size_t irecord = 0; irecord < nrecords; irecord++) {
    DBRecord record;
    for (size_t ifield = 0; ifield < nfields; ifield++) {
      const char* value = PQgetvalue(_sq_result, irecord, ifield);
      if (value != NULL) {
        record.addField(name_v[ifield], value);
      }
    }
    _record_v.push_back(record);
  }
  return _record_v;
}

void PostgreSQLInterface::clear() throw()
{
  if (_sq_result != NULL) {
    PQclear(_sq_result);
  }
  _sq_result = NULL;
}

void PostgreSQLInterface::close() throw(DBHandlerException)
{
  clear();
  PQfinish(_sq_conn);
}


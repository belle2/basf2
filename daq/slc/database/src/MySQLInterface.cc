/*
#include "daq/slc/database/MySQLInterface.h"

#include "daq/slc/base/StringUtil.h"

#include <iostream>

using namespace Belle2;

MySQLInterface::MySQLInterface(const std::string& host,
             const std::string& database,
             const std::string& user,
             const std::string& password,
             int port) throw()
  : DBInterface(host, database, user, password, port),
    _sq_conn(NULL), _sq_result(NULL) {}

void MySQLInterface::connect() throw(DBHandlerException)
{
  if ((_sq_conn = mysql_init((MYSQL*) NULL)) == NULL) {
    throw (DBHandlerException(__FILE__, __LINE__, "failed to init"));
  }
  if (!mysql_real_connect(_sq_conn, _host.c_str(),
                          _user.c_str(), _password.c_str(),
                          _database.c_str(), _port, NULL, 0)) {
    throw (DBHandlerException(__FILE__, __LINE__,
                              Belle2::form("Connection error:%s", mysql_error(_sq_conn))));
  }
}

void MySQLInterface::execute(const std::string& command)
throw(DBHandlerException)
{
  clear();
  if (mysql_query(_sq_conn, command.c_str())) {
    throw (DBHandlerException(__FILE__, __LINE__,
                              Belle2::form("Failed to execute command : %s %s",
                                           command.c_str(), mysql_error(_sq_conn))));
  }
}

DBRecordList& MySQLInterface::loadRecords() throw(DBHandlerException)
{
  if ((_sq_result = mysql_store_result(_sq_conn))) {
    const size_t nrecords = mysql_num_rows(_sq_result);
    const size_t nfields = mysql_num_fields(_sq_result);
    MYSQL_FIELD* fields = mysql_fetch_fields(_sq_result);;
    _record_v = DBRecordList();
    std::vector<std::string> name_v;
    for (size_t ifield = 0; ifield < nfields; ifield++) {
      const char* name = fields[ifield].name;
      if (name != NULL) name_v.push_back(name);
    }
    for (size_t irecord = 0; irecord < nrecords; irecord++) {
      DBRecord record;
      MYSQL_ROW rows = mysql_fetch_row(_sq_result);
      for (size_t ifield = 0; ifield < nfields; ifield++) {
        const char* value = rows[ifield];
        if (value != NULL) {
          record.addField(name_v[ifield], value);
        }
      }
      _record_v.push_back(record);
    }
    mysql_free_result(_sq_result);
    _sq_result = NULL;
    return _record_v;
  } else {
    throw (DBHandlerException(__FILE__, __LINE__, "Failed to get records"));
  }
}

void MySQLInterface::clear() throw()
{
  if (_sq_result != NULL) {
    mysql_free_result(_sq_result);
  }
  _sq_result = NULL;
}

void MySQLInterface::close() throw(DBHandlerException)
{
  clear();
  mysql_close(_sq_conn);
}
*/

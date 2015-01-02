#include "framework/xmldb/connection.h"

#include <framework/logging/Logger.h>
#include <string.h>

#include "RConfigure.h"

namespace Belle2 {
  namespace xmldb {

    Connection::Connection(const std::string& _connection_string)
    {
      conn_ = ::PQconnectdb(_connection_string.c_str());

      // Do not try to use the connection, when it failed...
      if (!ok()) return;

      integer_datetime_ = false;
      const char* int_time = ::PQparameterStatus(conn_, "integer_datetimes");
      if (int_time) {
        if (!strcmp(int_time, "on")) {
          integer_datetime_ = true;
        }  // if
      }  // if

      dumpConnectionParameters();
    }  // Connection constructor


    Connection::~Connection()
    {
      ::PQfinish(conn_);
    }  // Connection constructor


    void Connection::dumpConnectionParameters() const
    {
      if (ok()) {
        const int client_encoding = ::PQclientEncoding(conn_);
        const char* encoding_str = ::pg_encoding_to_char(client_encoding);

        B2INFO("Connected to '" << ::PQdb(conn_) << "' @ '" << ::PQhost(conn_)
               << "', "
#ifdef HAVE_PQLIBVERSION
               << "PQlibVersion " << std::dec << ::PQlibVersion() << ", ";
#endif
               << "client encoding '" << encoding_str
               << "', server version " << ::PQparameterStatus(conn_, "server_version")
               << ", server encoding '" << ::PQparameterStatus(conn_, "server_encoding")
               << "'");
      } else {
        B2WARNING("Not connected.");
      }
    }  // Connection::dumpConnectionParameters

    bool Connection::ok() const
    {
      return ::PQstatus(conn_) == ::CONNECTION_OK;
    }  // Connection::ok

  }  // namespace xmldb
}  // namespace Belle2


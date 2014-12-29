#pragma once

#include <libpq-fe.h>

#include <string>

#include "framework/xmldb/macros.h"

namespace Belle2 {
  namespace xmldb {

    /** Connection to a PostgreSQL database. */
    class Connection {
    public:
      /** Initialize the connection to the database.
       * @param _connection_string PostgreSQL connection string.
       */
      explicit Connection(const std::string& _connection_string);
      ~Connection();

      /** %Query the server for the state of the compile-time
       * integer_datetimes option.
       */
      bool integer_datetime() const MUST_USE_RESULT { return integer_datetime_; }

      /** Check the state of the database connection.
       * @return true, iff the connection is alive.
       */
      bool ok() const MUST_USE_RESULT;

    private:
      friend class Query;

      /** Get the handle to the database connection.
       *  @return The handle.
       */
      ::PGconn* conn() const MUST_USE_RESULT { return conn_; }
      /** Print information about the active database connection. */
      void dumpConnectionParameters() const;

      /** The PostgreSQL handle to the database connection. */
      ::PGconn* conn_;
      /** The server setting of the integer_datatime option. */
      bool integer_datetime_;
    };  // class Connection

  }  // namespace xmldb
}  // namespace Belle2


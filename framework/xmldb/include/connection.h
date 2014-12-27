#pragma once

#include <libpq-fe.h>

#include <string>

#include "framework/xmldb/macros.h"

namespace Belle2 {
  namespace xmldb {

//! Connection to a PostgreSQL database.
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
       * \return true, iff the connection is alive.
       */
      bool ok() const MUST_USE_RESULT;

    private:
      friend class Query;

      ::PGconn* conn() const MUST_USE_RESULT { return conn_; }
      void dumpConnectionParameters() const;

      ::PGconn* conn_;
      bool integer_datetime_;
    };  // class Connection

  }  // namespace xmldb
}  // namespace Belle2


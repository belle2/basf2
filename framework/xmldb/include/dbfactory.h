#pragma once

#include <cstdint>
#include <memory>

#include "framework/xmldb/macros.h"

namespace Belle2 {
  namespace xmldb {
    class Connection;

// this makes the situation with the database configuration a bit better...
    /** Helper class to obtain a connection to the database server. */
    class DBFactory {
    public:
      /** Obtain a valid connection to the database.
       *  This might trigger a new connection attempt, or re-use an existing
       *  connection. */
      static std::shared_ptr<Belle2::xmldb::Connection> connect() MUST_USE_RESULT;

    private:
      /** Disabled. */
      DBFactory();

      /** Verify that the version of the database scheme is as expected. */
      static bool verifySchemeVersion();

      /** A pointer to an active connection. */
      static std::shared_ptr<Belle2::xmldb::Connection> connection_;
    };
  }
}


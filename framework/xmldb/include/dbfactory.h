#pragma once

#include <cstdint>
#include <memory>

#include "framework/xmldb/macros.h"

namespace Belle2 {
  namespace xmldb {
    class Connection;

// this makes the situation with the database configuration a bit better...
    class DBFactory {
    public:
      static std::shared_ptr<Belle2::xmldb::Connection> connect() MUST_USE_RESULT;

    private:
      DBFactory();
      static bool verifySchemeVersion();

      static std::shared_ptr<Belle2::xmldb::Connection> connection_;
    };
  }
}


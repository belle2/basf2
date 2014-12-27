#include "framework/xmldb/dbfactory.h"
#include "framework/xmldb/LOCAL_CONFIG.h"
#include "framework/xmldb/connection.h"
#include "framework/xmldb/query.h"
#include "framework/xmldb/query_builder.h"

#include <framework/logging/Logger.h>
#include <iostream>

namespace Belle2 {
  namespace xmldb {
    std::shared_ptr<Belle2::xmldb::Connection> DBFactory::connection_ { nullptr };

    DBFactory::DBFactory()
    {
    }

    std::shared_ptr<Belle2::xmldb::Connection> DBFactory::connect()
    {
      if (!connection_ || !connection_->ok()) {
        connection_ = std::make_shared<Belle2::xmldb::Connection>(connectionString);
        if (!verifySchemeVersion()) {
          connection_.reset();
        };
      }
      return connection_;
    }

    bool DBFactory::verifySchemeVersion()
    {
      Belle2::xmldb::QueryBuilder g;
      g.setStatement("SELECT MAX(version) AS scheme_version FROM scheme_t",
                     0 /* # parameters */);
      Belle2::xmldb::Query q(connection_.get());
      if (!q.executeGenerated(g)) {
        std::cout << "query failed" << std::endl;
        return false;
      } // if
      if (q.numRows() != 1) {
        return false;
      } // if
      const int version_index = q.columnIndex("scheme_version");
      int32_t actual_version = q.getValueAsInt(0, version_index);
      if (actual_version != schemeVersion) {
        B2ERROR("wrong database scheme version. required: "
                << schemeVersion << ", in DB: " << actual_version);
        return false;
      }
      return true;
    }
  }
}


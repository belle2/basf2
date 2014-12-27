#pragma once

#include <limits>
#include <map>
#include <string>
#include <vector>

#include "framework/xmldb/macros.h"
#include "framework/xmldb/tree.h"


namespace Belle2 {
  namespace xmldb {
    class Connection;
  } // namespace xmldb

  namespace xmldb {

    class DB {
    public:
      // maps from name to id.
      // this is redundant by now, as File=name + id, but this might change
      // in the future.
      typedef std::map<std::string, Tree::File> FileList;
      typedef std::multimap<std::string, Tree::Commit> CommitsToFile;

      /** List files known to the database.

          @param _like String to match. In SQL syntax, i.e. "%like so%".
      */
      static FileList listFiles(Belle2::xmldb::Connection* _db,
                                const std::string& _like = "%")
      MUST_USE_RESULT;

      /** List commits for a given file.

          @param _file The file to examine.
          @param _like String to match. In SQL syntax, i.e. "%like so%".
          @param _from Lower end of creation time range.
          @param _to Upper end of creation time range.
      */
      static CommitsToFile find(Belle2::xmldb::Connection* _db, int32_t _file,
                                const std::string& _like = "%", long _from = 0,
                                long _to = std::numeric_limits<long>::max() / 1000000UL)
      MUST_USE_RESULT;
    }; // class DB

  } // namespace xmldb
} // namespace Belle2


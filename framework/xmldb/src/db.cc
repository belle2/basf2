#include "framework/xmldb/db.h"
#include "framework/xmldb/query.h"
#include "framework/xmldb/query_builder.h"

#include <stdexcept>

namespace Belle2 {
  namespace xmldb {

    DB::FileList DB::listFiles(Belle2::xmldb::Connection* _db,
                               const std::string& _like)
    {
      Belle2::xmldb::QueryBuilder g;
      g.setStatement("SELECT name, id "
                     "FROM file_t "
                     "WHERE name LIKE $1",
                     1 /* # parameters */);
      g.setParameterString(1, _like);
      Belle2::xmldb::Query q(_db);
      if (!q.executeGenerated(g)) {
        throw std::runtime_error("Query failed.");
      }  // if

      FileList dir;
      const int id_index = q.columnIndex("id");
      const int name_index = q.columnIndex("name");

      for (int row = 0; row < q.numRows(); ++row) {
        Belle2::xmldb::Tree::File f;
        f.id = q.getValueAsInt(row, id_index);
        f.name = q.getValueAsString(row, name_index);
        dir.insert(std::make_pair(f.name, f));
      } // for row

      return dir;

    }

    DB::CommitsToFile DB::find(Belle2::xmldb::Connection* _db, int32_t _file,
                               const std::string& _like, long _from, long _to)
    {
      Belle2::xmldb::QueryBuilder g;
      g.setStatement("SELECT c.comment, c.datetime, c.id as commitid, "
                     "b.name, b.id as branchid "
                     "FROM commit_t c "
                     "JOIN branch_t b ON c.branchid = b.id "
                     "WHERE b.name LIKE $1 AND c.datetime BETWEEN $2 AND $3 "
                     "AND b.fileid=$4 "
                     "ORDER BY c.datetime DESC",
                     4 /* # parameters */);
      g.setParameterString(1, _like);
      g.setParameterTimestamp(2, _from);
      g.setParameterTimestamp(3, _to);
      g.setParameterInt(4, _file);
      Belle2::xmldb::Query q(_db);
      if (!q.executeGenerated(g)) {
        throw std::runtime_error("Query failed.");
      }  // if

      CommitsToFile dir;
      const int comment_index = q.columnIndex("comment");
      const int created_index = q.columnIndex("datetime");
      const int id_index = q.columnIndex("commitid");
      const int name_index = q.columnIndex("name");
      const int bid_index = q.columnIndex("branchid");

      for (int row = 0; row < q.numRows(); ++row) {
        Belle2::xmldb::Tree::Commit id;
        id.id = q.getValueAsInt(row, id_index);
        id.comment = q.getValueAsString(row, comment_index);
        id.creation_date = q.getValueAsTimestamp(row, created_index);
        id.branch.name = q.getValueAsString(row, name_index);
        id.branch.id = q.getValueAsInt(row, bid_index);
        dir.insert(std::make_pair(id.branch.name, id));
      } // for row

      return dir;
    } // DB::find

  } // namespace xmldb
} // namespace Belle2


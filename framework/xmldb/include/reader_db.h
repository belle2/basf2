#pragma once

#include "framework/xmldb/macros.h"
#include "framework/xmldb/leaf.h"
#include "framework/xmldb/leaf_int.h"
#include "framework/xmldb/types.h"

#include <map>
#include <string>

namespace Belle2 {
  namespace xmldb {
    class Connection;
    class Query;
  } // namespace xmldb

  namespace xmldb {

    class Node;
    class Tree;

    class Reader_DB {
    public:
      Reader_DB();

      // format "file;branch". branch defaults to trunk.
      Tree* read(const std::string& _file) MUST_USE_RESULT;
      Tree* read(int64_t _commitid) MUST_USE_RESULT;

      std::vector<uint8_t> readCdata(int32_t _id,
                                     int64_t _size) MUST_USE_RESULT;

    private:
      int attrib_name_index_;
      int attrib_value_index_;
      int cdata_id_index_;
      int cdata_size_index_;
      int commitid_index_;
      int float_value_index_;
      int id_index_;
      int int_value_index_;
      int linktype_index_;
      int name_index_;
      int parent_index_;
      int text_value_index_;
      Tree* work_tree_;

      typedef std::map<int64_t, ::xmlNodePtr> id_to_node_map_t;
      id_to_node_map_t id_to_node_map_;

      Belle2::xmldb::Query* queryTree(int _rootnodeid) MUST_USE_RESULT;
      ::xmlNodePtr buildNode(Belle2::xmldb::Query* _q, int64_t _row);
    }; // class Reader_DB
  } // namespace xmldb
} // namespace Belle2


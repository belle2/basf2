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

    /** Implements loading of XML data from a SQL database. */
    class Reader_DB {
    public:
      Reader_DB();

      /** Load the latest revision of a file from the database.
       *  @param _file Name of the file to load. Format "file;branch".
       *  branch defaults to trunk.
       *  @return The tree representing the XML data.
       */
      Tree* read(const std::string& _file) MUST_USE_RESULT;
      /** Load a given commit from the database.
       *  @param _commitid The id of the commit to load.
       *  @return The tree representing the XML data.
       */
      Tree* read(int64_t _commitid) MUST_USE_RESULT;

      /** Load a binary data block from the database.
       *  @param _id The id of the binary block.
       *  @param _size The expected raw size of the data block.
       *  @return The data retrieved from the database.
       */
      std::vector<uint8_t> readCdata(int32_t _id,
                                     int64_t _size) MUST_USE_RESULT;

    private:
      int attrib_name_index_; /**< Column index of the attrib_name column. */
      int attrib_value_index_; /**< Column index of the attrib_value column. */
      int cdata_id_index_; /**< Column index of the cdata_id column. */
      int cdata_size_index_; /**< Column index of the cdata_size column. */
      int commitid_index_; /**< Column index of the commitid column. */
      int float_value_index_; /**< Column index of the float_value column. */
      int id_index_; /**< Column index of the id column. */
      int int_value_index_; /**< Column index of the int_value column. */
      int linktype_index_; /**< Column index of the linktype column. */
      int name_index_; /**< Column index of the name column. */
      int parent_index_; /**< Column index of the parent column. */
      int text_value_index_; /**< Column index of the text_value column. */
      /** The tree we are building. */
      Tree* work_tree_;

      typedef std::map<int64_t, ::xmlNodePtr> id_to_node_map_t;
      id_to_node_map_t id_to_node_map_;

      /** Retrieve a document from the database. */
      Belle2::xmldb::Query* queryTree(int _rootnodeid) MUST_USE_RESULT;
      /** Handle a single row of the SQL query result. */
      ::xmlNodePtr buildNode(Belle2::xmldb::Query* _q, int64_t _row);
    }; // class Reader_DB
  } // namespace xmldb
} // namespace Belle2


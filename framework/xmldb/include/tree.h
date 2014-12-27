#pragma once

#include <map>
#include <string>
#include <vector>

#include <libxml/tree.h>
#include <sys/time.h>

#include "framework/xmldb/macros.h"

namespace Belle2 {
  namespace xmldb {

    class Node;

    class Tree {
    public:

      struct File {
        int32_t id;
        std::string name;
      };

      struct Branch {
        int64_t id;
        std::string name;
        File file;
      };

      struct Commit {
        std::string comment;
        timeval creation_date;
        int64_t id;
        int64_t predecessor;
        Branch branch;
      };

      typedef std::vector<Commit> commit_list_t;

      /** Constructs an empty tree. */
      Tree();
      ~Tree();

      /*! Find a node by an XPath query.
       *
       * The query must return exactly one node.
       *
       * @param _xpath The XPath query string.
       * @return A pointer to the node, or nullptr in case of error.
       */
      ::xmlNodePtr findNode(const std::string& _xpath) const MUST_USE_RESULT;

      /** Attach meta information to a libxml node. */
      void attachNodeData(::xmlNodePtr _node, Node* _data);
      /** Retrieve meta information for a libxml node. */
      Node* getNodeData(::xmlNodePtr _node) const;

      ::xmlNodePtr rootNode() const MUST_USE_RESULT;

      /*! Retrieve a CDATA block from the database.
       * After the successful completion of the operation, the data
       * in the node is valid. The call is a NOP, if the data had already
       * been loaded.
       * @param _xpath An XPath query string describing which node to load.
       * @return true, if the data could be loaded, false otherwise.
       */
      bool loadCdata(const std::string& _xpath);

      Commit id_;
      ::xmlDocPtr doc_;
      ::xmlNsPtr configNs_;

      typedef std::map< ::xmlNodePtr, Node* > node_data_map_t;
      /** Maps from the libxml node pointer to our additional metadata
       * for the node. */
      node_data_map_t node_data_map;

      static bool isLeaf(::xmlNodePtr n) MUST_USE_RESULT;
      static bool isTreeChild(::xmlNodePtr n) MUST_USE_RESULT;
      // ignore XML comments, and pure text nodes
      static bool isTrueChild(::xmlNodePtr n) MUST_USE_RESULT;
    }; // class Tree

  } // namespace xmldb
} // namespace Belle2


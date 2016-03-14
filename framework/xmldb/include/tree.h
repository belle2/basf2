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

    /** Represents an XML file including its metadata. */
    class Tree {
    public:

      /** Metadata of a file stored in the database. */
      struct File {
        int32_t id; /**< Database id. */
        std::string name; /**< File name. */
      };

      /** Metadata of a branch stored in the database. */
      struct Branch {
        int64_t id; /**< Database id. */
        std::string name; /**< Branch name. */
        File file; /**< File this branch belongs to. */
      };

      /** Metadata of a commit to the database. */
      struct Commit {
        std::string comment; /**< Free-form commit message. */
        timeval creation_date; /**< Time the entry was saved. */
        int64_t id; /**< Database id. */
        int64_t predecessor; /**< Id of the commit's predecessor. */
        Branch branch; /**< Branch the commit belongs to. */
      };

      /** A List of commits. */
      typedef std::vector<Commit> commit_list_t;

      /** Constructs an empty tree. */
      Tree();
      /** Destructor. */
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

      /** Get the root node of the libxml2 tree.
       *  @return A pointer to the root node.
       */
      ::xmlNodePtr rootNode() const MUST_USE_RESULT;

      /*! Retrieve a CDATA block from the database.
       * After the successful completion of the operation, the data
       * in the node is valid. The call is a NOP, if the data had already
       * been loaded.
       * @param _xpath An XPath query string describing which node to load.
       * @return true, if the data could be loaded, false otherwise.
       */
      bool loadCdata(const std::string& _xpath);

      /** Information about the commit that has been loaded. */
      Commit id_;
      /** Pointer to the associated libxml2 document. */
      ::xmlDocPtr doc_;
      /** Libxml2 handle for the config namespace definition. */
      ::xmlNsPtr configNs_;

      /** node pointer -> metadata. */
      typedef std::map< ::xmlNodePtr, Node* > node_data_map_t;
      /** Maps from the libxml node pointer to our additional metadata
       * for the node. */
      node_data_map_t node_data_map;

      /** Is the node a leaf containing data? */
      static bool isLeaf(::xmlNodePtr n) MUST_USE_RESULT;
      /** Is this an inner node of the document?
       *  I.e. not a leaf, comment or text node.
       */
      static bool isTreeChild(::xmlNodePtr n) MUST_USE_RESULT;
      /** Does this node contribute to the content of the document?
       *  Ignores XML comments and pure text nodes.
       */
      static bool isTrueChild(::xmlNodePtr n) MUST_USE_RESULT;
    }; // class Tree

  } // namespace xmldb
} // namespace Belle2


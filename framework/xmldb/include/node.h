#pragma once

#include <cstdint>
#include <libxml/tree.h>
#include <stdexcept>
#include <string>
#include <sys/time.h>

#include "framework/xmldb/macros.h"
#include "framework/xmldb/types.h"

namespace Belle2 {
  namespace xmldb {
    namespace gui {
      class QConfigModel;
    }

    class Tree;

    /** Annotations for a libxml2 node. */
    class Node {
    public:
      explicit Node(::xmlNodePtr _xmlnode, const Tree* _owner);
      virtual ~Node();

      /** @return A text representation of the attributes of the XML node. */
      std::string attributes() const MUST_USE_RESULT;

      /** @return True, iff the node has been changed since being loaded from
       *  the database.
       */
      bool isDirty() const MUST_USE_RESULT;

      /** @return True, iff the node has a predecessor in the database. */
      inline bool hasPredecessor() const MUST_USE_RESULT {
        return predecessor_ != -1;
      }

      /** @return The id of the node in the database. */
      inline int64_t id() const MUST_USE_RESULT {
        return id_;
      } // id

      /** @return True, iff the node is a leaf, i.e. contains data. */
      bool isLeaf() const MUST_USE_RESULT;

      /** @return The name of the node. */
      std::string name() const MUST_USE_RESULT;
      /** @return The full path of the node (including the name), separated
       * with ".".
       */
      std::string path() const MUST_USE_RESULT;

      /** The relationship to the parent node as listed in the database. */
      enum class LinkType : int {
        parent = 1, /**< The parent node is on a higher level in the tree. */
        sibling = 2 /**< The parent node is a sibling. */
      };

      /** Full specification of the database parent. */
      struct LinkParent {
        Node* node; /**< Pointer to the node. */
        LinkType type; /**< Type of the relationship. */
      };

      /** Get the database parent of the node. */
      LinkParent DBParent() const MUST_USE_RESULT;
      /** Get the tree parent (always the node on the next higher level. */
      Node* parent() const MUST_USE_RESULT;

      /** Get the database id of the node's predecessor, or -1. */
      inline int64_t predecessor() const MUST_USE_RESULT {
        return predecessor_;
      }

      /** Pointer to the corresponding node in the libxml2 tree. */
      const ::xmlNodePtr xmlnode_;

    protected:
      /** Mark this node as dirty due to a change in its data. */
      void invalidateData();
      /** Mark this node as dirty, because it is on the path between a dirty
       *  node and the tree root node.
       */
      void invalidateLink();

    private:
      friend class Reader_DB;
      friend class Reader_XML;
      friend class Belle2::xmldb::gui::QConfigModel;

      /** The id of this node in the database.
       *  -1 : node has not been stored to the database, yet ("dirty").
       *
       *  Always -1 for nodes taken from XML files.
       */
      int64_t id_;
      /** True, iff the node needs to be re-written to the database because
       *  it lies in the path from a changed node to the root node.
       */
      bool link_invalid;

      /** The tree to which this node belongs. */
      const Tree* owner_;
      /** The id of the predecessor in the database, or -1. */
      int64_t predecessor_;

      /** The íd of the last commit that changed the value of this node. */
      int64_t last_changed_commit_;
    };  // class Node

  }  // namespace xmldb
}  // namespace Belle2


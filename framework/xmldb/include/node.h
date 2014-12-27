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

    class Node {
    public:
      explicit Node(::xmlNodePtr _xmlnode, const Tree* _owner);
      virtual ~Node();

      std::string attributes() const MUST_USE_RESULT;

      bool isDirty() const MUST_USE_RESULT;

      inline int64_t numChildren() const MUST_USE_RESULT {
        return num_children_;
      } // Children

      inline bool hasPredecessor() const MUST_USE_RESULT {
        return predecessor_ != -1;
      }

      inline int64_t id() const MUST_USE_RESULT {
        return id_;
      } // id

      bool isLeaf() const MUST_USE_RESULT;

      std::string name() const MUST_USE_RESULT;
      std::string path() const MUST_USE_RESULT;

      enum class LinkType : int {
        parent = 1,
        sibling = 2
      };

      struct LinkParent {
        Node* node;
        LinkType type;
      };

      LinkParent DBParent() const MUST_USE_RESULT;
      Node* parent() const MUST_USE_RESULT;

      inline int64_t predecessor() const MUST_USE_RESULT {
        return predecessor_;
      }

      const ::xmlNodePtr xmlnode_;

    protected:
      void invalidateData();
      void invalidateLink();

    private:
      friend class Reader_DB;
      friend class Reader_XML;
      friend class Belle2::xmldb::gui::QConfigModel;

      //! The id of this node in the database.
      /*! -1 : node has not been stored to the database, yet ("dirty").

          Always -1 for nodes taken from XML files.
      */
      int64_t id_;
      bool link_invalid;

      //! Incremented by the Reader
      int64_t num_children_;

      const Tree* owner_;
      int64_t predecessor_;

      int64_t last_changed_commit_;
    };  // class Node

  }  // namespace xmldb
}  // namespace Belle2


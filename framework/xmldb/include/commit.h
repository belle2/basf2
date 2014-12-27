#pragma once

#include "framework/xmldb/macros.h"

#include "framework/xmldb/leaf.h"
#include "framework/xmldb/types.h"

#include <string>
#include <vector>

namespace Belle2 {
  namespace xmldb {
    class Connection;
  } // namespace xmldb

  namespace xmldb {
    /** Meta-information about one revision in the XML database. */
    struct Commit {
      std::string comment_;
      int64_t id_;
      int64_t rootnodeid_;
      timeval time_;
    }; // struct Commit

    /** Meta-information about one commit to a node. */
    struct CommitToNode {
      Commit commit_;
      int64_t id_;
      Leaf::DataType type_;
      ConfigDouble float_value_;
      ConfigInt int_value_;
      std::string text_value_;
    }; // struct NodeHistory

    /** Revision history of one node in the XML database. */
    typedef std::vector<CommitToNode> NodeHistory;
  } // namespace xmldb
} // namespace Belle2


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
      std::string comment_; /**< The commit text given by the author. */
      int64_t id_; /**< The id of the commit. */
      int64_t rootnodeid_; /**< The id of the root node. */
      timeval time_; /**< The timestamp of the commit. */
    }; // struct Commit

    /** Meta-information about one commit to a node. */
    struct CommitToNode {
      Commit commit_; /**< Information about the commit. */
      int64_t id_; /**< The id of the predecessor node. */
      Leaf::DataType type_; /**< The type of the predecessor node. */
      ConfigDouble float_value_; /**< Floating point value of the old node. */
      ConfigInt int_value_; /**< Integer value of the old node. */
      std::string text_value_; /**< String value of the old node. */
    }; // struct NodeHistory

    /** Revision history of one node in the XML database. */
    typedef std::vector<CommitToNode> NodeHistory;
  } // namespace xmldb
} // namespace Belle2


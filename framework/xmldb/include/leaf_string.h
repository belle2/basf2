#pragma once

#include "framework/xmldb/macros.h"
#include "framework/xmldb/leaf.h"

#include <string>

namespace Belle2 {
  namespace xmldb {

    /** A leaf node containing an text value. */
    class LeafString : public Leaf {
    public:
      explicit LeafString(::xmlNodePtr _xmlnode, const Tree* _owner);

      /** Returns the data type, String.
       *  @return Leaf::String.
       */
      virtual DataType type() const;

      /** @return The the datum stored in the node. */
      std::string value() const MUST_USE_RESULT;

      /** Update the value stored in the node.
       *  @param _v the new value.
       */
      void setValue(const std::string& _v, bool _invalidate = true);

      /** @return A textual representation of the data. */
      virtual std::string textRepresentation();
    };  // class Node

  }  // namespace xmldb
}  // namespace Belle2


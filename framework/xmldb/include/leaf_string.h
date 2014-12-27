#pragma once

#include "framework/xmldb/macros.h"
#include "framework/xmldb/leaf.h"

#include <string>

namespace Belle2 {
  namespace xmldb {

    class LeafString : public Leaf {
    public:
      explicit LeafString(::xmlNodePtr _xmlnode, const Tree* _owner);

      virtual DataType type() const;

      std::string value() const MUST_USE_RESULT;

      void setValue(const std::string& _v, bool _invalidate = true);

      virtual std::string textRepresentation();
    };  // class Node

  }  // namespace xmldb
}  // namespace Belle2


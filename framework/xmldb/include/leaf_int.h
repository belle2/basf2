#pragma once

#include "framework/xmldb/macros.h"
#include "framework/xmldb/leaf.h"

namespace Belle2 {
  namespace xmldb {

    class LeafInt : public Leaf {
    public:
      LeafInt(::xmlNodePtr _xmlnode, const Tree* _owner, ConfigInt _value);

      virtual DataType type() const;

      inline ConfigInt value() const MUST_USE_RESULT {
        return value_;
      };

      void setValue(ConfigInt _v);

      virtual std::string textRepresentation();

    private:
      ConfigInt value_;
    };  // class LeafInt

  }  // namespace xmldb
}  // namespace Belle2


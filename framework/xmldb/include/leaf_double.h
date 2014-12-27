#pragma once

#include "framework/xmldb/macros.h"
#include "framework/xmldb/leaf.h"

namespace Belle2 {
  namespace xmldb {
    class LeafDouble : public Leaf {
    public:
      LeafDouble(::xmlNodePtr _xmlnode, const Tree* _owner,
                 ConfigDouble _value);

      virtual DataType type() const;

      inline ConfigDouble value() const MUST_USE_RESULT {
        return value_;
      };

      void setValue(ConfigDouble _v);

      virtual std::string textRepresentation();

    private:
      ConfigDouble value_;
    };  // class LeafDouble

  }  // namespace xmldb
}  // namespace Belle2


#pragma once

#include "framework/xmldb/macros.h"
#include "framework/xmldb/leaf.h"

namespace Belle2 {
  namespace xmldb {

    /** A leaf node containing an integer datum. */
    class LeafInt : public Leaf {
    public:
      LeafInt(::xmlNodePtr _xmlnode, const Tree* _owner, ConfigInt _value);

      /** Returns the data type, Int.
       *  @return Leaf::Int.
       */
      virtual DataType type() const;

      /** @return The the datum stored in the node. */
      inline ConfigInt value() const MUST_USE_RESULT {
        return value_;
      };

      /** Update the value stored in the node.
       *  @param _v the new value.
       */
      void setValue(ConfigInt _v);

      /** @return A textual representation of the data. */
      virtual std::string textRepresentation();

    private:
      /** The raw value. */
      ConfigInt value_;
    };  // class LeafInt

  }  // namespace xmldb
}  // namespace Belle2


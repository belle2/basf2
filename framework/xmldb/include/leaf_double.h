#pragma once

#include "framework/xmldb/macros.h"
#include "framework/xmldb/leaf.h"

namespace Belle2 {
  namespace xmldb {
    /** A leaf node containing a floating point datum. */
    class LeafDouble : public Leaf {
    public:
      LeafDouble(::xmlNodePtr _xmlnode, const Tree* _owner,
                 ConfigDouble _value);

      /** Returns the data type, Double.
       *  @return Leaf::Double.
       */
      virtual DataType type() const;

      /** @return The the datum stored in the node. */
      inline ConfigDouble value() const MUST_USE_RESULT {
        return value_;
      };

      /** Update the value stored in the node.
       *  @param _v the new value.
       */
      void setValue(ConfigDouble _v);

      /** @return A textual representation of the data. */
      virtual std::string textRepresentation();

    private:
      /** The raw value. */
      ConfigDouble value_;
    };  // class LeafDouble

  }  // namespace xmldb
}  // namespace Belle2


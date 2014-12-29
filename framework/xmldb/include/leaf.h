#pragma once

#include "framework/xmldb/node.h"

#include <string>

namespace Belle2 {
  namespace xmldb {

    /** A node in the XML tree that contains actual data. */
    class Leaf : public Node {
    public:
      /** The type of the data stored in a leaf. */
      enum class DataType {
        None, //< Not a leaf.
        Int, String, Double, Cdata
      };

      explicit Leaf(::xmlNodePtr _xmlnode, const Tree* _owner);

      virtual DataType type() const = 0;

      virtual std::string textRepresentation() = 0;

      /** Updates the text representation stored in the XML node when the raw
       *  data changes.
       */
      virtual bool updateXMLNode();
    };  // class Leaf

  }  // namespace xmldb
}  // namespace Belle2


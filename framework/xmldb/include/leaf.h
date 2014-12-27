#pragma once

#include "framework/xmldb/node.h"

#include <string>

namespace Belle2 {
  namespace xmldb {

    class Leaf : public Node {
    public:
      // None: not a leaf
      enum class DataType {
        None, Int, String, Double, Cdata
      };

      explicit Leaf(::xmlNodePtr _xmlnode, const Tree* _owner);

      virtual DataType type() const = 0;

      virtual std::string textRepresentation() = 0;

      virtual bool updateXMLNode();
    };  // class Leaf

  }  // namespace xmldb
}  // namespace Belle2


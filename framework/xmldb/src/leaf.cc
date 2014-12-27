#include "framework/xmldb/leaf.h"

namespace Belle2 {
  namespace xmldb {

    Leaf::Leaf(::xmlNodePtr _xmlnode, const Tree* _owner)
      : Node(_xmlnode, _owner)
    {
    } // Leaf constructor

    bool Leaf::updateXMLNode()
    {
      ::xmlNodeSetContent(xmlnode_,
                          reinterpret_cast<const ::xmlChar*>(textRepresentation().c_str()));
      // TODO: check return code
      return true;
    } // Leaf::updateXMLNode
  } // namespace xmldb
} // namespace Belle2


#include "framework/xmldb/leaf_string.h"

namespace Belle2 {
  namespace xmldb {

    LeafString::LeafString(::xmlNodePtr _xmlnode, const Tree* _owner)
      : Leaf(_xmlnode, _owner)
    {
    } // LeafString configuration


    void LeafString::setValue(const std::string& _v, bool _invalidate)
    {
      ::xmlNodeSetContent(xmlnode_, reinterpret_cast<const ::xmlChar*>(_v.c_str()));
      if (_invalidate) invalidateData();
    } // LeafString::setValue


    std::string LeafString::textRepresentation()
    {
      return value();
    } // LeafString::textRepresentation


    Leaf::DataType LeafString::type() const
    {
      return DataType::String;
    } // LeafString::type


    std::string LeafString::value() const
    {
      ::xmlChar* const v = ::xmlNodeGetContent(xmlnode_);
      std::string ret(v ? reinterpret_cast<const char*>(v) : "<NULL>");
      ::xmlFree(v);
      return ret;
    }
  } // namespace xmldb
} // namespace Belle2


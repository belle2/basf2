#include "framework/xmldb/leaf_int.h"

#include <sstream>

namespace Belle2 {
  namespace xmldb {

    LeafInt::LeafInt(::xmlNodePtr _xmlnode, const Tree* _owner,
                     ConfigInt _value)
      : Leaf(_xmlnode, _owner), value_(_value)
    {
    } // LeafInt configuration

    void LeafInt::setValue(ConfigInt _v)
    {
      value_ = _v;
      updateXMLNode();
      invalidateData();
    } // LeafInt::setValue

    std::string LeafInt::textRepresentation()
    {
      std::stringstream s;
      s << value_;
      return s.str();
    } // LeafInt::textRepresentation

    Leaf::DataType LeafInt::type() const
    {
      return DataType::Int;
    } // LeafInt::type
  } // namespace xmldb
} // namespace Belle2


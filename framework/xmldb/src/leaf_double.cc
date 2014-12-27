#include "framework/xmldb/leaf_double.h"

#include <sstream>

namespace Belle2 {
  namespace xmldb {

    LeafDouble::LeafDouble(::xmlNodePtr _xmlnode, const Tree* _owner,
                           ConfigDouble _value)
      : Leaf(_xmlnode, _owner), value_(_value)
    {
    } // LeafDouble configuration


    void LeafDouble::setValue(ConfigDouble _v)
    {
      value_ = _v;
      updateXMLNode();
      invalidateData();
    } // LeafDouble::setValue


    std::string LeafDouble::textRepresentation()
    {
      std::stringstream s;
      s << value_;
      return s.str();
    } // LeafDouble::textRepresentation


    Leaf::DataType LeafDouble::type() const
    {
      return DataType::Double;
    } // LeafDouble::type

  } // namespace xmldb
} // namespace Belle2


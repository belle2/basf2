#include <framework/utilities/Stream.h>

#include <TObject.h>
#include <TBufferXML.h>

#include <boost/algorithm/string/replace.hpp>

using namespace Belle2;

std::string Stream::serialize(const TObject* obj)
{
  //TODO: root 5.34/8 should fix that
  TObject* nonConstObj = const_cast<TObject*>(obj);
  const TString& xmlString = TBufferXML::ConvertToXML(nonConstObj);
  return std::string(xmlString.Data());
}

std::string Stream::escapeXML(const std::string& xmlString)
{
  //avoid nesting CDATA sections...
  std::string newString(xmlString);
  boost::replace_all(newString, "]]>", "]]]]><![CDATA[>");
  return "<![CDATA[" + newString + "]]>";
}

TObject* Stream::deserialize(const std::string& data)
{
  return TBufferXML::ConvertFromXML(data.c_str());
}

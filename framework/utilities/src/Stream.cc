#include <framework/utilities/Stream.h>

#include <TObject.h>
#include <TBufferXML.h>

using namespace Belle2;

std::string Stream::serialize(const TObject* obj)
{
  //TODO: root 5.34/8 should fix that
  TObject* nonConstObj = const_cast<TObject*>(obj);
  const TString& xmlString = TBufferXML::ConvertToXML(nonConstObj);
  return std::string(xmlString.Data());
}

boost::shared_ptr<TObject> Stream::deserialize(const std::string& data)
{
  return boost::shared_ptr<TObject>(TBufferXML::ConvertFromXML(data.c_str()));
}

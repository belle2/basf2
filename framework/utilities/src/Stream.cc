#include <framework/utilities/Stream.h>

#include <framework/pcore/EvtMessage.h>
#include <framework/logging/Logger.h>

#include <TBase64.h>
#include <TObject.h>
#include <TBufferXML.h>
#include <TMessage.h>

#include <boost/algorithm/string/replace.hpp>

using namespace Belle2;

std::string Stream::serializeXML(const TObject* obj)
{
  const TString& xmlString = TBufferXML::ConvertToXML(obj);
  return std::string(xmlString.Data());
}

std::string Stream::serializeAndEncode(const TObject* obj)
{
  TMessage::EnableSchemaEvolutionForAll();

  TMessage msg(kMESS_OBJECT);
  msg.SetWriteMode();

  // Currently disabled, but can be made to work by copying the input buffer in deserializeEncodedRawData
  // However, this is a workaround and thus doesn't seem like a good idea.
  // Once https://sft.its.cern.ch/jira/browse/ROOT-4550 is fixed, this can simply be turned on,
  // but make sure to check that old (uncompressed) data can still be deserialized.
  msg.SetCompressionLevel(0);

  msg.WriteObject(obj);
  msg.Compress(); //only does something if compression active

  //convert TMessage into base64-encoded string
  char* buf = msg.Buffer();
  UInt_t len = msg.Length();
  if (msg.CompBuffer()) {
    B2FATAL("compression used, but broken thanks to ROOT");
    buf = msg.CompBuffer();
    len = msg.CompLength();
  }

  const std::string& encodedStr(TBase64::Encode(buf, len).Data());
  return encodedStr;
}

std::string Stream::escapeXML(const std::string& xmlString)
{
  //avoid nesting CDATA sections...
  std::string newString(xmlString);
  boost::replace_all(newString, "]]>", "]]]]><![CDATA[>");
  return "<![CDATA[" + newString + "]]>";
}

TObject* Stream::deserializeXML(const std::string& data)
{
  return TBufferXML::ConvertFromXML(data.c_str());
}

TObject* Stream::deserializeEncodedRawData(const std::string& base64Data)
{
  if (base64Data.empty())
    return nullptr;

  //convert data back into raw byte stream
  const TString& data(TBase64::Decode(base64Data.c_str()));

  //const-cast ok since TMessage doesn't own the buffer
  InMessage msg(const_cast<char*>(data.Data()), data.Length());
  //some checking
  if (msg.What() != kMESS_OBJECT or msg.GetClass() == nullptr) {
    return nullptr;
  }
  TObject* obj = static_cast<TObject*>(msg.ReadObjectAny(msg.GetClass()));

  return obj;
}

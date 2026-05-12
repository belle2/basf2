/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/utilities/Stream.h>

#include <TBase64.h>
#include <TObject.h>
#include <TMessage.h>

using namespace Belle2;

namespace {
  /** Use TMessage for reading (needed constructor is protected. ) */
  class StreamMsg : public TMessage {
  public:
    StreamMsg(const void* buf, int len) : TMessage(const_cast<void*>(buf), len)
    {
      this->SetBit(kIsOwner, false);
    }
  };
}

std::string Stream::serializeAndEncode(const TObject* obj)
{
  TMessage::EnableSchemaEvolutionForAll();

  TMessage msg(kMESS_OBJECT);
  msg.SetWriteMode();

  msg.SetCompressionLevel(1);

  msg.WriteObject(obj);
  msg.Compress(); //only does something if compression active

  //convert TMessage into base64-encoded string
  char* buf = msg.Buffer();
  UInt_t len = msg.Length();

  // using the compressed buffer causes a seg fault (double free)
  // this can be tested with the unit-test StreamTest.raw
  // if (msg.CompBuffer()) {
  //   buf = msg.CompBuffer();
  //   len = msg.CompLength();
  // }

  const std::string& encodedStr(TBase64::Encode(buf, len).Data());
  return encodedStr;
}

TObject* Stream::deserializeEncodedRawData(const std::string& base64Data)
{
  if (base64Data.empty())
    return nullptr;

  //convert data back into raw byte stream
  const TString& data(TBase64::Decode(base64Data.c_str()));

  StreamMsg msg(data.Data(), data.Length());
  //some checking
  if (msg.What() != kMESS_OBJECT or msg.GetClass() == nullptr) {
    return nullptr;
  }
  auto* obj = static_cast<TObject*>(msg.ReadObjectAny(msg.GetClass()));

  return obj;
}

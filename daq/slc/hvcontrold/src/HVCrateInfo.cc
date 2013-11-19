#include "HVCrateInfo.h"

#include "base/StringUtil.h"
#include "base/Reader.h"
#include "base/Writer.h"

using namespace Belle2;

HVCrateInfo::HVCrateInfo(unsigned int id, unsigned int nslot, unsigned int nch)
{
  _id = id;
  for (size_t ns = 0; ns < nslot; ns++) {
    std::vector<HVChannelInfo*> ch_info_v;
    for (size_t nc = 0; nc < nch; nc++) {
      ch_info_v.push_back(new HVChannelInfo(id, ns + 1, nc + 1));
    }
    _ch_info_v_v.push_back(ch_info_v);
  }
}

HVCrateInfo::~HVCrateInfo() throw()
{
  for (size_t ns = 0; ns < getNSlot(); ns++) {
    for (size_t nc = 0; nc < getNChannel(); nc++) {
      delete _ch_info_v_v[ns][nc];
    }
  }
}

void HVCrateInfo::readObject(Reader& reader) throw(IOException)
{
  _id = reader.readUInt();
  size_t nslot = reader.readUInt();
  size_t nch = reader.readUInt();
  if (nslot != getNSlot() || nch != getNChannel()) {
    for (size_t ns = 0; ns < getNSlot(); ns++) {
      for (size_t nc = 0; nc < getNChannel(); nc++) {
        delete _ch_info_v_v[ns][nc];
      }
    }
    _ch_info_v_v.clear();
    for (size_t ns = 0; ns < nslot; ns++) {
      std::vector<HVChannelInfo*> ch_info_v;
      for (size_t nc = 0; nc < nch; nc++) {
        ch_info_v.push_back(new HVChannelInfo(_id, ns + 1, nc + 1));
      }
      _ch_info_v_v.push_back(ch_info_v);
    }
  }
  for (size_t ns = 0; ns < nslot; ns++) {
    for (size_t nc = 0; nc < nch; nc++) {
      HVChannelInfo* info = _ch_info_v_v[ns][nc];
      info->readObject(reader);
    }
  }
}

void HVCrateInfo::writeObject(Writer& writer) const throw(IOException)
{
  writer.writeUInt(_id);
  size_t nslot = getNSlot();
  writer.writeUInt(nslot);
  size_t nch = getNChannel();
  writer.writeUInt(nch);
  for (size_t ns = 0; ns < nslot; ns++) {
    for (size_t nc = 0; nc < nch; nc++) {
      HVChannelInfo* info = _ch_info_v_v[ns][nc];
      info->writeObject(writer);
    }
  }
}

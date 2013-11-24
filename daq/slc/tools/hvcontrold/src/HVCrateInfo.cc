#include "HVCrateInfo.h"

#include "base/StringUtil.h"
#include "base/Reader.h"
#include "base/Writer.h"

#include <iostream>

using namespace Belle2;

HVCrateInfo::HVCrateInfo(unsigned int id, unsigned int nslot, unsigned int nch)
{
  _id = id;
  allocate(_id, nslot, nch);
}

HVCrateInfo::~HVCrateInfo() throw()
{
  for (size_t ns = 0; ns < getNSlot(); ns++) {
    for (size_t nc = 0; nc < getNChannel(); nc++) {
      delete _ch_info_v_v[ns][nc];
      delete _ch_status_v_v[ns][nc];
    }
  }
}

void HVCrateInfo::allocate(unsigned int id, unsigned int nslot, unsigned int nch)
{
  if (nslot != getNSlot() || nch != getNChannel()) {
    for (size_t ns = 0; ns < getNSlot(); ns++) {
      for (size_t nc = 0; nc < getNChannel(); nc++) {
        delete _ch_info_v_v[ns][nc];
        delete _ch_status_v_v[ns][nc];
      }
    }
    _ch_info_v_v.clear();
    _ch_status_v_v.clear();
    for (size_t ns = 0; ns < nslot; ns++) {
      std::vector<HVChannelInfo*> ch_info_v;
      std::vector<HVChannelStatus*> ch_status_v;
      for (size_t nc = 0; nc < nch; nc++) {
        ch_info_v.push_back(new HVChannelInfo(id, ns + 1, nc + 1));
        ch_status_v.push_back(new HVChannelStatus(id, ns + 1, nc + 1));
      }
      _ch_info_v_v.push_back(ch_info_v);
      _ch_status_v_v.push_back(ch_status_v);
    }
  }
}

void HVCrateInfo::readObject(Reader& reader) throw(IOException)
{
  _id = reader.readUInt();
  size_t nslot = reader.readUInt();
  size_t nch = reader.readUInt();
  allocate(_id, nslot, nch);
  for (size_t ns = 0; ns < nslot; ns++) {
    for (size_t nc = 0; nc < nch; nc++) {
      HVChannelInfo* info = _ch_info_v_v[ns][nc];
      HVChannelStatus* status = _ch_status_v_v[ns][nc];
      info->readObject(reader);
      status->readObject(reader);
    }
  }
}

void HVCrateInfo::writeObject(Writer& writer) const throw(IOException)
{
  size_t nslot = getNSlot();
  size_t nch = getNChannel();
  writer.writeUInt(_id);
  writer.writeUInt(nslot);
  writer.writeUInt(nch);
  for (size_t ns = 0; ns < nslot; ns++) {
    for (size_t nc = 0; nc < nch; nc++) {
      HVChannelInfo* info = _ch_info_v_v[ns][nc];
      HVChannelStatus* status = _ch_status_v_v[ns][nc];
      info->writeObject(writer);
      status->writeObject(writer);
    }
  }
}

void HVCrateInfo::readInfo(Reader& reader) throw(IOException)
{
  _id = reader.readUInt();
  size_t nslot = reader.readUInt();
  size_t nch = reader.readUInt();
  allocate(_id, nslot, nch);
  for (size_t ns = 0; ns < nslot; ns++) {
    for (size_t nc = 0; nc < nch; nc++) {
      HVChannelInfo* info = _ch_info_v_v[ns][nc];
      info->readObject(reader);
    }
  }
}

void HVCrateInfo::writeInfo(Writer& writer) const throw(IOException)
{
  size_t nslot = getNSlot();
  size_t nch = getNChannel();
  writer.writeUInt(_id);
  writer.writeUInt(nslot);
  writer.writeUInt(nch);
  for (size_t ns = 0; ns < nslot; ns++) {
    for (size_t nc = 0; nc < nch; nc++) {
      HVChannelInfo* info = _ch_info_v_v[ns][nc];
      info->writeObject(writer);
    }
  }
}

void HVCrateInfo::readStatus(Reader& reader) throw(IOException)
{
  _id = reader.readUInt();
  size_t nslot = reader.readUInt();
  size_t nch = reader.readUInt();
  allocate(_id, nslot, nch);
  for (size_t ns = 0; ns < nslot; ns++) {
    for (size_t nc = 0; nc < nch; nc++) {
      HVChannelStatus* status = _ch_status_v_v[ns][nc];
      status->readObject(reader);
    }
  }
}

void HVCrateInfo::writeStatus(Writer& writer) const throw(IOException)
{
  size_t nslot = getNSlot();
  size_t nch = getNChannel();
  writer.writeUInt(_id);
  writer.writeUInt(nslot);
  writer.writeUInt(nch);
  for (size_t ns = 0; ns < nslot; ns++) {
    for (size_t nc = 0; nc < nch; nc++) {
      HVChannelStatus* status = _ch_status_v_v[ns][nc];
      status->writeObject(writer);
    }
  }
}


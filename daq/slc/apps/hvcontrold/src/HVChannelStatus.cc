#include "daq/slc/apps/hvcontrold/HVChannelStatus.h"

#include <daq/slc/base/Writer.h>
#include <daq/slc/base/Reader.h>

#include <cstring>

using namespace Belle2;

HVChannelStatus::HVChannelStatus()
{
  memset(&_status, 0, sizeof(hv_channel_status));
  setState(HVState::OFF_S);
}

void HVChannelStatus::setStatus(const hv_channel_status& status)
{
  memcpy(&_status, &status, sizeof(hv_channel_status));
}

void HVChannelStatus::setStatus(const hv_channel_status* status)
{
  memcpy(&_status, status, sizeof(hv_channel_status));
}

void HVChannelStatus::writeObject(Writer& writer) const throw(IOException)
{
  writer.writeInt(_status.configid);
  writer.writeInt(_status.state);
  writer.writeFloat(_status.voltage_mon);
  writer.writeFloat(_status.current_mon);
}

void HVChannelStatus::readObject(Reader& reader) throw(IOException)
{
  _status.configid = reader.readInt();
  _status.state = reader.readInt();
  _status.voltage_mon = reader.readFloat();
  _status.current_mon = reader.readFloat();
}

#include "daq/slc/apps/hvcontrold/HVChannelInfo.h"

#include <daq/slc/base/Writer.h>
#include <daq/slc/base/Reader.h>

#include <cstring>
#include <sstream>

using namespace Belle2;

void HVChannelInfo::setConfig(const HVChannelConfig& config)
{
  _config.setConfig(config.getConfig());
}

void HVChannelInfo::setStatus(const HVChannelStatus& status)
{
  _status.setStatus(status.getStatus());
}


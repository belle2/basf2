#include "daq/slc/hvcontrol/arich/ArichHVCommunicator.h"
#include "daq/slc/hvcontrol/arich/ArichHVTUI.h"

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

#include <iostream>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 3) {
    std::cout << "Usage : " << argv[0] << " <config> <calib>" << std::endl;
    return 1;
  }
  std::string confname = StringUtil::form("hvcontrol/%s", argv[1]);
  std::string calibname = StringUtil::form("hvcontrol/%s", argv[2]);
  ArichHVTUI tui;
  ConfigFile cfile(confname, calibname);
  const int crate = cfile.getInt("hv.crateid");
  const int nslots  = cfile.getInt("hv.nchannels");
  for (int i = 0; i < nslots; i++) {
    int slot = cfile.getInt(StringUtil::form("hv.channel[%d].slot", i + 1));
    int channel = cfile.getInt(StringUtil::form("hv.channel[%d].channel", i + 1));
    tui.addChannel(crate, slot, channel);
  }
  tui.initialize(confname, calibname);
  tui.run();
  return (0);
}

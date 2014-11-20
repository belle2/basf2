#ifndef WaveMergingModule_H
#define WaveMergingModule_H

#include <framework/core/Module.h>

#include <topcaf/dataobjects/EventWaveformPacket.h>

#include <map>
#include <TH1D.h>
#include <sstream>
#include <TFile.h>
#include <TProfile.h>
#include <TCanvas.h>

namespace Belle2 {

  class WaveMergingModule : public Module {
  public:

    WaveMergingModule();
    ~WaveMergingModule();

    //
    void initialize();
    void event();
    void terminate();

    //    RawEvent* MergeWavePackets(const RawEvent *raw_event);

  private:

  };

  typedef std::vector<EventWaveformPacket*> EventWaveformPackets;
}
#endif

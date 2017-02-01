#include <framework/core/Module.h>
#include <top/dataobjects/TOPRawWaveform.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include "TH1F.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TMultiGraph.h"
#include <string>
#include <set>
#include <map>
#include <vector>
#include <deque>
#include <utility>

namespace Belle2 {
  class TOPDataQualityOnlineModule : public Module {
    typedef unsigned long long topcaf_channel_id_t;
  public:
    TOPDataQualityOnlineModule();

    void defineHisto();
    void initialize();
    void beginRun();
    void event();
    void endRun();
    void terminate();
    void clear_graph();
    void update_graph();
    void drawWaveforms(TOPRawWaveform* wave);
    //void basicDebuggingPlots(TOPRawWaveform* wave);

  private:
    int m_iFrame;
    int m_iEvent;
    int m_refreshCount;
    int m_framesPerEvent;
    int m_nhits;
    //map: scrod_id --> (asic_id --> TMultiGraph)
    std::map<int, std::map<int, TMultiGraph*>> m_channels;
    std::map<int, std::map<int, std::set<int>>> m_channelLabels;
    std::map<int, TCanvas*> m_canvas; // map: scrod_id --> TCanvas
    TCanvas* m_canvas_nhits;
    TH1F* m_h_nhits;
  };

}

#include <framework/core/HistoModule.h>
#include <framework/core/Module.h>
#include <topcaf/dataobjects/EventWaveformPacket.h>
#include "TH1F.h"
#include "TH2F.h"
#include "TMultiGraph.h"
#include "TDirectory.h"
#include <string>
#include <set>
#include <map>
#include <utility>

namespace Belle2 {
  class TOPCAF_DataQualityModule : public HistoModule {
    typedef unsigned long long topcaf_channel_id_t;
  public:
    TOPCAF_DataQualityModule();

    void defineHisto();
    void initialize();
    void beginRun();
    void event();
    void endRun();
    void terminate();
    void drawWaveforms(EventWaveformPacket* ewp);
    void basicDebuggingPlots(EventWaveformPacket* ewp);

  private:
    //FIXME temporarily need to keep track of events in a file until event builder works
    int m_iEvent; // keep track of iterations within run
    TH1F* m_samples;//Plot all ADC sample values.
    TH2F* m_hitmap;
    TH1F* m_scrod_id;
    TH1F* m_asic_row;
    TH1F* m_asic_col;
    TH1F* m_asic_ch;
    TH1F* m_errorFlag;
    TH1F* m_flag;
    TH1F* m_asic_win;
    TH1F* m_entries;
    std::map<topcaf_channel_id_t, TH1F*> m_channelNoiseMap; // histogram of the values after correction;
    std::map<topcaf_channel_id_t, int> m_channelEventMap; // to find in which chunk a given channel is

    std::map<int, std::map<std::string, TMultiGraph*>> m_channels; // per board stack, per asic
    std::map<int, std::map<std::string, std::set<int>>> m_channelLabels; // per board stack, per asic

    bool m_DRAWWAVES;
    bool m_DEBUGGING;

    std::string m_histogramDirectoryName;
    TDirectory* m_directory;

  };

}

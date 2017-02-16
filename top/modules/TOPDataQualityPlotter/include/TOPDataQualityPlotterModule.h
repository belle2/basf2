#include <framework/core/HistoModule.h>
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <top/dataobjects/TOPRawWaveform.h>

#include <string>
#include <set>
#include <map>
#include <utility>

#include "TH1F.h"
#include "TH2F.h"
#include "TProfile.h"
#include "TMultiGraph.h"
#include "TDirectory.h"

namespace Belle2 {
  class TOPDataQualityPlotterModule : public HistoModule {
    typedef unsigned long long topcaf_channel_id_t;
  public:
    TOPDataQualityPlotterModule();

    void defineHisto();
    void initialize();
    void beginRun();
    void event();
    void endRun();
    void terminate();
    void drawWaveforms(TOPRawWaveform* rawwave);
    void basicDebuggingPlots(TOPRawWaveform* rawwave);

  private:
    int m_iEvent; // keep track of iterations within run
    TH1F* m_samples;//Plot all ADC sample values.
    std::map<int, TH2F*> m_hitmap; // one hitmap per scrod
    TH1F* m_scrod_id;
    TH1F* m_asic;
    TH1F* m_carrier;
    TH1F* m_asic_ch;
    TH1F* m_errorFlag;
    TH1F* m_flag;
    TH1F* m_asic_win;
    TH1F* m_entries;
    TH1F* m_moduleID;
    TH1F* m_pixelID;

    StoreArray<TOPRawWaveform> m_waveform;
    std::map<topcaf_channel_id_t, TH1F*> m_channelNoiseMap; // histogram of the values after correction;
    std::map<topcaf_channel_id_t, int> m_channelEventMap; // to find in which chunk a given channel is

    std::map<int, std::map<std::string, TMultiGraph*>> m_channels; // per board stack, per asic
    std::map<int, std::map<std::string, std::set<int>>> m_channelLabels; // per board stack, per asic

    std::map<int, std::map<int, std::map<int, std::map<int, TProfile*>>>> m_waveformHists; // scrod, carrier, asic, channel

    bool m_DRAWWAVES;
    bool m_DEBUGGING;
    bool m_NOISE;

    std::string m_histogramDirectoryName;
    TDirectory* m_directory;

  };

}

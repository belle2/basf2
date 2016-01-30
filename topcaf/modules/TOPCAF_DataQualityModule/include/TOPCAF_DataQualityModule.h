#include <framework/core/HistoModule.h>

#include <framework/core/Module.h>
#include "TH1F.h"
#include <string>

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

  private:
    //FIXME temporarily need to keep track of events in a file until event builder works
    int m_iEvent; // keep track of iterations within run
    TH1F* m_samples;//Plot all ADC sample values.
    std::map<topcaf_channel_id_t, TH1F*> m_channelNoiseMap; // histogram of the values after correction;
    std::map<topcaf_channel_id_t, int> m_channelEventMap; // to find in which chunk a given channel is
    std::string m_histogramDirectoryName;
  };

}

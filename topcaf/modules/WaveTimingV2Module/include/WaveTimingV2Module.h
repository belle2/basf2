#ifndef WaveTimingV2Module_H
#define WaveTimingV2Module_H

#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>
#include <topcaf/dataobjects/TOPCAFDigit.h>
#include "TH1D.h"
#include "TH2F.h"

namespace Belle2 {
  class WaveTimingV2Module : public Module {
  public:

    WaveTimingV2Module();
    ~WaveTimingV2Module();

    //
    void initialize();
    void beginRun();
    void event();
    void terminate();
    //    void defineHisto();

  private:
    double m_fraction, m_time_delay, m_rate, m_sigma, m_thresh;
    double m_crude_time, m_cf_time;
    TH1D* m_tmp_h;
    //    TH2F* m_occupancy;
    StoreArray<TOPCAFDigit>  m_topcafdigits_ptr;

    //
    double m_time2tdc;
  };

}
#endif

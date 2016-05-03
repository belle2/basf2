#ifndef WaveTimingV5Module_H
#define WaveTimingV5Module_H

#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>
#include <topcaf/dataobjects/TOPCAFDigit.h>
#include "TH1D.h"
#include "TH2F.h"

namespace Belle2 {
  class WaveTimingV5Module : public Module {
  public:

    WaveTimingV5Module();
    ~WaveTimingV5Module();

    //
    void initialize();
    void beginRun();
    void event();
    void terminate();
    //    void defineHisto();

  private:
    double m_fraction, m_time_delay, m_rate, m_sigma, m_thresh, m_frac, m_thresh_n;
    double m_crude_time, m_cf_time;
    StoreArray<TOPCAFDigit>  m_topcafdigits_ptr;
    double m_time2tdc;
    bool m_dTcalib;
  };

}
#endif

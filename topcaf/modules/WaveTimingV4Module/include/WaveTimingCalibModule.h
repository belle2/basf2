#ifndef WaveTimingCalib_H
#define WaveTimingCalib_H

#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>
#include <topcaf/dataobjects/TOPCAFDigitCalib.h>
#include "TH1D.h"
#include "TH2F.h"

namespace Belle2 {
  class WaveTimingCalibModule : public Module {
  public:

    WaveTimingCalibModule();
    ~WaveTimingCalibModule();

    //
    void initialize();
    void beginRun();
    void event();
    void terminate();
    //    void defineHisto();

  private:
    double m_fraction, m_time_delay, m_rate, m_sigma, m_thresh, m_frac, m_thresh_n;
    double m_crude_time, m_cf_time;
    StoreArray<TOPCAFDigitCalib>  m_topcafdigits_ptr;
    double m_time2tdc;
    bool m_dTcalib;
    bool m_isSkim;
  };

}
#endif

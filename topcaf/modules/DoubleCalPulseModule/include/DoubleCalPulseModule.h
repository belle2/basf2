#ifndef DoubleCalPulseModule_H
#define DoubleCalPulseModule_H

#include <map>
#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>
#include <topcaf/dataobjects/TOPCAFDigit.h>
#include "TH1D.h"
#include "TH2F.h"

namespace Belle2 {
  class DoubleCalPulseModule : public Module {
  public:

    DoubleCalPulseModule();
    ~DoubleCalPulseModule();

    //
    void initialize();
    void beginRun();
    void event();
    void terminate();
    //    void defineHisto();

  private:
    double m_fraction, m_time_delay, m_rate, m_sigma, m_thresh;
    double m_crude_time, m_cf_time;
    int m_cal_ch;
    TH1D* m_tmp_h;

    //    TH2F* m_occupancy;

    //
    double m_time2tdc;
  };

}
#endif

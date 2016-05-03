#ifndef DoubleCalPulseV5Module_H
#define DoubleCalPulseV5Module_H

#include <map>
#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>
#include <topcaf/dataobjects/TOPCAFDigit.h>
#include "TH1D.h"
#include "TH2F.h"

namespace Belle2 {
  class DoubleCalPulseV5Module : public Module {
  public:

    DoubleCalPulseV5Module();
    ~DoubleCalPulseV5Module();

    void initialize();
    void beginRun();
    void event();
    void terminate();
    //    void defineHisto();

  private:
    int m_cal_ch;
    double m_tmin, m_tmax;
    double m_wmin, m_wmax;
    double m_adcmin, m_adcmax;

  };

}
#endif

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

#ifndef DoubleCalPulseCalibModule_H
#define DoubleCalPulseCalibModule_H

#include <map>
#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>
//#include <framework/datastore/SelectSubset.h>
#include <topcaf/dataobjects/TOPCAFDigitCalib.h>
#include "TH1D.h"
#include "TH2F.h"

namespace Belle2 {
  class DoubleCalPulseCalibModule : public Module {
  public:

    DoubleCalPulseCalibModule();
    ~DoubleCalPulseCalibModule();

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
    //  SelectSubset<TOPCAFDigit> m_selector;
  };

}
#endif

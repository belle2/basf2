#ifndef DoubleCalPulseV2Module_H
#define DoubleCalPulseV2Module_H

#include <map>
#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>
#include <topcaf/dataobjects/TOPCAFDigit.h>
#include "TH1D.h"
#include "TH2F.h"

namespace Belle2 {
  class DoubleCalPulseV2Module : public Module {
  public:

    DoubleCalPulseV2Module();
    ~DoubleCalPulseV2Module();

    void initialize();
    void beginRun();
    void event();
    void terminate();
    //    void defineHisto();

  private:
    int m_cal_ch;
  };

}
#endif

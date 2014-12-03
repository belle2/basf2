#ifndef WaveTimingModule_H
#define WaveTimingModule_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <top/dataobjects/TOPDigit.h>
#include "TH1D.h"

namespace Belle2 {
  class WaveTimingModule : public Module {
  public:

    WaveTimingModule();
    ~WaveTimingModule();

    //
    void initialize();
    void event();
    void terminate();

  private:
    double m_fraction, m_time_delay, m_rate;
    double m_crude_time, m_cf_time;
    TH1D* m_tmp_h;
    StoreArray<TOPDigit>  m_topdigits_ptr;

    //
    double m_time2tdc;
  };

}
#endif

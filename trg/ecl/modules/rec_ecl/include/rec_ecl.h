#ifndef REC_ECL_H
#define REC_ECL_H

#include <framework/core/Module.h>
#include <framework/core/HistoModule.h>        // <- Substitution of HistoModule.h
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLCalDigit.h>
#include "trg/ecl/TrgEclMapping.h"
#include "trg/ecl/dataobjects/TRGECLCluster.h"
#include <stdio.h>
#include <vector>
#include <algorithm>

namespace Belle2 {

  class rec_eclModule : public Module {  // <- derived from HistoModule class

  public:
    //! Constructor
    rec_eclModule();
    //! Destructor
    virtual ~rec_eclModule();

    //!Module functions
    virtual void initialize();
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

  private:

    TrgEclMapping* trgeclmap;
    //
    StoreArray<ECLCalDigit> m_ECLCalDigitData;
    StoreArray<ECLDigit>    m_ECLDigitData;
    StoreArray<TRGECLCluster> m_TRGECLCluster;

  };
}
#endif

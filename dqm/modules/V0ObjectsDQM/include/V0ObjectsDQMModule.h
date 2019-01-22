//+
// File : V0ObjectsDQMModule.h
// Description : Module to monitor displaced vertices
//
// Author : Bryan Fulsom PNNL
// Date : 2019-01-17
//-

#include <framework/core/HistoModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>
#include <analysis/dataobjects/ParticleList.h>
#include <string>
#include "TH2F.h"
#include "TH1F.h"

namespace Belle2 {

  class V0ObjectsDQMModule : public HistoModule {

  public:

    V0ObjectsDQMModule();

    void initialize() override;
    void beginRun() override;
    void event() override;
    void endRun() override;
    void terminate() override;

    void defineHisto() override;

  private:

    /* x vs. y in slices of z */
    TH2F* m_h_xvsy[32] = {nullptr};
    /** Trigger identifier string used to select events for the histograms */
    std::string m_triggerIdentifier = "";

    /** Name of the V0 particle list */
    std::string m_V0PListName = "";
  };

} // end namespace Belle2


//+
// File : PysicsObjectsDQMModule.h
// Description : Module to monitor physics objects
//
// Author : Boqun Wang, University of Cincinnati
// Date :  May - 2018
//-

#include <framework/core/HistoModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>
#include <analysis/dataobjects/ParticleList.h>
#include <string>
#include "TH1F.h"

namespace Belle2 {

  class PhysicsObjectsDQMModule : public HistoModule {

  public:

    PhysicsObjectsDQMModule();

    void initialize() override;
    void beginRun() override;
    void event() override;
    void endRun() override;
    void terminate() override;

    void defineHisto() override;

  private:
    /** KS0 invariant mass */
    TH1F* m_h_mKS0 = nullptr;

    /** PI0 invariant mass */
    TH1F* m_h_mPI0 = nullptr;

    /** R2 */
    TH1F* m_h_R2 = nullptr;

    /** Trigger identifier string used to select events for the histograms */
    std::string m_triggerIdentifier = "";

    /** Name of the pi0 particle list */
    std::string m_pi0PListName = "";

    /** Name of the KS0 particle list */
    std::string m_ks0PListName = "";
  };

} // end namespace Belle2


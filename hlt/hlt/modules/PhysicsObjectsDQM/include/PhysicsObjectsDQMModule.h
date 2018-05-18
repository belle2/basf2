//+
// File : PysicsObjectsDQMModule.h
// Description : Module to monitor physics objects
//
// Author : Boqun Wang
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
#include "TH2F.h"

namespace Belle2 {

  class PhysicsObjectsDQMModule : public HistoModule {

  public:

    PhysicsObjectsDQMModule();
    virtual ~PhysicsObjectsDQMModule();

    virtual void initialize();
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    virtual void defineHisto();

  private:
    // Internal storage of the tracks as particles.
    StoreObjPtr<ParticleList> m_pionParticles;
    // Internal storage of the ECL clusters as particles.
    StoreObjPtr<ParticleList> m_gammaParticles;

    /** KS0 invariant mass */
    TH1F* m_h_mKS0;

    /** PI0 invariant mass */
    TH1F* m_h_mPI0;

    std::string m_cut_string;
  };

} // end namespace Belle2


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
    virtual ~PhysicsObjectsDQMModule();

    virtual void initialize();
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    virtual void defineHisto();

  private:
    /** KS0 invariant mass */
    TH1F* m_h_mKS0 = nullptr;

    /** PI0 invariant mass */
    TH1F* m_h_mPI0 = nullptr;

    /** String to tag events */
    std::string m_tag_string = "";
  };

} // end namespace Belle2


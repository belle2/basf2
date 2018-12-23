#ifndef TRCECLQAMMODULE_h
#define TRCECLQAMMODULE_h


#include "trg/ecl/dataobjects/TRGECLUnpackerStore.h"
#include <mdst/dataobjects/TRGSummary.h>


#include <framework/datastore/StoreObjPtr.h>
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/DataStore.h>
#include <stdlib.h>
#include <iostream>
#include <TFile.h>
#include <TTree.h>
#include <TString.h>

//using namespace std;

namespace Belle2 {

  class TRGECLQAMModule : public Module {

  public:
    //! Costructor
    TRGECLQAMModule();
    //! Destrunctor
    virtual ~TRGECLQAMModule();
    int basf2evt;
  public:
    //! version
    std::string version() const;
    //! initialize
    virtual void initialize() override;
    //! begin Run
    virtual void beginRun() override;
    //! Event
    virtual void event() override;
    //! End Run
    virtual void endRun() override;
    //! terminate
    virtual void terminate() override;


  private:
    //! The run #
    int m_nRun;
    //! The # of Events
    int m_nevent;
    /** Object of ECLTRG Physics Efficiency */
    //! ECLTRG Total Energy Efficiency
    double m_psnm8;
    //! ECLTRG Cluster Efficiency
    double m_psnm13;
    //! ECLTRG Physics Efficiency
    double m_psnm8_13;

    //** GDL Logic Test */
    //! Track Trigger Rate
    double m_psnm7_15;
    //! ECLTRG Total Energy Rate
    double m_psnm8_15;
    //! ECLTRG Cluster Rate
    double m_psnm13_15;

    //** ECLTRG TC Hit Map
    //! FWD problem TC
    Double_t m_FWD;
    //! Barrel problem TC
    double m_BAR;
    //! BWD problem TC
    double m_BWD;
    //! Total proble TC
    double m_ALL;

    //! Hit TCID
    int TCID[576];

    //! Output Root File Name
    std::string m_outputfile;

    //!
    StoreArray<TRGECLUnpackerStore> m_TRGECLUnpackerStore;
    StoreObjPtr<TRGSummary> m_TRGSummary;
    //    StoreObjPtr<EventMetaData> EvtMeta;

  };


}

#endif

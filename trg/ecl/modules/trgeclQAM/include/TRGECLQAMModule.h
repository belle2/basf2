/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef TRCECLQAMMODULE_h
#define TRCECLQAMMODULE_h


#include "trg/ecl/dataobjects/TRGECLUnpackerStore.h"
#include "trg/ecl/dataobjects/TRGECLUnpackerEvtStore.h"

#include <mdst/dataobjects/TRGSummary.h>


#include <framework/datastore/StoreObjPtr.h>
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <vector>

#include <TH1.h>


//using namespace std;

namespace Belle2 {

  class TRGECLQAMModule : public Module {

  public:
    //! Costructor
    TRGECLQAMModule();
    //! Destrunctor
    virtual ~TRGECLQAMModule();

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

    //ECLTRG Data
    //! Total Energy Histogram
    TH1F* h_etot = new TH1F("h_etot", "Energy Total", 4000, 0, 4000);
    //! Total Energy
    int m_etot;
    //! Total Energy mean
    double m_etot_mean;
    //! Total Energy error
    double m_etot_error;
    //! Total Energy sigma
    double m_etot_sigma;
    //! tcenergy check
    std::vector<int> etot;

    //! Caltime Histogram
    TH1F* h_caltime = new TH1F("h_caltime", "Caltime", 2000, -1000, 1000);
    //! Caltime mean
    double m_caltime_mean;
    //! Caltime error
    double m_caltime_error;
    //! Caltime sigma
    double m_caltime_sigma;
    //! Caltime check
    std::vector<int> caltime;

    // Cluster Energy 2D plot
    //! Cluster Energy Histogram
    TH1F* h_clusterE = new TH1F("h_clusterE", "ClusterE", 4000, 0, 4000);
    //! Cluster Energy
    int clusterE;
    //! Cluster Energy Vector
    std::vector<int> cluster;

    //! ECL Trigger Unpacker TC output
    StoreArray<TRGECLUnpackerStore> m_TRGECLUnpackerStore;
    //! ECL Trigger Unpacker Event output
    StoreArray<TRGECLUnpackerEvtStore> m_TRGECLUnpackerEvtStore;
    //! Trigger Summary
    StoreObjPtr<TRGSummary> m_TRGSummary;

  };


}

#endif

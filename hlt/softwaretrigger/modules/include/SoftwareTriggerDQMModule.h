#ifndef PHYSICSTRIGGERDQMMODULE_H
#define PHYSICSTRIGGERDQMMODULE_H
//+
// File : PysicsTriggerDQMModule.h
// Description : Module to monitor raw data accumulating histos
//
// Author : Chunhua LI, the University of Melbourne
// Date :  4 - March - 2015
//-

#include <stdlib.h>
#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/resource.h>
#include <sys/uio.h>

#include <framework/core/HistoModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>
#include "TH1F.h"
#include "TH2F.h"

namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class SoftwareTriggerDQMModule : public HistoModule {

    // Public functions
  public:

    //! Constructor / Destructor
    SoftwareTriggerDQMModule();
    virtual ~SoftwareTriggerDQMModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    //! Histogram definition
    virtual void defineHisto();

    // Data members

  private:


    //! Histograms
    /**the number of tracks*/
    TH1F* h_NTrack;

    /**the number of ECL clusters*/
    TH1F* h_NCluster;

    /**the total deposited energy in ECL*/
    TH1F* h_ESum;

    /**the total visible energy*/
    TH1F* h_EVis;

    /**Overall HLT contains two parts: Fast_Reco + hlt*/
    /**the fast_Reco trigger rate*/
    TH1F* h_fast_reco_map;

    /**unique fast reco*/
    TH1F* h_fast_reco_unique_map;


    /**the hlt trigger rate, R = the number of event trigger by the line to the total number of events triggered by hlt*/
    TH1F* h_hlt_map;

    /**the unique hlt trigger rat, R = the number of event trigger by the line ONLY to the total number of event trigger by hlt*/
    TH1F* h_hlt_unique_map;

    /**D0, D+, D*+, Jpsi->ee, Jpsi->mumu mass*/
    TH1F* h_dqm[5];

  };

} // end namespace Belle2

#endif // MODULEHELLO_H

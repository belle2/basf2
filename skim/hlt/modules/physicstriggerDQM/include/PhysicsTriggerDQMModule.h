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

  class PhysicsTriggerDQMModule : public HistoModule {

    // Public functions
  public:

    //! Constructor / Destructor
    PhysicsTriggerDQMModule();
    virtual ~PhysicsTriggerDQMModule();

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

    /**the largest momentum*/
    TH1F* h_P1;

    /**the largest energy of clusters*/
    TH1F* h_E1;

    /**the maximum angle between tracks*/
    TH1F* h_MaxAngleTT;

    /**the maximum angle between clusters*/
    TH1F* h_MaxAngleGG;


    //histograms for track information
    // TH1F* h_TrackP;
    // TH1F* h_TrackCostheta;

    /**the azimuthal angle of tracks*/
    // TH1F* h_TrackPhi;

    /**the Dr*/
    // TH1F* h_TrackDr;

    /**Dz*/
    // TH1F* h_TrackDz;

    //histograms for cluster information
    /**the energy of clusters*/
    // TH1F* h_ClusterE;

    /**the polar angle of clusters*/
    //  TH1F* h_ClusterTheta;


    //TH1F* h_ClusterPhi;
    /**the timing of clusters*/
    // TH1F* h_ClusterTiming;
  };

} // end namespace Belle2

#endif // MODULEHELLO_H

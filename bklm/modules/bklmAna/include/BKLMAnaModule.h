/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:                                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BKLMANA_H
#define BKLMANA_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include "bklm/geometry/Module.h"
#include <bklm/geometry/GeometryPar.h>
#include <mdst/dataobjects/Track.h>
#include <bklm/dataobjects/BKLMHit2d.h>
#include <tracking/dataobjects/ExtHit.h>
#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TH2D.h"

#include <map>
#include <iostream>
#include <set>

//using namespace std;

namespace Belle2 {

  /*! A class defining a module that perform efficiencies studies on bklm*/

  class BKLMAnaModule : public Module {

  public:

    //! Constructor
    BKLMAnaModule();

    //! Destructor
    virtual ~BKLMAnaModule();

    //! Initialize at start of job
    virtual void initialize();

    //! Do any needed actions at the start of a simulation run
    virtual void beginRun();

    virtual void event();

    //! Do any needed actions at the end of a simulation run
    virtual void endRun();

    //! Terminate at the end of job
    virtual void terminate();

  protected:

    //! output TFile
    TFile* m_file;

    //! name of the output TFile
    std::string m_filename;

    //! TTree that holds several variable of interest
    TTree* m_extTree;


  private:

    //! run number
    int  m_run;

    //! number of validated ExtHit in BKLM in one event
    int  m_nExtHit;

    //! keep the global position of validated ExtHit in BKLM
    float m_extx[200];

    //! keep the global position of validated ExtHit in BKLM
    float m_exty[200];

    //! keep the global position of validated ExtHit in BKLM
    float m_extz[200];


    //! bklm GeometryPar
    //bklm::GeometryPar* m_GeoPar;

    //! distribution of distance between mathced extHit and bklmHit2d
    TH1F* m_hdistance;

    //! histogram of total entries of extHit vs. track momentum
    TH1F* m_totalMom;

    //! histogram of entries with matched bklmHit2d of extHit vs. track momentum
    TH1F* m_passMom;

    //! efficiency associating to ExtHit vs. track momentum
    TH1F* m_effiMom;

    //! histogram of total entries of extHit associating to ExtHit of each layer vs.theta phi angle of ExtHit
    TH2F* m_totalThephi[15];

    //! histogram of entries with matched bklmHit2d of extHit vs.theta phi angle of ExtHit
    TH2F* m_passThephi[15];

    //! efficiency associating to ExtHit of each layer vs.theta phi angle of ExtHit
    TH2F* m_effiThephi[15];

    //! histogram of total entries of extHit associating to ExtHit of each layer vs. theta phi angle of the track
    TH2F* m_totalTrkThephi[15];

    //! histogram of extHit with matched bklmHit2d of each layer vs. theta phi angle of the track
    TH2F* m_passTrkThephi[15];

    //! efficiency associating to ExtHit of each layer vs. theta phi angle of the track
    TH2F* m_effiTrkThephi[15];

    //! Y X coordinate of total entries of extHit that in klm region
    TH2F* m_totalYX;

    //! Y X coordinate of extHit with matched bklmHit2d
    TH2F* m_passYX;

    //! associated efficiencies vs glaoble y, x coordinate
    TH2F* m_effiYX;

    //! Y Z coordinate of total entries of extHit that in klm region
    TH2F* m_totalYZ;

    //! Y Z coordinate of extHit with matched bklmHit2d
    TH2F* m_passYZ;

    //! associated efficiencies vs glaoble y, z coordinate
    TH2F* m_effiYZ;

    //! hits2D StoreArray
    StoreArray<BKLMHit2d> hits2D;
    //! extHits StoreArray
    StoreArray<ExtHit> extHits;
    //! tracks StoreArray
    StoreArray<Track> tracks;

  };

} // end of namespace Belle2

#endif //MODULEBKLMANA_H 

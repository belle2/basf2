/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dataobjects/bklm/BKLMHit2d.h>

/* Belle 2 headers. */
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>

/* ROOT headers. */
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TTree.h>

namespace Belle2 {

  /*! A class defining a module that perform efficiencies studies on bklm*/

  class BKLMAnaModule : public Module {

  public:

    //! Constructor
    BKLMAnaModule();

    //! Destructor
    ~BKLMAnaModule();

    //! Initialize at start of job
    void initialize() override;

    //! Do any needed actions at the start of a simulation run
    void beginRun() override;

    //! This method is called for each event
    void event() override;

    //! Do any needed actions at the end of a simulation run
    void endRun() override;

    //! Terminate at the end of job
    void terminate() override;

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

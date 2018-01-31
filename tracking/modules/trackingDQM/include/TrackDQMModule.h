/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys                                              *
 *                                                                        *
 * Prepared for track quality check                                       *
 * Prepared for Phase 2 and Belle II geometry                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/HistoModule.h>

#include <framework/core/Module.h>
#include <framework/core/HistoModule.h>
#include <TH1F.h>

namespace Belle2 {

  /** DQM of tracks
    * their momentum,
    * Number of hits in tracks,
    * Number of tracks.
    *
    */
  class TrackDQMModule : public HistoModule {  // <- derived from HistoModule class

  public:

    /** Constructor */
    TrackDQMModule();
    /* Destructor */
    ~TrackDQMModule();

    /** Module functions */
    void initialize() override final;
    void beginRun() override final;
    void event() override final;
    void endRun() override final;
    void terminate() override final;

    /**
    * Histogram definitions such as TH1(), TH2(), TNtuple(), TTree().... are supposed
    * to be placed in this function.
    */
    void defineHisto() override final;

  private:
    /** Name of the collection to use for the Reco Track */
//    std::string m_storeRecoTrackName;
    /** Frames StoreArray name */
//    std::string m_storeFramesName;

    /** Track momentum Pt.X */
    TH1F* m_MomX;
    /** Track momentum Pt.Y */
    TH1F* m_MomY;
    /** Track momentum Pt.Z */
    TH1F* m_MomZ;
    /** Track momentum Pt */
    TH1F* m_MomPt;
    /** Track momentum Magnitude */
    TH1F* m_Mom;
    /** Number of hits on PXD */
    TH1F* m_HitsPXD;
    /** Number of hits on VXD */
    TH1F* m_HitsSVD;
    /** Number of hits on CDC */
    TH1F* m_HitsCDC;
    /** Number of all hits in tracks */
    TH1F* m_Hits;
    /** Number of tracks only with VXD */
    TH1F* m_TracksVXD;
    /** Number of tracks only with CDC */
    TH1F* m_TracksCDC;
    /** Number of full tracks with VXD+CDC */
    TH1F* m_TracksVXDCDC;
    /** Number of all finding tracks */
    TH1F* m_Tracks;

  };  //end class declaration

}  // end namespace Belle2


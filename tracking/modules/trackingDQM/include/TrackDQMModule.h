/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys                                              *
 *                                                                        *
 * Prepared for cluster shape correction quality check                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TrackDQMMODULE_H_
#define TrackDQMMODULE_H_

#undef DQM
#ifndef DQM
#include <framework/core/HistoModule.h>
#else
#include <daq/dqm/modules/DqmHistoManagerModule.h>
#endif

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
    void initialize() override;
    void beginRun() override;
    void event() override;
    void endRun() override;
    void terminate() override;

    /**
    * Histogram definitions such as TH1(), TH2(), TNtuple(), TTree().... are supposed
    * to be placed in this function.
    */
    void defineHisto() override;

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

#endif  // TrackDQMMODULE_H_


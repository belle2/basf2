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
#include <pxd/reconstruction/HitCorrector.h>
#include <pxd/geometry/SensorInfo.h>
#include <vector>
#include <TH2F.h>

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
    virtual ~TrackDQMModule();

    /** Module functions */
    virtual void initialize();
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    /**
    * Histogram definitions such as TH1(), TH2(), TNtuple(), TTree().... are supposed
    * to be placed in this function.
    */
    virtual void defineHisto();

  private:
    /** Name of the collection to use for the Reco Track */
    std::string m_storeRecoTrackName;
    /** Frames StoreArray name */
    std::string m_storeFramesName;

    /** Track momentum - X */
    TH1F* m_MomX;
    /** Track momentum - Y */
    TH1F* m_MomY;
    /** Track momentum - Z */
    TH1F* m_MomZ;
    /** Track momentum pT */
    TH1F* m_Mom;
    /** Number of hits on PXD */
    TH1F* m_HitsPXD;
    /** Number of hits on VXD */
    TH1F* m_HitsSVD;
    /** Number of hits on CDC */
    TH1F* m_HitsCDC;
    /** Number of hits in tracks */
    TH1F* m_Hits;
    /** Number of tracks with VXD */
    TH1F* m_TracksVXD;
    /** Number of tracks with CDC */
    TH1F* m_TracksCDC;
    /** Number of tracks with VXD+CDC */
    TH1F* m_TracksVXDCDC;
    /** Number of finding tracks */
    TH1F* m_Tracks;

  };  //end class declaration

}  // end namespace Belle2

#endif  // TrackDQMMODULE_H_


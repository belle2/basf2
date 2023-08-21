/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dataobjects/KLMTrack.h>
#include <klm/dataobjects/KLMHit2d.h>
#include <klm/geometry/KLMGeometryPar.h>

/* Basf2 headers. */
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <tracking/dataobjects/RecoTrack.h>

/* ROOT headers. */
#include <TEfficiency.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>

namespace Belle2 {

  //! This module perform straight line track finding and fitting for BKLM/EKLM
  class KLMTrackingModule: public Module {

  public:

    //! Constructor
    KLMTrackingModule();

    //! Destructor
    ~KLMTrackingModule();

    //! Initialize at start of job
    void initialize() override;

    //! begin run stuff
    void beginRun() override;

    //! Unpack one event and create digits
    void event() override;

    //! end run stuff
    void endRun() override;

    //! Terminate at the end of job
    void terminate() override;

    //! Judge if two hits come from the same sector
    bool sameSector(KLMHit2d* hit1, KLMHit2d* hit2);

    //! find the closest RecoTrack, match KLMTrack to RecoTrack, if the matched RecoTrack is found, return true
    bool findClosestRecoTrack(KLMTrack* klmTrk, RecoTrack*& closestTrack);


  protected:

    //! option for efficieny study mode, in this mode, the layer under study should not be used in tracking
    bool m_studyEffi;

    //! whether match KLMTrack to RecoTrack
    bool m_MatchToRecoTrack;

    //! angle required between RecoTrack and KLMTrack, if openangle is larger than m_maxAngleRequired, they don't match
    double m_maxAngleRequired = 10;

    //! maximum distance required between track and KLMHit2d to be accepted for efficiency calculation
    double m_maxDistance = 10;

    //! maximum sigma for hit acceptance during efficiency calculation
    double m_maxSigma = 5;

    //! minimum number of hits in sector for track finder to run (-2 from initial seed)
    unsigned int m_minHitList = 2;

    //! max number of hits in sector for track finder to run
    unsigned int m_maxHitList = 60;

    //! output file name containing efficiencies plots
    std::string m_outPath = "standaloneKLMEffi.root";

  private:

    //! KLMGeometryPar to call on B/E-KLM
    Belle2::KLM::KLMGeometryPar* m_GeoPar;

    //! TFile that store efficieny plots
    TFile* m_file = nullptr;

    //! Denominator of each layer
    TH1F* m_total[2][8];

    //! Numerator of each layer
    TH1F* m_pass[2][8];

    //! Efficieny of each layer
    TEfficiency* m_effiVsLayer[2][8];

    //! Efficieny at global position Y vs X
    //TEfficiency* m_effiYX;
    TH2F* m_effiYX;

    //! Efficieny at global position Y vs Z
    //TEfficiency* m_effiYZ;
    TH2F* m_effiYZ;

    //! passed event at global position Y vs X
    TH2F* m_passYX;

    //! total event at global position Y vs X
    TH2F* m_totalYX;

    //! passed event at global position Y vs Z
    TH2F* m_passYZ;

    //! total event at global position Y vs Z
    TH2F* m_totalYZ;
    //! KLMTrack StoreArray
    StoreArray<KLMTrack> m_storeTracks;

    //! KLMHit2d StoreArray
    StoreArray<KLMHit2d> hits2D;

    //! RecoTrack StoreArray
    StoreArray<RecoTrack> recoTracks;

    //! RecoHitInformation StoreArray
    StoreArray<RecoHitInformation> recoHitInformation;

    //! run the track finding and fitting
    void runTracking(int mode, int iSubdetector, int section, int sector, int layer);

    //! calculate efficiency
    void generateEffi(int iSubdetector, int section, int sector, int layer);

    //! my defined sort function using layer number
    static bool sortByLayer(KLMHit2d* hit1, KLMHit2d* hit2);

    //! judge whether the current layer is understudy
    bool isLayerUnderStudy(int section, int iSector, int iLayer, KLMHit2d* hit);

    //! judge whether the hits come from the sctor under study
    bool isSectorUnderStudy(int section, int iSector, KLMHit2d* hit);

    //! calculate distance from track to hit
    double distanceToHit(KLMTrack* track, KLMHit2d* hit,
                         double& error,
                         double& sigma);

    //! run number
    std::vector<int> m_runNumber;

    //! total number of processed events in the run
    int m_runTotalEvents;

    //! total number of processed events
    std::vector<int> m_totalEvents;

    //! total number of processed events in the run with at lease one BKLMTrack
    int m_runTotalEventsWithTracks;

    //! total number of processed events with at least one BKLMTrack
    std::vector<int> m_totalEventsWithTracks;
  };
} // end namespace Belle2

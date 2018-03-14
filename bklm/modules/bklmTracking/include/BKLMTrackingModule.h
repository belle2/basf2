/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yinghui GUAN                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef  BKLMTRACKING_H
#define  BKLMTRACKING_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <bklm/dataobjects/BKLMHit2d.h>
#include <bklm/dataobjects/BKLMTrack.h>
#include <bklm/modules/bklmTracking/BKLMTrackFinder.h>
#include <bklm/geometry/Module.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <bklm/geometry/GeometryPar.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TEfficiency.h>

namespace Belle2 {

  //! This module perform straight line track finding and fitting for BKLM
  class BKLMTrackingModule: public Module {

  public:

    //! Constructor
    BKLMTrackingModule();

    //! Destructor
    virtual ~BKLMTrackingModule();

    //! Initialize at start of job
    virtual void initialize();

    //! begin run stuff
    virtual void beginRun();

    //! Unpack one event and create digits
    virtual void event();

    //! end run stuff
    virtual void endRun();

    //! Terminate at the end of job
    virtual void terminate();

    //! Judge if two hits come from the same sector
    bool sameSector(BKLMHit2d* hit1, BKLMHit2d* hit2);

    //! find the closest RecoTrack, match BKLMTrack to RecoTrack, if the matched RecoTrack is found, return true
    bool findClosestRecoTrack(BKLMTrack* bklmTrk, RecoTrack*& closestTrack);


  protected:

    //! option for efficieny study mode, in this mode, the layer under study should not be used in tracking
    bool m_studyEffi;

    //! whether match BKLMTrack to RecoTrack
    bool m_MatchToRecoTrack;

    //! angle required between RecoTrack and BKLMTrack, if openangle is larger than m_maxAngleRequired, they don't match
    double m_maxAngleRequired = 10;

    //! do the BKLMTrack fitting in global system (multi-sectors track) or local system (sector by sector)
    bool m_globalFit;

    //! output file name containing efficiencies plots
    std::string m_outPath = "bklmEffi.root";

  private:

    //! bklm GeometryPar
    bklm::GeometryPar*   m_GeoPar = NULL;

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
    //! BKLMTrack StoreArray
    StoreArray<BKLMTrack> m_storeTracks;

    //! BKLMHit2d StoreArray
    StoreArray<BKLMHit2d> hits2D;

    //! RecoTrack StoreArray
    StoreArray<RecoTrack> recoTracks;

    //! RecoHitInformation StoreArray
    StoreArray<RecoHitInformation> recoHitInformation;

    //! run the track finding and fitting
    void runTracking(int mode, int isforward, int sector, int layer);

    //! calculate efficiency
    void generateEffi(int isforward, int sector, int layer);

    //! my defined sort function using layer number
    static bool sortByLayer(BKLMHit2d* hit1, BKLMHit2d* hit2);

    //! judge whether the current layer is understudy
    bool isLayerUnderStudy(int isForward, int iSector, int iLayer, BKLMHit2d* hit);

    //! judge whether the hits come from the sctor understudy
    bool isSectorUnderStudy(int isForward, int iSector, BKLMHit2d* hit);

    //! calculate distance from track to hit
    double distanceToHit(BKLMTrack* track, BKLMHit2d* hit,
                         double& error,
                         double& sigma);
  };
} // end namespace Belle2
#endif

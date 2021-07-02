/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Vitaliy Popov, Dmytro Minchenko                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/bklm/geometry/GeometryPar.h>
#include <klm/dataobjects/eklm/EKLMElementNumbers.h>
#include <klm/dataobjects/KLMDigit.h>
#include <klm/dataobjects/KLMElementNumbers.h>
#include <klm/dataobjects/KLMPlaneArrayIndex.h>
#include <klm/dbobjects/KLMChannelStatus.h>

/* Belle 2 headers. */
#include <analysis/dataobjects/ParticleList.h>
#include <calibration/CalibrationCollectorModule.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>

/* ROOT headers. */
#include <TFile.h>
#include <TH1F.h>
#include <TTree.h>

/* C++ headers. */
#include <map>
#include <string>

namespace Belle2 {

  /**
   * Module KLMStripEfficiencyCollectorModule.
   * @details
   * Module for collecting data for track matching efficiency.
   */

  class KLMStripEfficiencyCollectorModule : public CalibrationCollectorModule {

  private:

    /**
     * Hit data.
     */
    struct HitData {

      /** Subdetector. */
      int subdetector;

      /** Section. */
      int section;

      /** Layer. */
      int layer;

      /** Sector. */
      int sector;

      /** Plane. */
      int plane;

      /** Strip. */
      int strip;

      /** Local coordinate. */
      double localPosition;

      /** Extrapolation hit. */
      const ExtHit* hit;

      /** Digit. */
      const KLMDigit* digit;

    };

  public:

    /**
     * Constructor.
     */
    KLMStripEfficiencyCollectorModule();

    /**
     * Destructor.
     */
    ~KLMStripEfficiencyCollectorModule();

    /**
     * Initializer.
     */
    void prepare() override;

    /**
     * Finish data processing.
     */
    void finish() override;

    /**
     * This method is called for each event.
     */
    void collect() override;

    /**
     * This method is called at the beginning of the run.
     */
    void startRun() override;

    /**
     * This method is called at the end of run.
     */
    void closeRun() override;

  private:

    /**
     * Add hit to map.
     * @param[in] hitMap      Hit map.
     * @param[in] planeGlobal Plane global number.
     * @param[in] hitData     Hit data.
     */
    void addHit(std::map<KLMPlaneNumber, struct HitData>& hitMap,
                KLMPlaneNumber planeGlobal, struct HitData* hitData);

    /**
     * Find matching digit.
     * @param[in] hitData Hit data.
     */
    void findMatchingDigit(struct HitData* hitData);

    /**
     * Collect the data for one muon.
     * @param[in] muon                 Muon.
     * @param[in] matchedDigitsInPlane Matched digits.
     * @param[in] allExtHitsInPlane    Number of ExtHits.
     * @return True if the muon satisfies the selection criteria.
     */
    bool collectDataTrack(const Particle* muon, TH1F* matchedDigitsInPlane,
                          TH1F* allExtHitsInPlane);

    /** Muon list name. */
    std::string m_MuonListName;

    /**
     * Maximal distance in the units of strip number from ExtHit to
     * matching KLMDigit.
     */
    double m_AllowedDistance1D;

    /** Minimal number of matching digits. */
    int m_MinimalMatchingDigits;

    /** Minimal number of matching digits in outer layers. */
    int m_MinimalMatchingDigitsOuterLayers;

    /** Minimal momentum in case there are no hits in outer layers. */
    double m_MinimalMomentumNoOuterLayers;

    /** Whether to remove unused muons. */
    bool m_RemoveUnusedMuons;

    /** Whether to ignore ExtHits with backward propagation. */
    bool m_IgnoreBackwardPropagation;

    /** Channel status. */
    DBObjPtr<KLMChannelStatus> m_ChannelStatus;

    /** KLM digits. */
    StoreArray<KLMDigit> m_Digits;

    /** Tracks. */
    StoreArray<Track> m_tracks;

    /** ExtHits. */
    StoreArray<ExtHit> m_extHits;

    /** Muons. */
    StoreObjPtr<ParticleList> m_MuonList;

    /** Element numbers. */
    const KLMElementNumbers* m_ElementNumbers;

    /** EKLM element numbers. */
    const EKLMElementNumbers* m_eklmElementNumbers;

    /** BKLM geometry. */
    const bklm::GeometryPar* m_GeometryBKLM;

    /** Plane array index. */
    const KLMPlaneArrayIndex* m_PlaneArrayIndex;

    /** Debug mode. */
    bool m_Debug;

    /** Matching data file name */
    std::string m_MatchingFileName;

    /** Matching data file. */
    TFile* m_MatchingFile;

    /** Matching data tree. */
    TTree* m_MatchingTree;

    /** Matching hit data. */
    struct HitData m_MatchingHitData;

    /** Matched strip. */
    int m_MatchedStrip;

  };

}

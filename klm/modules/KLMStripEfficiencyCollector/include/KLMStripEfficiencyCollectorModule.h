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
#include <klm/bklm/dataobjects/BKLMDigit.h>
#include <klm/bklm/geometry/GeometryPar.h>
#include <klm/dataobjects/KLMElementNumbers.h>
#include <klm/dataobjects/KLMPlaneArrayIndex.h>
#include <klm/dbobjects/KLMChannelStatus.h>
#include <klm/eklm/dataobjects/EKLMDigit.h>
#include <klm/eklm/dataobjects/ElementNumbersSingleton.h>
#include <klm/eklm/geometry/GeometryData.h>

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

      /** EKLM digit. */
      const EKLMDigit* eklmDigit;

      /** BKLM digit. */
      const BKLMDigit* bklmDigit;

    };

  public:

    /**
     * Constructor.
     */
    KLMStripEfficiencyCollectorModule();

    /**
     * Destructor.
     */
    virtual ~KLMStripEfficiencyCollectorModule();

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
     * @param[in] hitMap  Hit map.
     * @param[in] plane   Plane number.
     * @param[in] hitData Hit data.
     */
    void addHit(std::map<uint16_t, struct HitData>& hitMap,
                uint16_t planeGlobal, struct HitData* hitData);

    /**
     * Find matching digit.
     * @param[in] hitData Hit data.
     */
    void findMatchingDigit(struct HitData* hitData);

    /**
     * Collect the data for one muon.
     * @param[in] muon Muon.
     * @return True if the muon satisfies the selection criteria.
     */
    bool collectDataTrack(const Particle* muon);

    /** Muon list name. */
    std::string m_MuonListName;

    /**
     * Maximal distance in the units of strip number from ExtHit to
     * matching (B|E)KLMDigit.
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

    /** Channel status. */
    DBObjPtr<KLMChannelStatus> m_ChannelStatus;

    /** EKLM digits. */
    StoreArray<EKLMDigit> m_EklmDigits;

    /** BKLM digits. */
    StoreArray<BKLMDigit> m_BklmDigits;

    /** Tracks. */
    StoreArray<Track> m_tracks;

    /** ExtHits. */
    StoreArray<ExtHit> m_extHits;

    /** Muons. */
    StoreObjPtr<ParticleList> m_MuonList;

    /** Element numbers. */
    const KLMElementNumbers* m_ElementNumbers;

    /** EKLM element numbers. */
    const EKLM::ElementNumbersSingleton* m_ElementNumbersEKLM;

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

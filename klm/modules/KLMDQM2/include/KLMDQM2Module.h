/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/bklm/geometry/GeometryPar.h>
#include <klm/dataobjects/bklm/BKLMElementNumbers.h>
#include <klm/dataobjects/eklm/EKLMElementNumbers.h>
#include <klm/dataobjects/KLMDigit.h>
#include <klm/dataobjects/KLMElementNumbers.h>
#include <klm/dataobjects/KLMPlaneArrayIndex.h>
#include <klm/dbobjects/KLMChannelStatus.h>

/* Basf2 headers. */
#include <analysis/dataobjects/ParticleList.h>
#include <framework/core/HistoModule.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/SoftwareTriggerResult.h>
#include <tracking/dataobjects/ExtHit.h>

/* ROOT headers. */
#include <TH1F.h>

/* C++ headers. */
#include <map>
#include <string>


namespace Belle2 {
  /**
  * Additional Module for KLMDQM plots after HLT filters
  *
  * An additional module developed to display plane efficiencies for the KLM dduring runs (i.e. for online analyses).
  * This module would be called after HLT filter in order to use mumu-tight skim to select reasonable events.
  * The output histograms would be plane efficiences = MatchedDigits/AllExtits.

  */
  class KLMDQM2Module : public HistoModule {

  public:

    /** Hit data. */
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

    /**
    * Constructor: Sets the description, the properties and the parameters of the module.
    */
    KLMDQM2Module();

    /**
    * Destructor
    */
    ~KLMDQM2Module();

    /**
     * Definition of the histograms.
     * Required part of HistoModule. Done in initialize()
     * framework/core/HistoModule
     */
    void defineHisto() override;

    /** Register input and output data */
    virtual void initialize() override;

    /** Called when entering a new run */
    virtual void beginRun() override;

    /** Selection for mumu_tight_skim, then DQM plot filling  */
    virtual void event() override;

    /** Called if the current run ends */
    virtual void endRun() override;

    /** Called at the end of the event processing */
    virtual void terminate() override;



  private:

    /*******************************************/
    /*******************************************/
    //PRE-DEFINED FUNCTIONS
    /*******************************************/
    /*******************************************/


    /**
     * Find matching digit.
     * @param[in] hitData Hit data.
     */
    void findMatchingDigit(struct HitData* hitData);

    /**
     * Collect the data for one muon.
     * @param[in] muon                 Muon.
     * @param[in] matchedHitsBKLM      Matched digits in BKLM
     * @param[in] allExtHitsBKLM       Number of ExtHits in BKLM
     * @param[in] matchedHitsEKLM      Matched digits in EKLM
     * @param[in] allExtHitsEKLM       Number of ExtHits in EKLM
     * @param[in] matchedHitsBKLMSec   Matched digits in BKLM per Sector
     * @param[in] allExtHitsBKLMSec    Number of ExtHits per Sector
     * @param[in] matchedHitsEKLMSec   Matched digits in EKLM per Sector
     * @param[in] allExtHitsEKLMSec    Number of ExtHits in EKLM per Sector
     * @return True if the muon satisfies the selection criteria.
     */
    bool collectDataTrack(const Particle* muon, TH1F* matchedHitsBKLM,
                          TH1F* allExtHitsBKLM, TH1F* matchedHitsEKLM,
                          TH1F* allExtHitsEKLM, TH1F* matchedHitsBKLMSec,
                          TH1F* allExtHitsBKLMSec, TH1F* matchedHitsEKLMSec,
                          TH1F* allExtHitsEKLMSec);

    /**
     * Add hit to map.
     * @param[in] hitMap      Hit map.
     * @param[in] planeGlobal Plane global number.
     * @param[in] hitData     Hit data.
     */
    void addHit(std::map<KLMPlaneNumber, struct HitData>& hitMap,
                KLMPlaneNumber planeGlobal, struct HitData* hitData);

    /**
     * Uses TrigResult along with desired software cut
     * to determine whether histograms are filled or not
     * for a given event.
     */
    bool triggerFlag();

    /*******************************************/
    /*******************************************/
    //INPUT RELATED
    /*******************************************/
    /*******************************************/

    /** KLM element numbers. */
    const KLMElementNumbers* m_ElementNumbers;

    /** EKLM element numbers. */
    const EKLMElementNumbers* m_eklmElementNumbers;

    /** Plane array index. */
    const KLMPlaneArrayIndex* m_PlaneArrayIndex;

    /** BKLM geometry. */
    const bklm::GeometryPar* m_GeometryBKLM;

    /** KLM digits. */
    StoreArray<KLMDigit> m_Digits;

    /** Trigger Information */
    StoreObjPtr<SoftwareTriggerResult> m_softwareTriggerResult;

    /** ExtHits. */
    StoreArray<ExtHit> m_extHits;

    /** Muons. */
    StoreObjPtr<ParticleList> m_MuonList;

    /** Channel status. */
    DBObjPtr<KLMChannelStatus> m_ChannelStatus;

    /*******************************************/
    /*******************************************/
    //MODULE PARAMETERS
    /*******************************************/
    /*******************************************/

    /** Muon list name. */
    std::string m_MuonListName;

    /** Maximal distance in the units of strip number
     * from ExtHit to matching KLMDigit */
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

    /** Directory for KLM DQM histograms in ROOT file. */
    std::string m_HistogramDirectoryName;

    /** Software Trigger Name.  */
    std::string m_SoftwareTriggerName;

    /*******************************************/
    /*******************************************/
    //OUTPUT RELATED
    /*******************************************/
    /*******************************************/

    /** Matched hits in plane for BKLM */
    TH1F* m_MatchedHitsBKLM;

    /** Extrapolated hits in plane for BKLM */
    TH1F* m_AllExtHitsBKLM;

    /** Matched hits in plane for EKLM */
    TH1F* m_MatchedHitsEKLM;

    /** Extrapolated hits in plane for EKLM */
    TH1F* m_AllExtHitsEKLM;

    /** Matched hits in sector for BKLM */
    TH1F* m_MatchedHitsBKLMSector;

    /** Extrapolated hits in sector for BKLM */
    TH1F* m_AllExtHitsBKLMSector;

    /** Matched hits in sector for EKLM */
    TH1F* m_MatchedHitsEKLMSector;

    /** Extrapolated hits in sector for EKLM */
    TH1F* m_AllExtHitsEKLMSector;


    /*******************************************/
    /*******************************************/
    //OTHER USEFUL VARIABLES
    /*******************************************/
    /*******************************************/

    /** Number of layers/planes for BKLM. */
    const int m_PlaneNumBKLM = BKLMElementNumbers::getMaximalLayerGlobalNumber(); // 15 layers per octant, forward and backward

    /** Number of layers/planes for EKLM. */
    const int m_PlaneNumEKLM = EKLMElementNumbers::getMaximalPlaneGlobalNumber(); // 12 or 14 layers per quadrant, forward and backward

  };
}

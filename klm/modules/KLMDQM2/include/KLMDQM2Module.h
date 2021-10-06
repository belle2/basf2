/**************************************************************************
    * BASF2 (Belle Analysis Framework 2)                                     *
    * Copyright(C) 2021 - Belle II Collaboration                             *
    *                                                                        *
    * Author: The Belle II Collaboration                                     *
    * Contributors: Tommy Lam                                                *
    *                                                                        *
    * This software is provided "as is" without any warranty.                *
    **************************************************************************/

#pragma once
/* KLM headers. */
#include <klm/dataobjects/bklm/BKLMElementNumbers.h>
#include <klm/dataobjects/eklm/EKLMElementNumbers.h>
#include <klm/dataobjects/KLMDigit.h>
#include <klm/dataobjects/KLMChannelArrayIndex.h>
#include <klm/dataobjects/KLMPlaneArrayIndex.h>
#include <klm/dataobjects/KLMElementNumbers.h>
#include <klm/dataobjects/KLMSectorArrayIndex.h>
#include <klm/bklm/geometry/GeometryPar.h>
#include <klm/dbobjects/KLMChannelStatus.h>
//#include <klm/dataobjects/KLMElementNumberDefinitions.h>

/* Belle 2 headers. */
#include <analysis/dataobjects/ParticleList.h>
#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/database/DBObjPtr.h>
#include <rawdata/dataobjects/RawFTSW.h>
#include <rawdata/dataobjects/RawKLM.h>
#include <mdst/dataobjects/SoftwareTriggerResult.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>

/* ROOT headers. */
#include <TH1F.h>
#include <TH2F.h>
#include <TLine.h>
#include <TText.h>
#include <TTree.h>
#include <TFile.h>

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

    /** Test Description */
    virtual void beginRun() override;

    /** selection for mumu_tight_skim, then DQM plot filling  */
    virtual void event() override;

    /** empty */
    virtual void endRun() override;

    /** empty */
    virtual void terminate() override;



  private:

    /*******************************************/
    /*******************************************/
    //INPUT RELATED
    /*******************************************/
    /*******************************************/


    /** KLM element numbers. */
    const KLMElementNumbers* m_ElementNumbers;

    /** Plane array index. */
    const KLMPlaneArrayIndex* m_PlaneArrayIndex;

    /** Element numbers. */
    const EKLMElementNumbers* m_eklmElementNumbers;

    /** BKLM geometry. */
    const bklm::GeometryPar* m_GeometryBKLM;


    /** Raw FTSW. */
    StoreArray<RawFTSW> m_RawFtsws;

    /** Raw KLM. */
    StoreArray<RawKLM> m_RawKlms;

    /** KLM digits. */
    StoreArray<KLMDigit> m_Digits;


    /** Trigger Information */
    StoreObjPtr<SoftwareTriggerResult> trigResult;

    /** ExtHits. */
    StoreArray<ExtHit> m_extHits;

    /** Muons. */
    StoreObjPtr<ParticleList> m_MuonList;

    /** Debug mode. */
    bool m_Debug;

    /*******************************************/
    /*******************************************/
    //MODULE PARAMETERS
    /*******************************************/
    /*******************************************/


    /**< Minimal number of hits in a channel required
     * to flag it as 'masked' or 'hot' */
    int m_MinHitsForFlagging;


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


    /*******************************************/
    /*******************************************/
    //ROOT AESTHETIC RELATED
    /*******************************************/
    /*******************************************/

    /** TLine for background region in 2d hits histograms. */
    TLine m_2DHitsLine;

    /** TLine for boundary in plane histograms. */
    TLine m_PlaneLine;

    /** TText for names in plane histograms. */
    TText m_PlaneText;


    /*******************************************/
    /*******************************************/
    //OUTPUT RELATED
    /*******************************************/
    /*******************************************/

    /** Matched hits in plane for BKLM */
    TH1F* m_MatchedHitsBKLM;

    /** Extrapolated hits in plane for BKLM */
    TH1F* m_AllExtHitsBKLM;

    /** Matched over Extrapolated hits in plane for BKLM */
    TH1F* m_PlaneEfficienciesBKLM;

    /** Matched hits in plane for EKLM */
    TH1F* m_MatchedHitsEKLM;

    /** Extrapolated hits in plane for EKLM */
    TH1F* m_AllExtHitsEKLM;

    /** Matched over Extrapolated hits in plane for EKLM */
    TH1F* m_PlaneEfficienciesEKLM;


    /** Matched hits in sector for BKLM */
    TH1F* m_MatchedHitsBKLMSector;

    /** Extrapolated hits in sector for BKLM */
    TH1F* m_AllExtHitsBKLMSector;

    /** Matched over Extrapolated hits in sector for BKLM */
    TH1F* m_PlaneEfficienciesBKLMSector;

    /** Matched hits in sector for EKLM */
    TH1F* m_MatchedHitsEKLMSector;

    /** Extrapolated hits in sector for EKLM */
    TH1F* m_AllExtHitsEKLMSector;

    /** Matched over Extrapolated hits in sector for EKLM */
    TH1F* m_PlaneEfficienciesEKLMSector;

    /** Number of hits per channel. */
    TH1F** m_MatchedHitsInPlane[
      EKLMElementNumbers::getMaximalSectorGlobalNumberKLMOrder() +
      BKLMElementNumbers::getMaximalSectorGlobalNumber()] = {nullptr};
    /** Number of hits per channel. */
    TH1F** m_AllExtHitsInPlane[
      EKLMElementNumbers::getMaximalSectorGlobalNumberKLMOrder() +
      BKLMElementNumbers::getMaximalSectorGlobalNumber()] = {nullptr};
    /** Number of hits per channel. */
    TH1F** m_PlaneEfficienciesSector[
      EKLMElementNumbers::getMaximalSectorGlobalNumberKLMOrder() +
      BKLMElementNumbers::getMaximalSectorGlobalNumber()] = {nullptr};

    /*******************************************/
    /*******************************************/
    //OTHER USEFUL VARIABLES
    /*******************************************/
    /*******************************************/


    /** Minimal number of processed events for error messages. */
    //is related to m_MinProcessedEventsForMessagesInput
    double m_MinProcessedEventsForMessages;

    /** Number of channel hit histograms per sector for BKLM. */
    const int m_ChannelHitHistogramsBKLM = 2;

    /** Number of channel hit histograms per sector for EKLM. */
    const int m_ChannelHitHistogramsEKLM = 3;


    /** Number of layers/planes for BKLM. */
    const int m_PlaneNumBKLM = 240; //15 layers per octant, forward and backward

    /** Number of layers/planes for EKLM. */
    //TODO: check if this is 13 or 14 layers per octant
    const int m_PlaneNumEKLM = 208;//12 or 14 layers per quadrant, forward and backward

    /** Matched strip. */
    int m_MatchedStrip;

    /** Channel status. */
    DBObjPtr<KLMChannelStatus> m_ChannelStatus;

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
     * @param[in] MatchedHitsBKLM      Matched digits in BKLM
     * @param[in] AllExtHitsBKLM       Number of ExtHits in BKLM
     * @param[in] MatchedHitsEKLM      Matched digits in EKLM
     * @param[in] AllExtHitsEKLM       Number of ExtHits in EKLM
     * @param[in] MatchedHitsBKLMSec   Matched digits in BKLM per Sector
     * @param[in] AllExtHitsBKLMSec    Number of ExtHits per Sector
     * @param[in] MatchedHitsEKLMSec   Matched digits in EKLM per Sector
     * @param[in] AllExtHitsEKLMSec    Number of ExtHits in EKLM per Sector
     * @return True if the muon satisfies the selection criteria.
     */
    bool collectDataTrack(const Particle* muon, TH1F* MatchedHitsBKLM,
                          TH1F* AllExtHitsBKLM, TH1F* MatchedHitsEKLM,
                          TH1F* AllExtHitsEKLM, TH1F* MatchedHitsBKLMSec,
                          TH1F* AllExtHitsBKLMSec, TH1F* MatchedHitsEKLMSec,
                          TH1F* AllExtHitsEKLMSec);

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
    //DEBUG RELATED
    /*******************************************/
    /** Matching data file name */
    std::string m_MatchingFileName;

    /** Matching data file. */
    TFile* m_MatchingFile;

    /** Matching data tree. */
    TTree* m_MatchingTree;

    /** Matching hit data. */
    struct HitData m_MatchingHitData;



    /*******************************************/
    //EFFICIENCY ALGORITHM FUNCTIONS
    /*******************************************/




  };
}

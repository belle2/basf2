/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sara Neuhaus                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/HistoModule.h>

#include <framework/datastore/StoreArray.h>
#include <trg/cdc/dataobjects/CDCTriggerTrack.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>
#include <trg/cdc/dataobjects/CDCTriggerMLPInput.h>

#include "TH1F.h"

namespace Belle2 {

  /** CDC Trigger DQM Module */
  class CDCTriggerDQMModule : public HistoModule {  // <- derived from HistoModule class

  public:

    /** Constructor */
    CDCTriggerDQMModule();
    /** Destructor */
    virtual ~CDCTriggerDQMModule();

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

    /** Name of the histogram directory in ROOT file */
    std::string m_histogramDirectoryName;

    // names for unpacked objects
    /** CDCTriggerHitSegment StoreArray name for TS hits from unpacker */
    std::string m_unpackedSegmentHitsName;
    /** CDCTriggerTrack StoreArray name for 2D finder tracks from unpacker */
    std::string m_unpacked2DTracksName;
    /** CDCTriggerTrack StoreArray name for neuro tracks from unpacker */
    std::string m_unpackedNeuroTracksName;
    /** StoreArray name for neuro input vector from unpacker */
//    std::string m_unpackedNeuroInputName;
    /** StoreArray name for neuro input 2d finder tracks */
    std::string m_unpackedNeuroInput2DTracksName;
    /** StoreArray name for neuro input Track segments */
    std::string m_unpackedNeuroInputSegmentsName;
    // store arrays for direct access
    /** Storearray for TS hits from unpacker */
    StoreArray<CDCTriggerSegmentHit> m_unpackedSegmentHits;
    /** Storearray for 2D finder tracks from unpacker */
    StoreArray<CDCTriggerTrack> m_unpacked2DTracks;
    /** Storearray for neuro tracks from unpacker */
    StoreArray<CDCTriggerTrack> m_unpackedNeuroTracks;
    /** Storearray for neuro input vector from unpacker */
//    StoreArray<CDCTriggerMLPInput> m_unpackedNeuroInput;
    /** Store array for neuro input 2dfinder tracks */
    StoreArray<CDCTriggerTrack> m_unpackedNeuroInput2DTracks;
    /** StoreArray for neuro input Track segments */
    StoreArray<CDCTriggerSegmentHit> m_unpackedNeuroInputSegments;


    // histograms for neurotrigger
    TH1F* m_neuroOutZ;              /**< z distribution from unpacker */
    TH1F* m_neuroOutCosTheta;       /**< cos theta distribution from unpacker */
    TH1F* m_neuroOutHitPattern;     /**< number of hits related to unpacked track */
    TH1F* m_neuroOutPhi0;           /**< phi distribution from unpacker */
    TH1F* m_neuroOutInvPt;          /**< Inverse Pt distribution from unpacker */
    TH1F* m_neuroOutm_time;         /**< m_time distribution from unpacker */
    TH1F* m_neuroOutTrackCount;     /**< number of tracks per event */
    TH1F* m_neuroOutVsInTrackCount; /**< number of neuroOutTracks - number of 2dinTracks */
    TH1F* m_neuroInTSID;            /**< ID of incoming track segments */
    TH1F* m_neuroInTSPrioT_Layer0;  /**< Priority time of track segments in layer 0 */
    TH1F* m_neuroInTSPrioT_Layer1;  /**< Priority time of track segments in layer 1 */
    TH1F* m_neuroInTSPrioT_Layer2;  /**< Priority time of track segments in layer 2 */
    TH1F* m_neuroInTSPrioT_Layer3;  /**< Priority time of track segments in layer 3 */
    TH1F* m_neuroInTSPrioT_Layer4;  /**< Priority time of track segments in layer 4 */
    TH1F* m_neuroInTSPrioT_Layer5;  /**< Priority time of track segments in layer 5 */
    TH1F* m_neuroInTSPrioT_Layer6;  /**< Priority time of track segments in layer 6 */
    TH1F* m_neuroInTSPrioT_Layer7;  /**< Priority time of track segments in layer 7 */
    TH1F* m_neuroInTSPrioT_Layer8;  /**< Priority time of track segments in layer 8 */
    TH1F* m_neuroInTSFoundT_Layer0; /**< Found time of track segments in layer 0 */
    TH1F* m_neuroInTSFoundT_Layer1; /**< Found time of track segments in layer 1 */
    TH1F* m_neuroInTSFoundT_Layer2; /**< Found time of track segments in layer 2 */
    TH1F* m_neuroInTSFoundT_Layer3; /**< Found time of track segments in layer 3 */
    TH1F* m_neuroInTSFoundT_Layer4; /**< Found time of track segments in layer 4 */
    TH1F* m_neuroInTSFoundT_Layer5; /**< Found time of track segments in layer 5 */
    TH1F* m_neuroInTSFoundT_Layer6; /**< Found time of track segments in layer 6 */
    TH1F* m_neuroInTSFoundT_Layer7; /**< Found time of track segments in layer 7 */
    TH1F* m_neuroInTSFoundT_Layer8; /**< Found time of track segments in layer 8 */
    TH1F* m_neuroInVs2DOutTrackCount;    /**< number of tracks neuro input 2dtracks vs 2doutput tracks*/
    TH1F* m_neuroSelTSPrioT_Layer0;  /**< Priority time of selected track segments in layer 0 */
    TH1F* m_neuroSelTSPrioT_Layer1;  /**< Priority time of selected track segments in layer 1 */
    TH1F* m_neuroSelTSPrioT_Layer2;  /**< Priority time of selected track segments in layer 2 */
    TH1F* m_neuroSelTSPrioT_Layer3;  /**< Priority time of selected track segments in layer 3 */
    TH1F* m_neuroSelTSPrioT_Layer4;  /**< Priority time of selected track segments in layer 4 */
    TH1F* m_neuroSelTSPrioT_Layer5;  /**< Priority time of selected track segments in layer 5 */
    TH1F* m_neuroSelTSPrioT_Layer6;  /**< Priority time of selected track segments in layer 6 */
    TH1F* m_neuroSelTSPrioT_Layer7;  /**< Priority time of selected track segments in layer 7 */
    TH1F* m_neuroSelTSPrioT_Layer8;  /**< Priority time of selected track segments in layer 8 */
    TH1F* m_neuroSelTSFoundT_Layer0; /**< Found time of selected track segments in layer 0 */
    TH1F* m_neuroSelTSFoundT_Layer1; /**< Found time of selected track segments in layer 1 */
    TH1F* m_neuroSelTSFoundT_Layer2; /**< Found time of selected track segments in layer 2 */
    TH1F* m_neuroSelTSFoundT_Layer3; /**< Found time of selected track segments in layer 3 */
    TH1F* m_neuroSelTSFoundT_Layer4; /**< Found time of selected track segments in layer 4 */
    TH1F* m_neuroSelTSFoundT_Layer5; /**< Found time of selected track segments in layer 5 */
    TH1F* m_neuroSelTSFoundT_Layer6; /**< Found time of selected track segments in layer 6 */
    TH1F* m_neuroSelTSFoundT_Layer7; /**< Found time of selected track segments in layer 7 */
    TH1F* m_neuroSelTSFoundT_Layer8; /**< Found time of selected track segments in layer 8 */
    TH1F* m_neuroInTSCount;          /**< number of TS per track */
    TH1F* m_neuroInInvPt;            /**< Inverse Pt distribution from incoming 2dtrack */
    TH1F* m_neuroInPhi0;             /**< Phi0 of incoming 2dtrack */
    TH1F* m_neuroInm_time;           /**< m_time distribution from incoming 2dtracks */
    TH1F* m_neuroInTrackCount;       /**< number of neuro input 2dtracks per event */
    TH1F* m_2DOutInvPt;              /**< Inverse Pt of 2dtracks */
    TH1F* m_2DOutPhi0;               /**< Phi0 of 2dtracks */
    TH1F* m_2DOutm_time;             /**< m_time 2dtracks */
    TH1F* m_2DOutTrackCount;         /**< number of 2dtracks per event */
    TH1F* m_neuroSelTSCount;         /**< number of selected TS per SL */
    TH1F* m_neuroSelTSID;            /**< ID of selected track segments */

  };

}

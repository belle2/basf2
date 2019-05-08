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
#include <tracking/dataobjects/RecoTrack.h>

#include "TH1F.h"
#include "TH2F.h"

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
    std::string m_unpackedNeuroInputName;
    /** StoreArray name for neuro input 2d finder tracks */
    std::string m_unpackedNeuroInput2DTracksName;
    /** StoreArray name for neuro input Track segments */
    std::string m_unpackedNeuroInputSegmentsName;
    // names for simulated objects
    /** CDCTriggerTrack StoreArray name for neuro tracks from TSIM
     *  (from CDCTriggerNeuro module with unpacked TS hits and 2D tracks as input) */
    std::string m_simNeuroTracksName;
    /** StoreArray name for neuro input vector from TSIM */
    std::string m_simNeuroInputName;
    /** switch to turn on a comparison with the reconstruction */
    std::string m_showRecoTracks;

    // store arrays for direct access
    /** Storearray for TS hits from unpacker */
    StoreArray<CDCTriggerSegmentHit> m_unpackedSegmentHits;
    /** Storearray for 2D finder tracks from unpacker */
    StoreArray<CDCTriggerTrack> m_unpacked2DTracks;
    /** Storearray for neuro tracks from unpacker */
    StoreArray<CDCTriggerTrack> m_unpackedNeuroTracks;
    /** Storearray for neuro input vector from unpacker */
    StoreArray<CDCTriggerMLPInput> m_unpackedNeuroInput;
    /** Store array for neuro input 2dfinder tracks */
    StoreArray<CDCTriggerTrack> m_unpackedNeuroInput2DTracks;
    /** StoreArray for neuro input Track segments */
    StoreArray<CDCTriggerSegmentHit> m_unpackedNeuroInputSegments;
    /** Storearray for neuro tracks from TSIM */
    StoreArray<CDCTriggerTrack> m_simNeuroTracks;
    /** Storearray for neuro input vector from TSIM */
    StoreArray<CDCTriggerMLPInput> m_simNeuroInput;
    /** Storearray for RecoTracks */
    StoreArray<RecoTrack> m_RecoTracks;


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
    TH1F* m_2DInTSID;                /**< ID of 2D incoming axial track segments */
    TH1F* m_2DInTSPrioT_Layer0;      /**< Priority time of 2D track segments in layer 0 */
    TH1F* m_2DInTSPrioT_Layer2;      /**< Priority time of 2D track segments in layer 2 */
    TH1F* m_2DInTSPrioT_Layer4;      /**< Priority time of 2D track segments in layer 4 */
    TH1F* m_2DInTSPrioT_Layer6;      /**< Priority time of 2D track segments in layer 6 */
    TH1F* m_2DInTSPrioT_Layer8;      /**< Priority time of 2D track segments in layer 8 */
    TH1F* m_2DInTSFoundT_Layer0;     /**< Found time of 2D track segments in layer 0 */
    TH1F* m_2DInTSFoundT_Layer2;     /**< Found time of 2D track segments in layer 2 */
    TH1F* m_2DInTSFoundT_Layer4;     /**< Found time of 2D track segments in layer 4 */
    TH1F* m_2DInTSFoundT_Layer6;     /**< Found time of 2D track segments in layer 6 */
    TH1F* m_2DInTSFoundT_Layer8;     /**< Found time of 2D track segments in layer 8 */
    TH1F* m_2DInTSCount;             /**< number of 2D incoming TS per SL */
    TH1F* m_neuroSector;             /**< unpacked sector */
    TH1F* m_neuroDeltaZ;             /**< unpacked z - TSIM z */
    TH1F* m_neuroDeltaTheta;         /**< unpacked theta - TSIM theta */
    TH2F* m_neuroScatterZ;           /**< unpacked z vs TSIM z, scatter plot*/

    TH1F* m_neuroDeltaInputID;       /**< unpacked ID input - TSIM ID input */
    TH1F* m_neuroDeltaInputT;        /**< unpacked time input - TSIM time input */
    TH1F* m_neuroDeltaInputAlpha;    /**< unpacked alpha input - TSIM alpha input */
    TH1F* m_neuroDeltaTSID;          /**< unpacked selected TSID - TSIM selected TSID */
    TH1F* m_neuroDeltaSector;        /**< unpacked sector - TSIM sector */
    TH1F* m_simSameTS;               /**< number of TS selected in both, unpacked and TSIM tracks */
    TH1F* m_simDiffTS;               /**< number of TS selcted in TSIM but not in unpacked */
    TH1F* m_RecoZ;                   /**< reconstructed z */
    TH1F* m_RecoCosTheta;            /**< reconstructed cos(theta) */
    TH1F* m_RecoInvPt;               /**< reconstructed inverse Pt */
    TH1F* m_RecoPhi;                 /**< reconstructed phi */

    TH1F* m_RecoHWZ;                 /**< matched to HW reconstructed z */
    TH1F* m_RecoHWCosTheta;          /**< matched to HW reconstructed cos(theta) */
    TH1F* m_RecoHWInvPt;             /**< matched to HW reconstructed inverse Pt */
    TH1F* m_RecoHWPhi;               /**< matched to HW reconstructed phi */
    TH2F* m_RecoHWZScatter;          /**< matched to HW reconstructed z scatter plot*/

    TH1F* m_RecoSWZ;                 /**< matched to SW reconstructed z */
    TH1F* m_RecoSWCosTheta;          /**< matched to SW reconstructed cos(theta) */
    TH1F* m_RecoSWInvPt;             /**< matched to SW reconstructed inverse Pt */
    TH1F* m_RecoSWPhi;               /**< matched to SW reconstructed phi */
    TH2F* m_RecoSWZScatter;          /**< matched to SW reconstructed z scatter plot*/

    TH1F* m_DeltaRecoHWZ;            /**< matched to HW reconstructed z */
    TH1F* m_DeltaRecoHWCosTheta;     /**< matched to HW reconstructed cos(theta) */
    TH1F* m_DeltaRecoHWInvPt;        /**< matched to HW reconstructed inverse Pt */
    TH1F* m_DeltaRecoHWPhi;          /**< matched to HW reconstructed phi */

    TH1F* m_DeltaRecoSWZ;            /**< matched to SW reconstructed z */
    TH1F* m_DeltaRecoSWCosTheta;     /**< matched to SW reconstructed cos(theta) */
    TH1F* m_DeltaRecoSWInvPt;        /**< matched to SW reconstructed inverse Pt */
    TH1F* m_DeltaRecoSWPhi;          /**< matched to SW reconstructed phi */

    TH1F* m_neuroOutQuad5Z;              /**< z distribution from unpacker */
    TH1F* m_neuroOutQuad5CosTheta;       /**< cos theta distribution from unpacker */
    TH1F* m_neuroOutQuad5Phi0;           /**< phi distribution from unpacker */
    TH1F* m_neuroOutQuad5InvPt;          /**< Inverse Pt distribution from unpacker */

    TH1F* m_neuroOutQuad0Z;              /**< z distribution from unpacker */
    TH1F* m_neuroOutQuad0CosTheta;       /**< cos theta distribution from unpacker */
    TH1F* m_neuroOutQuad0Phi0;           /**< phi distribution from unpacker */
    TH1F* m_neuroOutQuad0InvPt;          /**< Inverse Pt distribution from unpacker */
    TH1F* m_neuroOutQuad1Z;              /**< z distribution from unpacker */
    TH1F* m_neuroOutQuad1CosTheta;       /**< cos theta distribution from unpacker */
    TH1F* m_neuroOutQuad1Phi0;           /**< phi distribution from unpacker */
    TH1F* m_neuroOutQuad1InvPt;          /**< Inverse Pt distribution from unpacker */
    TH1F* m_neuroOutQuad2Z;              /**< z distribution from unpacker */
    TH1F* m_neuroOutQuad2CosTheta;       /**< cos theta distribution from unpacker */
    TH1F* m_neuroOutQuad2Phi0;           /**< phi distribution from unpacker */
    TH1F* m_neuroOutQuad2InvPt;          /**< Inverse Pt distribution from unpacker */
    TH1F* m_neuroOutQuad3Z;              /**< z distribution from unpacker */
    TH1F* m_neuroOutQuad3CosTheta;       /**< cos theta distribution from unpacker */
    TH1F* m_neuroOutQuad3Phi0;           /**< phi distribution from unpacker */
    TH1F* m_neuroOutQuad3InvPt;          /**< Inverse Pt distribution from unpacker */

    TH1F* m_neuroOutSWZ;              /**< z distribution from simulation */
    TH1F* m_neuroOutSWCosTheta;       /**< cos theta distribution from simulation */
    TH1F* m_neuroOutSWPhi0;           /**< phi distribution from simulation */
    TH1F* m_neuroOutSWInvPt;          /**< Inverse Pt distribution from simulation */
  };

}

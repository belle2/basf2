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
    std::string m_unpackedNeuroInputVectorName;
    /** StoreArray name for neuro input 2d finder tracks */
    std::string m_unpackedNeuroInput2DTracksName;
    /** StoreArray name for neuro input Track segments */
    std::string m_unpackedNeuroInputSegmentsName;
    // names for simulated objects
    /** CDCTriggerTrack StoreArray name for neuro tracks from TSIM
     *  (from CDCTriggerNeuro module with unpacked TS hits and 2D tracks as input) */
    std::string m_simNeuroTracksName;
    /** StoreArray name for simulated neuro input vector using HW TS, HW 2D*/
    std::string m_simNeuroInputVectorName;
    /** switch to turn on a comparison with the reconstruction */
    std::string m_showRecoTracks;

    /** StoreArray name for simulated TS hits */
    std::string m_simSegmentHitsName;
    /** StoreArray name for simulated 2D finder tracks using simulated TS */
    std::string m_sim2DTracksSWTSName;
    /** StoreArray name for neuro tracks using simulated TS and simulated 2D */
    std::string m_simNeuroTracksSWTSSW2DName;
    /** StoreArray name for neuro input vector using simulated TS and simulated 2D */
    std::string m_simNeuroInputVectorSWTSSW2DName;

    // store arrays for direct access
    /** Storearray for TS hits from unpacker */
    StoreArray<CDCTriggerSegmentHit> m_unpackedSegmentHits;
    /** Storearray for 2D finder tracks from unpacker */
    StoreArray<CDCTriggerTrack> m_unpacked2DTracks;
    /** Storearray for neuro tracks from unpacker */
    StoreArray<CDCTriggerTrack> m_unpackedNeuroTracks;
    /** Storearray for neuro input vector from unpacker */
    StoreArray<CDCTriggerMLPInput> m_unpackedNeuroInputVector;
    /** Store array for neuro input 2dfinder tracks */
    StoreArray<CDCTriggerTrack> m_unpackedNeuroInput2DTracks;
    /** StoreArray for neuro input Track segments */
    StoreArray<CDCTriggerSegmentHit> m_unpackedNeuroInputSegments;
    /** Storearray for neuro tracks from TSIM */
    StoreArray<CDCTriggerTrack> m_simNeuroTracks;
    /** Storearray for neuro input vector from TSIM */
    StoreArray<CDCTriggerMLPInput> m_simNeuroInputVector;
    /** Storearray for RecoTracks */
    StoreArray<RecoTrack> m_RecoTracks;

    /** Storearray for simulated TS hits */
    StoreArray<CDCTriggerSegmentHit> m_simSegmentHits;
    /** Storearray for simulated 2D finder tracks using simulated TS*/
    StoreArray<CDCTriggerTrack> m_sim2DTracksSWTS;
    /** Storearray for neuro input vector using simulated TS and simulated 2D */
    StoreArray<CDCTriggerMLPInput> m_simNeuroInputVectorSWTSSW2D;
    /** Storearray for neuro tracks using simulated TS and simulated 2D */
    StoreArray<CDCTriggerTrack> m_simNeuroTracksSWTSSW2D;

    // histograms for neurotrigger
    TH1F* m_neuroHWOutZ;              /**< z distribution from unpacker */
    TH1F* m_neuroHWOutCosTheta;       /**< cos theta distribution from unpacker */
    TH1F* m_neuroHWOutHitPattern;     /**< number of hits related to unpacked track */
    TH1F* m_neuroHWOutPhi0;           /**< phi distribution from unpacker */
    TH1F* m_neuroHWOutInvPt;          /**< Inverse Pt distribution from unpacker */
    TH1F* m_neuroHWOutm_time;         /**< m_time distribution from unpacker */
    TH1F* m_neuroHWOutTrackCount;     /**< number of tracks per event */
    TH1F* m_neuroHWOutVsInTrackCount; /**< number of neuroOutTracks - number of 2dinTracks */

    TH1F* m_neuroHWInTSID;            /**< ID of incoming track segments */
    TH1F* m_neuroHWInTSPrioT_Layer0;  /**< Priority time of track segments in layer 0 */
    TH1F* m_neuroHWInTSPrioT_Layer1;  /**< Priority time of track segments in layer 1 */
    TH1F* m_neuroHWInTSPrioT_Layer2;  /**< Priority time of track segments in layer 2 */
    TH1F* m_neuroHWInTSPrioT_Layer3;  /**< Priority time of track segments in layer 3 */
    TH1F* m_neuroHWInTSPrioT_Layer4;  /**< Priority time of track segments in layer 4 */
    TH1F* m_neuroHWInTSPrioT_Layer5;  /**< Priority time of track segments in layer 5 */
    TH1F* m_neuroHWInTSPrioT_Layer6;  /**< Priority time of track segments in layer 6 */
    TH1F* m_neuroHWInTSPrioT_Layer7;  /**< Priority time of track segments in layer 7 */
    TH1F* m_neuroHWInTSPrioT_Layer8;  /**< Priority time of track segments in layer 8 */
    TH1F* m_neuroHWInTSFoundT_Layer0; /**< Found time of track segments in layer 0 */
    TH1F* m_neuroHWInTSFoundT_Layer1; /**< Found time of track segments in layer 1 */
    TH1F* m_neuroHWInTSFoundT_Layer2; /**< Found time of track segments in layer 2 */
    TH1F* m_neuroHWInTSFoundT_Layer3; /**< Found time of track segments in layer 3 */
    TH1F* m_neuroHWInTSFoundT_Layer4; /**< Found time of track segments in layer 4 */
    TH1F* m_neuroHWInTSFoundT_Layer5; /**< Found time of track segments in layer 5 */
    TH1F* m_neuroHWInTSFoundT_Layer6; /**< Found time of track segments in layer 6 */
    TH1F* m_neuroHWInTSFoundT_Layer7; /**< Found time of track segments in layer 7 */
    TH1F* m_neuroHWInTSFoundT_Layer8; /**< Found time of track segments in layer 8 */
    TH1F* m_neuroHWInVs2DOutTrackCount;    /**< number of tracks neuro input 2dtracks vs 2doutput tracks*/
    TH1F* m_neuroHWSelTSPrioT_Layer0;  /**< Priority time of selected track segments in layer 0 */
    TH1F* m_neuroHWSelTSPrioT_Layer1;  /**< Priority time of selected track segments in layer 1 */
    TH1F* m_neuroHWSelTSPrioT_Layer2;  /**< Priority time of selected track segments in layer 2 */
    TH1F* m_neuroHWSelTSPrioT_Layer3;  /**< Priority time of selected track segments in layer 3 */
    TH1F* m_neuroHWSelTSPrioT_Layer4;  /**< Priority time of selected track segments in layer 4 */
    TH1F* m_neuroHWSelTSPrioT_Layer5;  /**< Priority time of selected track segments in layer 5 */
    TH1F* m_neuroHWSelTSPrioT_Layer6;  /**< Priority time of selected track segments in layer 6 */
    TH1F* m_neuroHWSelTSPrioT_Layer7;  /**< Priority time of selected track segments in layer 7 */
    TH1F* m_neuroHWSelTSPrioT_Layer8;  /**< Priority time of selected track segments in layer 8 */
    TH1F* m_neuroHWSelTSFoundT_Layer0; /**< Found time of selected track segments in layer 0 */
    TH1F* m_neuroHWSelTSFoundT_Layer1; /**< Found time of selected track segments in layer 1 */
    TH1F* m_neuroHWSelTSFoundT_Layer2; /**< Found time of selected track segments in layer 2 */
    TH1F* m_neuroHWSelTSFoundT_Layer3; /**< Found time of selected track segments in layer 3 */
    TH1F* m_neuroHWSelTSFoundT_Layer4; /**< Found time of selected track segments in layer 4 */
    TH1F* m_neuroHWSelTSFoundT_Layer5; /**< Found time of selected track segments in layer 5 */
    TH1F* m_neuroHWSelTSFoundT_Layer6; /**< Found time of selected track segments in layer 6 */
    TH1F* m_neuroHWSelTSFoundT_Layer7; /**< Found time of selected track segments in layer 7 */
    TH1F* m_neuroHWSelTSFoundT_Layer8; /**< Found time of selected track segments in layer 8 */
    TH1F* m_neuroHWInTSCount;          /**< number of TS per track */

    TH1F* m_neuroHWInInvPt;            /**< Inverse Pt distribution from incoming 2dtrack */
    TH1F* m_neuroHWInPhi0;             /**< Phi0 of incoming 2dtrack */
    TH1F* m_neuroHWInm_time;           /**< m_time distribution from incoming 2dtracks */
    TH1F* m_neuroHWInTrackCount;       /**< number of neuro input 2dtracks per event */
    TH1F* m_2DHWOutInvPt;              /**< Inverse Pt of 2dtracks */
    TH1F* m_2DHWOutPhi0;               /**< Phi0 of 2dtracks */
    TH1F* m_2DHWOutm_time;             /**< m_time 2dtracks */
    TH1F* m_2DHWOutTrackCount;         /**< number of 2dtracks per event */

    TH1F* m_neuroHWSelTSCount;         /**< number of selected TS per SL */
    TH1F* m_neuroHWSelTSID;            /**< ID of selected track segments */

    TH1F* m_2DHWInTSID;                /**< ID of 2D incoming axial track segments */
    TH1F* m_2DHWInTSPrioT_Layer0;      /**< Priority time of 2D track segments in layer 0 */
    TH1F* m_2DHWInTSPrioT_Layer2;      /**< Priority time of 2D track segments in layer 2 */
    TH1F* m_2DHWInTSPrioT_Layer4;      /**< Priority time of 2D track segments in layer 4 */
    TH1F* m_2DHWInTSPrioT_Layer6;      /**< Priority time of 2D track segments in layer 6 */
    TH1F* m_2DHWInTSPrioT_Layer8;      /**< Priority time of 2D track segments in layer 8 */
    TH1F* m_2DHWInTSFoundT_Layer0;     /**< Found time of 2D track segments in layer 0 */
    TH1F* m_2DHWInTSFoundT_Layer2;     /**< Found time of 2D track segments in layer 2 */
    TH1F* m_2DHWInTSFoundT_Layer4;     /**< Found time of 2D track segments in layer 4 */
    TH1F* m_2DHWInTSFoundT_Layer6;     /**< Found time of 2D track segments in layer 6 */
    TH1F* m_2DHWInTSFoundT_Layer8;     /**< Found time of 2D track segments in layer 8 */
    TH1F* m_2DHWInTSCount;             /**< number of 2D incoming TS per SL */

    TH1F* m_neuroHWSector;           /**< unpacked sector */
    TH1F* m_neuroSWSector;           /**< hw TS hw 2D sw NN sector */
    TH1F* m_neuroSWTSSW2DSector;     /**< sw TS sw 2D sw NN sector */
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

    TH1F* m_neuroHWOutQuad5Z;              /**< z distribution from unpacker */
    TH1F* m_neuroHWOutQuad5CosTheta;       /**< cos theta distribution from unpacker */
    TH1F* m_neuroHWOutQuad5Phi0;           /**< phi distribution from unpacker */
    TH1F* m_neuroHWOutQuad5InvPt;          /**< Inverse Pt distribution from unpacker */

    TH1F* m_neuroHWOutQuad0Z;              /**< z distribution from unpacker */
    TH1F* m_neuroHWOutQuad0CosTheta;       /**< cos theta distribution from unpacker */
    TH1F* m_neuroHWOutQuad0Phi0;           /**< phi distribution from unpacker */
    TH1F* m_neuroHWOutQuad0InvPt;          /**< Inverse Pt distribution from unpacker */
    TH1F* m_neuroHWOutQuad1Z;              /**< z distribution from unpacker */
    TH1F* m_neuroHWOutQuad1CosTheta;       /**< cos theta distribution from unpacker */
    TH1F* m_neuroHWOutQuad1Phi0;           /**< phi distribution from unpacker */
    TH1F* m_neuroHWOutQuad1InvPt;          /**< Inverse Pt distribution from unpacker */
    TH1F* m_neuroHWOutQuad2Z;              /**< z distribution from unpacker */
    TH1F* m_neuroHWOutQuad2CosTheta;       /**< cos theta distribution from unpacker */
    TH1F* m_neuroHWOutQuad2Phi0;           /**< phi distribution from unpacker */
    TH1F* m_neuroHWOutQuad2InvPt;          /**< Inverse Pt distribution from unpacker */
    TH1F* m_neuroHWOutQuad3Z;              /**< z distribution from unpacker */
    TH1F* m_neuroHWOutQuad3CosTheta;       /**< cos theta distribution from unpacker */
    TH1F* m_neuroHWOutQuad3Phi0;           /**< phi distribution from unpacker */
    TH1F* m_neuroHWOutQuad3InvPt;          /**< Inverse Pt distribution from unpacker */

    TH1F* m_neuroSWOutZ;              /**< z distribution from simulation */
    TH1F* m_neuroSWOutCosTheta;       /**< cos theta distribution from simulation */
    TH1F* m_neuroSWOutPhi0;           /**< phi distribution from simulation */
    TH1F* m_neuroSWOutInvPt;          /**< Inverse Pt distribution from simulation */

    TH1F* m_neuroSWTSSW2DOutZ;              /**< z distribution from simulation using simulated TS and simulated 2D */
    TH1F* m_neuroSWTSSW2DOutCosTheta;       /**< cos theta distribution from simulation using simulated TS and simulated 2D */
    TH1F* m_neuroSWTSSW2DOutPhi0;           /**< phi distribution from simulation using simulated TS and simulated 2D */
    TH1F* m_neuroSWTSSW2DOutInvPt;          /**< Inverse Pt distribution from simulation using simulated TS and simulated 2D */

    TH1F* m_RecoSWTSSW2DZ;                 /**< matched to SWTSSW2DSWNN reconstructed z */
    TH1F* m_RecoSWTSSW2DCosTheta;          /**< matched to SWTSSW2DSWNN reconstructed cos(theta) */
    TH1F* m_RecoSWTSSW2DInvPt;             /**< matched to SWTSSW2DSWNN reconstructed inverse Pt */
    TH1F* m_RecoSWTSSW2DPhi;               /**< matched to SWTSSW2DSWNN reconstructed phi */
    TH2F* m_RecoSWTSSW2DZScatter;          /**< matched to SWTSSW2DSWNN reconstructed z scatter plot*/

    TH1F* m_DeltaRecoSWTSSW2DZ;            /**< matched to SWTSSW2DSWNN reconstructed z */
    TH1F* m_DeltaRecoSWTSSW2DCosTheta;     /**< matched to SWTSSW2DSWNN reconstructed cos(theta) */
    TH1F* m_DeltaRecoSWTSSW2DInvPt;        /**< matched to SWTSSW2DSWNN reconstructed inverse Pt */
    TH1F* m_DeltaRecoSWTSSW2DPhi;          /**< matched to SWTSSW2DSWNN reconstructed phi */
  };

}

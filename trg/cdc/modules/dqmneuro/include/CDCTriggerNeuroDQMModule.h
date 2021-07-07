/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
  class CDCTriggerNeuroDQMModule : public HistoModule {  // <- derived from HistoModule class

  public:
    /** Constructor */
    CDCTriggerNeuroDQMModule();
    /** Destructor */
    virtual ~CDCTriggerNeuroDQMModule();

    /** Module functions */
    virtual void initialize() override;
    virtual void beginRun() override;
    virtual void event() override;
    virtual void endRun() override;
    virtual void terminate() override;

    /**
     * Histogram definitions such as TH1(), TH2(), TNtuple(), TTree().... are supposed
     * to be placed in this function.
    */
    virtual void defineHisto() override;
    struct TSLine {
      const CDCTriggerSegmentHit* hit;
      std::string strline;
      //vector<const CDCTriggerTrack*> relin2dtracks;
      //vector<const CDCTriggerTrack*> relin2dtracks;
      explicit TSLine(const CDCTriggerSegmentHit& h)
      {
        hit = &h;
      }
    };
    typedef std::vector<TSLine> TSLines;

  private:
    /** Fill a histogram only with non-zero values */
    void condFill(TH1F* histo, float value)
    {
      if (value != 0) {
        histo->Fill(value);
      }
    }
    /** get pattern of the track */
    unsigned getPattern(CDCTriggerTrack* track, std::string hitCollectionName)
    {
      unsigned pattern = 0;
      for (const CDCTriggerSegmentHit& hit : track->getRelationsTo<CDCTriggerSegmentHit>(hitCollectionName)) {
        unsigned sl = hit.getISuperLayer();
        if (sl % 2 == 1) pattern |= (1 << ((sl - 1) / 2));
      }
      return pattern;
    }
    /** Validity of the pattern*/
    bool isValidPattern(unsigned pattern)
    {
      bool valid = false;
      switch (pattern) {
        case 7:
        case 11:
        case 13:
        case 14:
        case 15:
          valid = true;
      }
      return valid;
    }
    std::string padto(std::string s, unsigned l)
    {
      if (s.size() < l) {
        s.insert(s.begin(), l - s.size(), ' ');
      }
      return s;
    }
    std::string padright(std::string s, unsigned l)
    {
      if (s.size() < l) {
        s.insert(s.end(), l - s.size(), ' ');
      }
      return s;
    }
    bool have_relation(const CDCTriggerTrack& track, const CDCTriggerSegmentHit& hit, std::string& arrayname)
    {
      bool related = false;
      for (const CDCTriggerSegmentHit& ts : track.getRelationsTo<CDCTriggerSegmentHit>(arrayname)) {
        if (&ts == &hit) {related = true;}
      }
      return related;
    }
    void sorted_insert(TSLines& lines, TSLine& line, std::string& arrayname, std::string& firstsortarray, std::string& secondsortarray)
    {
      bool inserted = false;
      bool related = false;
      TSLines::iterator it = lines.begin();
      for (const CDCTriggerTrack& track : line.hit->getRelationsFrom<CDCTriggerTrack>(firstsortarray)) {
        if (!inserted) {
          for (TSLines::iterator i = lines.begin(); i < lines.end(); ++i) {
            if (i->hit->getISuperLayer() % 2 != line.hit->getISuperLayer() % 2) {
              continue;
            }
            if (have_relation(track, *(i->hit), arrayname)) {
              it = i;
              related = true;
              if (i->hit->getSegmentID() > line.hit->getSegmentID()) {
                inserted = true;
                break;
              }
            }
          }
        } else { break; }
      }
      if (related) {
        if (!inserted) {++it; }
        lines.insert(it, line);
      } else {
        for (const CDCTriggerTrack& track : line.hit->getRelationsFrom<CDCTriggerTrack>(secondsortarray)) {
          if (!inserted) {
            for (TSLines::iterator i = it; i < lines.end(); ++i) {
              if (i->hit->getISuperLayer() % 2 != line.hit->getISuperLayer() % 2) {
                continue;
              }
              if (have_relation(track, *(i->hit), arrayname)) {
                it = i;
                related = true;
                if (i->hit->getSegmentID() > line.hit->getSegmentID()) {
                  inserted = true;
                  break;
                }
              }
            }
          } else { break; }
        }
        if (related) {
          if (!inserted) {++it; }
          lines.insert(it, line);
        } else {
          lines.push_back(line);
        }
      }
    }

    /** Name of the histogram directory in ROOT file */
    std::string m_histogramDirectoryName;
    /** Switch to supress output for dqm online module */
    bool m_limitedoutput;
    /** Number of identical track segments to be required for matching between HW and SW Neurotrigger */
    int m_nsamets;
    // names for unpacked objects
    /** Name for TS hits from unpacker */
    std::string m_unpackedSegmentHitsName;
    /** Name for 2D finder tracks from unpacker */
    std::string m_unpacked2DTracksName;
    /** Name for neuro tracks from unpacker */
    std::string m_unpackedNeuroTracksName;
    /** Name for neuro input vector from unpacker */
    std::string m_unpackedNeuroInputVectorName;
    /** Name for neuro input 2d finder tracks */
    std::string m_unpackedNeuroInput2DTracksName;
    /** Name for neuro input Track segments */
    std::string m_unpackedNeuroInputSegmentsName;

    // names for simulated objects
    /** Name for neuro tracks from TSIM using HW TS, HW 2D */
    std::string m_simNeuroTracksName;
    /** Name for simulated neuro input vector using HW TS, HW 2D*/
    std::string m_simNeuroInputVectorName;
    /** Switch to turn on a comparison with the reconstruction */
    bool m_showRecoTracks;
    /** Select events with a specific RecoTrack track multiplicity. -1 for all events */
    int m_recoTrackMultiplicity;
    /** Switch to skip events without unpacked TS */
    bool m_skipWithoutHWTS;
    /** Select only RecoTracks with a maximum z distance to the IP. -1 for all tracks */
    double m_maxRecoZDist;
    /** Select only RecoTracks with a maximum d0 distance to the z axis. -1 for all tracks */
    double m_maxRecoD0Dist;

    /** Name for simulated TS hits */
    std::string m_simSegmentHitsName;
    /** Name for simulated 2D finder tracks using simulated TS */
    std::string m_sim2DTracksSWTSName;
    /** Name for neuro tracks using simulated TS and simulated 2D */
    std::string m_simNeuroTracksSWTSSW2DName;
    /** Name for neuro input vector using simulated TS and simulated 2D */
    std::string m_simNeuroInputVectorSWTSSW2DName;

    /** Name for the RecoTrack array name */
    std::string m_recoTracksName;


    // store arrays for direct access
    /** StoreArray for TS hits from unpacker */
    StoreArray<CDCTriggerSegmentHit> m_unpackedSegmentHits;
    /** StoreArray for 2D finder tracks from unpacker */
    StoreArray<CDCTriggerTrack> m_unpacked2DTracks;
    /** StoreArray for neuro tracks from unpacker */
    StoreArray<CDCTriggerTrack> m_unpackedNeuroTracks;
    /** StoreArray for neuro input vector from unpacker */
    StoreArray<CDCTriggerMLPInput> m_unpackedNeuroInputVector;
    /** StoreArray for neuro input 2dfinder tracks */
    StoreArray<CDCTriggerTrack> m_unpackedNeuroInput2DTracks;
    /** StoreArray for neuro input Track segments */
    StoreArray<CDCTriggerSegmentHit> m_unpackedNeuroInputSegments;
    /** StoreArray for neuro tracks from TSIM */
    StoreArray<CDCTriggerTrack> m_simNeuroTracks;
    /** StoreArray for neuro input vector from TSIM */
    StoreArray<CDCTriggerMLPInput> m_simNeuroInputVector;
    /** StoreArray for RecoTracks */
    StoreArray<RecoTrack> m_RecoTracks;

    /** StoreArray for simulated TS hits */
    StoreArray<CDCTriggerSegmentHit> m_simSegmentHits;
    /** StoreArray for simulated 2D finder tracks using simulated TS */
    StoreArray<CDCTriggerTrack> m_sim2DTracksSWTS;
    /** StoreArray for neuro input vector using simulated TS and simulated 2D */
    StoreArray<CDCTriggerMLPInput> m_simNeuroInputVectorSWTSSW2D;
    /** StoreArray for neuro tracks using simulated TS and simulated 2D */
    StoreArray<CDCTriggerTrack> m_simNeuroTracksSWTSSW2D;

    // histograms for neurotrigger
    TH1F* m_neuroHWOutZ = nullptr;                     /**< z distribution of unpacked neuro tracks */
    TH1F* m_neuroHWOutCosTheta = nullptr;              /**< cos theta distribution of unpacked neuro tracks */
    TH1F* m_neuroHWOutInvPt = nullptr;                 /**< Inverse Pt distribution of unpacked neuro tracks */
    TH1F* m_neuroHWOutPt = nullptr;                    /**< Pt distribution of unpacked neuro tracks */
    TH1F* m_neuroHWOutPhi0 = nullptr;                  /**< phi distribution of unpacked neuro tracks */
    TH1F* m_neuroHWOutHitPattern = nullptr;            /**< stereo hit pattern of simulated neuro tracks (hw TS hw 2D sw NN) */
    TH1F* m_neuroHWOutm_time = nullptr;                /**< m_time distribution of unpacked neuro tracks */
    TH1F* m_neuroHWOutTrackCount = nullptr;            /**< number of unpacked tracks per event */
    TH1F* m_neuroHWSector = nullptr;                   /**< sector of unpacked neuro tracks */

    TH1F* m_neuroHWInInvPt = nullptr;                  /**< Inverse Pt distribution from incoming 2dtrack */
    TH1F* m_neuroHWInPhi0 = nullptr;                   /**< Phi0 of incoming 2dtrack */
    TH1F* m_neuroHWInm_time = nullptr;                 /**< m_time distribution from incoming 2dtracks */
    TH1F* m_neuroHWInTrackCount = nullptr;             /**< number of neuro input 2dtracks per event */
    TH1F* m_neuroHWOutVsInTrackCount = nullptr;        /**< neuroHWOutTracks - neuroHWInTrackCount */

    TH1F* m_2DHWOutInvPt = nullptr;                    /**< Inverse Pt of 2dtracks */
    TH1F* m_2DHWOutPhi0 = nullptr;                     /**< Phi0 of 2dtracks */
    TH1F* m_2DHWOutm_time = nullptr;                   /**< m_time 2dtracks */
    TH1F* m_2DHWOutTrackCount = nullptr;               /**< number of 2dtracks per event */
    TH1F* m_neuroHWInVs2DOutTrackCount = nullptr;      /**< neuroHWInTrackCount - 2DHWOutTrackCountput*/

    // hw TS hw 2D sw NN
    TH1F* m_neuroSWOutZ = nullptr;                     /**< z distribution from simulation (hw TS hw 2D sw NN) */
    TH1F* m_neuroSWOutCosTheta = nullptr;              /**< cos theta distribution from simulation (hw TS hw 2D sw NN) */
    TH1F* m_neuroSWOutInvPt = nullptr;                 /**< Inverse Pt distribution from simulation (hw TS hw 2D sw NN) */
    TH1F* m_neuroSWOutPhi0 = nullptr;                  /**< phi distribution from simulation (hw TS hw 2D sw NN) */
    TH1F* m_neuroSWOutHitPattern = nullptr;            /**< stereo hit pattern of simulated neuro tracks (hw TS hw 2D sw NN) */
    TH1F* m_neuroSWOutTrackCount = nullptr;            /**< number of simulated tracks per event (hw TS hw 2D sw NN) */
    TH1F* m_neuroSWSector = nullptr;                   /**< NN sector from simulation (hw TS hw 2D sw NN) */


    TH1F* m_2DSWOutInvPt = nullptr;                    /**< Inverse Pt of 2dtracks (sw TS sw 2D) */
    TH1F* m_2DSWOutPhi0 = nullptr;                     /**< Phi0 of 2dtracks (sw TS sw 2D) */
    TH1F* m_2DSWOutm_time = nullptr;                   /**< m_time 2dtracks (sw TS sw 2D) */
    TH1F* m_2DSWOutTrackCount = nullptr;               /**< number of 2dtracks per event (sw TS sw 2D) */

    // sw TS sw 2D sw NN
    TH1F* m_neuroSWTSSW2DOutZ = nullptr;               /**< z distribution from simulation (sw TS sw 2D sw NN) */
    TH1F* m_neuroSWTSSW2DOutCosTheta = nullptr;        /**< cos theta distribution from simulation (sw TS sw 2D sw NN) */
    TH1F* m_neuroSWTSSW2DOutInvPt = nullptr;           /**< Inverse Pt distribution from simulation (sw TS sw 2D sw NN) */
    TH1F* m_neuroSWTSSW2DOutPhi0 = nullptr;            /**< phi distribution from simulation (sw TS sw 2D sw NN) */
    TH1F* m_neuroSWTSSW2DOutHitPattern = nullptr;      /**< stereo hit pattern of simulated neuro tracks (sw TS sw 2D sw NN) */
    TH1F* m_neuroSWTSSW2DOutTrackCount = nullptr;      /**< number of simulated tracks per event (sw TS sw 2D sw NN) */
    TH1F* m_neuroSWTSSW2DSector = nullptr;             /**< NN sector from simulation (sw TS sw 2D sw NN) */

    // hw TS hw 2D sw NN vs hw NN
    TH1F* m_neuroDeltaZ = nullptr;                     /**< unpacked z - TSIM z */
    TH1F* m_neuroDeltaTheta = nullptr;                 /**< unpacked theta - TSIM theta */
    TH2F* m_neuroScatterZ = nullptr;                   /**< unpacked z vs TSIM z, scatter plot*/
    TH2F* m_neuroScatterTheta = nullptr;               /**< unpacked theta vs TSIM theta, scatter plot*/

    TH1F* m_neuroDeltaInputID = nullptr;               /**< unpacked ID input - TSIM ID input */
    TH1F* m_neuroDeltaInputT = nullptr;                /**< unpacked time input - TSIM time input */
    TH1F* m_neuroDeltaInputAlpha = nullptr;            /**< unpacked alpha input - TSIM alpha input */
    TH1F* m_neuroDeltaTSID = nullptr;                  /**< unpacked selected TSID - TSIM selected TSID */
    TH1F* m_neuroDeltaSector = nullptr;                /**< unpacked sector - TSIM sector */
    TH1F* m_simSameTS = nullptr;                       /**< number of TS selected in both, unpacked and TSIM tracks */
    TH1F* m_simDiffTS = nullptr;                       /**< number of TS selcted in TSIM but not in unpacked */

    // hw NN output for each quadrant
    TH1F* m_neuroHWOutQuad5Z = nullptr;                /**< z distribution from unpacker (no quadrant set) */
    TH1F* m_neuroHWOutQuad5CosTheta = nullptr;         /**< cos theta distribution from unpacker (no quadrant set) */
    TH1F* m_neuroHWOutQuad5Phi0 = nullptr;             /**< phi distribution from unpacker (no quadrant set) */
    TH1F* m_neuroHWOutQuad5InvPt = nullptr;            /**< Inverse Pt distribution from unpacker (no quadrant set) */
    TH1F* m_neuroHWOutQuad0Z = nullptr;                /**< z distribution from unpacker (quadrant 0) */
    TH1F* m_neuroHWOutQuad0CosTheta = nullptr;         /**< cos theta distribution from unpacker (quadrant 0) */
    TH1F* m_neuroHWOutQuad0Phi0 = nullptr;             /**< phi distribution from unpacker (quadrant 0) */
    TH1F* m_neuroHWOutQuad0InvPt = nullptr;            /**< Inverse Pt distribution from unpacker (quadrant 0) */
    TH1F* m_neuroHWOutQuad1Z = nullptr;                /**< z distribution from unpacker (quadrant 1) */
    TH1F* m_neuroHWOutQuad1CosTheta = nullptr;         /**< cos theta distribution from unpacker (quadrant 1) */
    TH1F* m_neuroHWOutQuad1Phi0 = nullptr;             /**< phi distribution from unpacker (quadrant 1) */
    TH1F* m_neuroHWOutQuad1InvPt = nullptr;            /**< Inverse Pt distribution from unpacker (quadrant 1) */
    TH1F* m_neuroHWOutQuad2Z = nullptr;                /**< z distribution from unpacker (quadrant 2) */
    TH1F* m_neuroHWOutQuad2CosTheta = nullptr;         /**< cos theta distribution from unpacker (quadrant 2) */
    TH1F* m_neuroHWOutQuad2Phi0 = nullptr;             /**< phi distribution from unpacker (quadrant 2) */
    TH1F* m_neuroHWOutQuad2InvPt = nullptr;            /**< Inverse Pt distribution from unpacker (quadrant 2) */
    TH1F* m_neuroHWOutQuad3Z = nullptr;                /**< z distribution from unpacker (quadrant 3) */
    TH1F* m_neuroHWOutQuad3CosTheta = nullptr;         /**< cos theta distribution from unpacker (quadrant 3) */
    TH1F* m_neuroHWOutQuad3Phi0 = nullptr;             /**< phi distribution from unpacker (quadrant 3) */
    TH1F* m_neuroHWOutQuad3InvPt = nullptr;            /**< Inverse Pt distribution from unpacker (quadrant 3) */

    // hw NN received TS
    TH1F* m_neuroHWInTSID = nullptr;                   /**< ID of incoming track segments */
    TH1F* m_neuroHWInTSCount = nullptr;                /**< number of TS per track */
    TH1F* m_neuroHWInTSPrioT_Layer0 = nullptr;         /**< Priority time of track segments in layer 0 */
    TH1F* m_neuroHWInTSPrioT_Layer1 = nullptr;         /**< Priority time of track segments in layer 1 */
    TH1F* m_neuroHWInTSPrioT_Layer2 = nullptr;         /**< Priority time of track segments in layer 2 */
    TH1F* m_neuroHWInTSPrioT_Layer3 = nullptr;         /**< Priority time of track segments in layer 3 */
    TH1F* m_neuroHWInTSPrioT_Layer4 = nullptr;         /**< Priority time of track segments in layer 4 */
    TH1F* m_neuroHWInTSPrioT_Layer5 = nullptr;         /**< Priority time of track segments in layer 5 */
    TH1F* m_neuroHWInTSPrioT_Layer6 = nullptr;         /**< Priority time of track segments in layer 6 */
    TH1F* m_neuroHWInTSPrioT_Layer7 = nullptr;         /**< Priority time of track segments in layer 7 */
    TH1F* m_neuroHWInTSPrioT_Layer8 = nullptr;         /**< Priority time of track segments in layer 8 */
    TH1F* m_neuroHWInTSFoundT_Layer0 = nullptr;        /**< Found time of track segments in layer 0 */
    TH1F* m_neuroHWInTSFoundT_Layer1 = nullptr;        /**< Found time of track segments in layer 1 */
    TH1F* m_neuroHWInTSFoundT_Layer2 = nullptr;        /**< Found time of track segments in layer 2 */
    TH1F* m_neuroHWInTSFoundT_Layer3 = nullptr;        /**< Found time of track segments in layer 3 */
    TH1F* m_neuroHWInTSFoundT_Layer4 = nullptr;        /**< Found time of track segments in layer 4 */
    TH1F* m_neuroHWInTSFoundT_Layer5 = nullptr;        /**< Found time of track segments in layer 5 */
    TH1F* m_neuroHWInTSFoundT_Layer6 = nullptr;        /**< Found time of track segments in layer 6 */
    TH1F* m_neuroHWInTSFoundT_Layer7 = nullptr;        /**< Found time of track segments in layer 7 */
    TH1F* m_neuroHWInTSFoundT_Layer8 = nullptr;        /**< Found time of track segments in layer 8 */

    TH1F* m_neuroHWInTSPrioB_Layer0 = nullptr;         /**< Priority bits of track segments in layer 0 */
    TH1F* m_neuroHWInTSPrioB_Layer1 = nullptr;         /**< Priority bits of track segments in layer 1 */
    TH1F* m_neuroHWInTSPrioB_Layer2 = nullptr;         /**< Priority bits of track segments in layer 2 */
    TH1F* m_neuroHWInTSPrioB_Layer3 = nullptr;         /**< Priority bits of track segments in layer 3 */
    TH1F* m_neuroHWInTSPrioB_Layer4 = nullptr;         /**< Priority bits of track segments in layer 4 */
    TH1F* m_neuroHWInTSPrioB_Layer5 = nullptr;         /**< Priority bits of track segments in layer 5 */
    TH1F* m_neuroHWInTSPrioB_Layer6 = nullptr;         /**< Priority bits of track segments in layer 6 */
    TH1F* m_neuroHWInTSPrioB_Layer7 = nullptr;         /**< Priority bits of track segments in layer 7 */
    TH1F* m_neuroHWInTSPrioB_Layer8 = nullptr;         /**< Priority bits of track segments in layer 8 */

    TH1F* m_neuroHWInTSLR_Layer0 = nullptr;            /**< Left/Right of track segments in layer 0 */
    TH1F* m_neuroHWInTSLR_Layer1 = nullptr;            /**< Left/Right of track segments in layer 1 */
    TH1F* m_neuroHWInTSLR_Layer2 = nullptr;            /**< Left/Right of track segments in layer 2 */
    TH1F* m_neuroHWInTSLR_Layer3 = nullptr;            /**< Left/Right of track segments in layer 3 */
    TH1F* m_neuroHWInTSLR_Layer4 = nullptr;            /**< Left/Right of track segments in layer 4 */
    TH1F* m_neuroHWInTSLR_Layer5 = nullptr;            /**< Left/Right of track segments in layer 5 */
    TH1F* m_neuroHWInTSLR_Layer6 = nullptr;            /**< Left/Right of track segments in layer 6 */
    TH1F* m_neuroHWInTSLR_Layer7 = nullptr;            /**< Left/Right of track segments in layer 7 */
    TH1F* m_neuroHWInTSLR_Layer8 = nullptr;            /**< Left/Right of track segments in layer 8 */

    // hw NN selected TS
    TH1F* m_neuroHWSelTSID = nullptr;                  /**< ID of selected track segments */
    TH1F* m_neuroHWSelTSCount = nullptr;               /**< number of selected TS per SL */
    TH1F* m_neuroHWSelTSPrioT_Layer0 = nullptr;        /**< Priority time of selected track segments in layer 0 */
    TH1F* m_neuroHWSelTSPrioT_Layer1 = nullptr;        /**< Priority time of selected track segments in layer 1 */
    TH1F* m_neuroHWSelTSPrioT_Layer2 = nullptr;        /**< Priority time of selected track segments in layer 2 */
    TH1F* m_neuroHWSelTSPrioT_Layer3 = nullptr;        /**< Priority time of selected track segments in layer 3 */
    TH1F* m_neuroHWSelTSPrioT_Layer4 = nullptr;        /**< Priority time of selected track segments in layer 4 */
    TH1F* m_neuroHWSelTSPrioT_Layer5 = nullptr;        /**< Priority time of selected track segments in layer 5 */
    TH1F* m_neuroHWSelTSPrioT_Layer6 = nullptr;        /**< Priority time of selected track segments in layer 6 */
    TH1F* m_neuroHWSelTSPrioT_Layer7 = nullptr;        /**< Priority time of selected track segments in layer 7 */
    TH1F* m_neuroHWSelTSPrioT_Layer8 = nullptr;        /**< Priority time of selected track segments in layer 8 */
    TH1F* m_neuroHWSelTSFoundT_Layer0 = nullptr;       /**< Found time of selected track segments in layer 0 */
    TH1F* m_neuroHWSelTSFoundT_Layer1 = nullptr;       /**< Found time of selected track segments in layer 1 */
    TH1F* m_neuroHWSelTSFoundT_Layer2 = nullptr;       /**< Found time of selected track segments in layer 2 */
    TH1F* m_neuroHWSelTSFoundT_Layer3 = nullptr;       /**< Found time of selected track segments in layer 3 */
    TH1F* m_neuroHWSelTSFoundT_Layer4 = nullptr;       /**< Found time of selected track segments in layer 4 */
    TH1F* m_neuroHWSelTSFoundT_Layer5 = nullptr;       /**< Found time of selected track segments in layer 5 */
    TH1F* m_neuroHWSelTSFoundT_Layer6 = nullptr;       /**< Found time of selected track segments in layer 6 */
    TH1F* m_neuroHWSelTSFoundT_Layer7 = nullptr;       /**< Found time of selected track segments in layer 7 */
    TH1F* m_neuroHWSelTSFoundT_Layer8 = nullptr;       /**< Found time of selected track segments in layer 8 */

    TH1F* m_neuroHWSelTSPrioB_Layer0 = nullptr;        /**< Priority bits of track segments in layer 0 */
    TH1F* m_neuroHWSelTSPrioB_Layer1 = nullptr;        /**< Priority bits of track segments in layer 1 */
    TH1F* m_neuroHWSelTSPrioB_Layer2 = nullptr;        /**< Priority bits of track segments in layer 2 */
    TH1F* m_neuroHWSelTSPrioB_Layer3 = nullptr;        /**< Priority bits of track segments in layer 3 */
    TH1F* m_neuroHWSelTSPrioB_Layer4 = nullptr;        /**< Priority bits of track segments in layer 4 */
    TH1F* m_neuroHWSelTSPrioB_Layer5 = nullptr;        /**< Priority bits of track segments in layer 5 */
    TH1F* m_neuroHWSelTSPrioB_Layer6 = nullptr;        /**< Priority bits of track segments in layer 6 */
    TH1F* m_neuroHWSelTSPrioB_Layer7 = nullptr;        /**< Priority bits of track segments in layer 7 */
    TH1F* m_neuroHWSelTSPrioB_Layer8 = nullptr;        /**< Priority bits of track segments in layer 8 */

    TH1F* m_neuroHWSelTSLR_Layer0 = nullptr;           /**< Left/Right of track segments in layer 0 */
    TH1F* m_neuroHWSelTSLR_Layer1 = nullptr;           /**< Left/Right of track segments in layer 1 */
    TH1F* m_neuroHWSelTSLR_Layer2 = nullptr;           /**< Left/Right of track segments in layer 2 */
    TH1F* m_neuroHWSelTSLR_Layer3 = nullptr;           /**< Left/Right of track segments in layer 3 */
    TH1F* m_neuroHWSelTSLR_Layer4 = nullptr;           /**< Left/Right of track segments in layer 4 */
    TH1F* m_neuroHWSelTSLR_Layer5 = nullptr;           /**< Left/Right of track segments in layer 5 */
    TH1F* m_neuroHWSelTSLR_Layer6 = nullptr;           /**< Left/Right of track segments in layer 6 */
    TH1F* m_neuroHWSelTSLR_Layer7 = nullptr;           /**< Left/Right of track segments in layer 7 */
    TH1F* m_neuroHWSelTSLR_Layer8 = nullptr;           /**< Left/Right of track segments in layer 8 */


    // hw 2D received TS
    //TH1F* m_2DHWInTSID = nullptr;                      /**< ID of 2D incoming axial track segments */
    //TH1F* m_2DHWInTSCount = nullptr;                   /**< number of 2D incoming TS per event */
    TH1F* m_2DHWInTSPrioT_Layer0 = nullptr;            /**< Priority time of 2D track segments in layer 0 */
    TH1F* m_2DHWInTSPrioT_Layer2 = nullptr;            /**< Priority time of 2D track segments in layer 2 */
    TH1F* m_2DHWInTSPrioT_Layer4 = nullptr;            /**< Priority time of 2D track segments in layer 4 */
    TH1F* m_2DHWInTSPrioT_Layer6 = nullptr;            /**< Priority time of 2D track segments in layer 6 */
    TH1F* m_2DHWInTSPrioT_Layer8 = nullptr;            /**< Priority time of 2D track segments in layer 8 */
    TH1F* m_2DHWInTSFoundT_Layer0 = nullptr;           /**< Found time of 2D track segments in layer 0 */
    TH1F* m_2DHWInTSFoundT_Layer2 = nullptr;           /**< Found time of 2D track segments in layer 2 */
    TH1F* m_2DHWInTSFoundT_Layer4 = nullptr;           /**< Found time of 2D track segments in layer 4 */
    TH1F* m_2DHWInTSFoundT_Layer6 = nullptr;           /**< Found time of 2D track segments in layer 6 */
    TH1F* m_2DHWInTSFoundT_Layer8 = nullptr;           /**< Found time of 2D track segments in layer 8 */

    TH1F* m_2DHWInTSPrioB_Layer0 = nullptr;            /**< Priority bits of 2D track segments in layer 0 */
    TH1F* m_2DHWInTSPrioB_Layer2 = nullptr;            /**< Priority bits of 2D track segments in layer 2 */
    TH1F* m_2DHWInTSPrioB_Layer4 = nullptr;            /**< Priority bits of 2D track segments in layer 4 */
    TH1F* m_2DHWInTSPrioB_Layer6 = nullptr;            /**< Priority bits of 2D track segments in layer 6 */
    TH1F* m_2DHWInTSPrioB_Layer8 = nullptr;            /**< Priority bits of 2D track segments in layer 8 */
    TH1F* m_2DHWInTSLR_Layer0 = nullptr;               /**< Left/Right of 2D track segments in layer 0 */
    TH1F* m_2DHWInTSLR_Layer2 = nullptr;               /**< Left/Right of 2D track segments in layer 2 */
    TH1F* m_2DHWInTSLR_Layer4 = nullptr;               /**< Left/Right of 2D track segments in layer 4 */
    TH1F* m_2DHWInTSLR_Layer6 = nullptr;               /**< Left/Right of 2D track segments in layer 6 */
    TH1F* m_2DHWInTSLR_Layer8 = nullptr;               /**< Left/Right of 2D track segments in layer 8 */


    // hw TS selected by sw NN
    TH1F* m_neuroSWSelTSID = nullptr;                  /**< ID of incoming track segments */
    TH1F* m_neuroSWSelTSCount = nullptr;               /**< number of TS per SL */
    TH1F* m_neuroSWSelTSPrioT_Layer0 = nullptr;        /**< Priority time of track segments in layer 0 */
    TH1F* m_neuroSWSelTSPrioT_Layer1 = nullptr;        /**< Priority time of track segments in layer 1 */
    TH1F* m_neuroSWSelTSPrioT_Layer2 = nullptr;        /**< Priority time of track segments in layer 2 */
    TH1F* m_neuroSWSelTSPrioT_Layer3 = nullptr;        /**< Priority time of track segments in layer 3 */
    TH1F* m_neuroSWSelTSPrioT_Layer4 = nullptr;        /**< Priority time of track segments in layer 4 */
    TH1F* m_neuroSWSelTSPrioT_Layer5 = nullptr;        /**< Priority time of track segments in layer 5 */
    TH1F* m_neuroSWSelTSPrioT_Layer6 = nullptr;        /**< Priority time of track segments in layer 6 */
    TH1F* m_neuroSWSelTSPrioT_Layer7 = nullptr;        /**< Priority time of track segments in layer 7 */
    TH1F* m_neuroSWSelTSPrioT_Layer8 = nullptr;        /**< Priority time of track segments in layer 8 */
    TH1F* m_neuroSWSelTSFoundT_Layer0 = nullptr;       /**< Found time of track segments in layer 0 */
    TH1F* m_neuroSWSelTSFoundT_Layer1 = nullptr;       /**< Found time of track segments in layer 1 */
    TH1F* m_neuroSWSelTSFoundT_Layer2 = nullptr;       /**< Found time of track segments in layer 2 */
    TH1F* m_neuroSWSelTSFoundT_Layer3 = nullptr;       /**< Found time of track segments in layer 3 */
    TH1F* m_neuroSWSelTSFoundT_Layer4 = nullptr;       /**< Found time of track segments in layer 4 */
    TH1F* m_neuroSWSelTSFoundT_Layer5 = nullptr;       /**< Found time of track segments in layer 5 */
    TH1F* m_neuroSWSelTSFoundT_Layer6 = nullptr;       /**< Found time of track segments in layer 6 */
    TH1F* m_neuroSWSelTSFoundT_Layer7 = nullptr;       /**< Found time of track segments in layer 7 */
    TH1F* m_neuroSWSelTSFoundT_Layer8 = nullptr;       /**< Found time of track segments in layer 8 */

    TH1F* m_neuroSWSelTSPrioB_Layer0 = nullptr;        /**< Priority bits of track segments in layer 0 */
    TH1F* m_neuroSWSelTSPrioB_Layer1 = nullptr;        /**< Priority bits of track segments in layer 1 */
    TH1F* m_neuroSWSelTSPrioB_Layer2 = nullptr;        /**< Priority bits of track segments in layer 2 */
    TH1F* m_neuroSWSelTSPrioB_Layer3 = nullptr;        /**< Priority bits of track segments in layer 3 */
    TH1F* m_neuroSWSelTSPrioB_Layer4 = nullptr;        /**< Priority bits of track segments in layer 4 */
    TH1F* m_neuroSWSelTSPrioB_Layer5 = nullptr;        /**< Priority bits of track segments in layer 5 */
    TH1F* m_neuroSWSelTSPrioB_Layer6 = nullptr;        /**< Priority bits of track segments in layer 6 */
    TH1F* m_neuroSWSelTSPrioB_Layer7 = nullptr;        /**< Priority bits of track segments in layer 7 */
    TH1F* m_neuroSWSelTSPrioB_Layer8 = nullptr;        /**< Priority bits of track segments in layer 8 */

    TH1F* m_neuroSWSelTSLR_Layer0 = nullptr;           /**< Left/Right of track segments in layer 0 */
    TH1F* m_neuroSWSelTSLR_Layer1 = nullptr;           /**< Left/Right of track segments in layer 1 */
    TH1F* m_neuroSWSelTSLR_Layer2 = nullptr;           /**< Left/Right of track segments in layer 2 */
    TH1F* m_neuroSWSelTSLR_Layer3 = nullptr;           /**< Left/Right of track segments in layer 3 */
    TH1F* m_neuroSWSelTSLR_Layer4 = nullptr;           /**< Left/Right of track segments in layer 4 */
    TH1F* m_neuroSWSelTSLR_Layer5 = nullptr;           /**< Left/Right of track segments in layer 5 */
    TH1F* m_neuroSWSelTSLR_Layer6 = nullptr;           /**< Left/Right of track segments in layer 6 */
    TH1F* m_neuroSWSelTSLR_Layer7 = nullptr;           /**< Left/Right of track segments in layer 7 */
    TH1F* m_neuroSWSelTSLR_Layer8 = nullptr;           /**< Left/Right of track segments in layer 8 */


    // sw TS incoming
    TH1F* m_neuroSWTSSW2DInTSID = nullptr;             /**< ID of incoming track segments */
    TH1F* m_neuroSWTSSW2DInTSCount = nullptr;          /**< number of TS per event */
    TH1F* m_neuroSWTSSW2DInTSPrioT_Layer0 = nullptr;   /**< Priority time of track segments in layer 0 */
    TH1F* m_neuroSWTSSW2DInTSPrioT_Layer1 = nullptr;   /**< Priority time of track segments in layer 1 */
    TH1F* m_neuroSWTSSW2DInTSPrioT_Layer2 = nullptr;   /**< Priority time of track segments in layer 2 */
    TH1F* m_neuroSWTSSW2DInTSPrioT_Layer3 = nullptr;   /**< Priority time of track segments in layer 3 */
    TH1F* m_neuroSWTSSW2DInTSPrioT_Layer4 = nullptr;   /**< Priority time of track segments in layer 4 */
    TH1F* m_neuroSWTSSW2DInTSPrioT_Layer5 = nullptr;   /**< Priority time of track segments in layer 5 */
    TH1F* m_neuroSWTSSW2DInTSPrioT_Layer6 = nullptr;   /**< Priority time of track segments in layer 6 */
    TH1F* m_neuroSWTSSW2DInTSPrioT_Layer7 = nullptr;   /**< Priority time of track segments in layer 7 */
    TH1F* m_neuroSWTSSW2DInTSPrioT_Layer8 = nullptr;   /**< Priority time of track segments in layer 8 */
    TH1F* m_neuroSWTSSW2DInTSFoundT_Layer0 = nullptr;  /**< Found time of track segments in layer 0 */
    TH1F* m_neuroSWTSSW2DInTSFoundT_Layer1 = nullptr;  /**< Found time of track segments in layer 1 */
    TH1F* m_neuroSWTSSW2DInTSFoundT_Layer2 = nullptr;  /**< Found time of track segments in layer 2 */
    TH1F* m_neuroSWTSSW2DInTSFoundT_Layer3 = nullptr;  /**< Found time of track segments in layer 3 */
    TH1F* m_neuroSWTSSW2DInTSFoundT_Layer4 = nullptr;  /**< Found time of track segments in layer 4 */
    TH1F* m_neuroSWTSSW2DInTSFoundT_Layer5 = nullptr;  /**< Found time of track segments in layer 5 */
    TH1F* m_neuroSWTSSW2DInTSFoundT_Layer6 = nullptr;  /**< Found time of track segments in layer 6 */
    TH1F* m_neuroSWTSSW2DInTSFoundT_Layer7 = nullptr;  /**< Found time of track segments in layer 7 */
    TH1F* m_neuroSWTSSW2DInTSFoundT_Layer8 = nullptr;  /**< Found time of track segments in layer 8 */

    TH1F* m_neuroSWTSSW2DInTSPrioB_Layer0 = nullptr;   /**< Priority bits of track segments in layer 0 */
    TH1F* m_neuroSWTSSW2DInTSPrioB_Layer1 = nullptr;   /**< Priority bits of track segments in layer 1 */
    TH1F* m_neuroSWTSSW2DInTSPrioB_Layer2 = nullptr;   /**< Priority bits of track segments in layer 2 */
    TH1F* m_neuroSWTSSW2DInTSPrioB_Layer3 = nullptr;   /**< Priority bits of track segments in layer 3 */
    TH1F* m_neuroSWTSSW2DInTSPrioB_Layer4 = nullptr;   /**< Priority bits of track segments in layer 4 */
    TH1F* m_neuroSWTSSW2DInTSPrioB_Layer5 = nullptr;   /**< Priority bits of track segments in layer 5 */
    TH1F* m_neuroSWTSSW2DInTSPrioB_Layer6 = nullptr;   /**< Priority bits of track segments in layer 6 */
    TH1F* m_neuroSWTSSW2DInTSPrioB_Layer7 = nullptr;   /**< Priority bits of track segments in layer 7 */
    TH1F* m_neuroSWTSSW2DInTSPrioB_Layer8 = nullptr;   /**< Priority bits of track segments in layer 8 */

    TH1F* m_neuroSWTSSW2DInTSLR_Layer0 = nullptr;      /**< Left/Right of track segments in layer 0 */
    TH1F* m_neuroSWTSSW2DInTSLR_Layer1 = nullptr;      /**< Left/Right of track segments in layer 1 */
    TH1F* m_neuroSWTSSW2DInTSLR_Layer2 = nullptr;      /**< Left/Right of track segments in layer 2 */
    TH1F* m_neuroSWTSSW2DInTSLR_Layer3 = nullptr;      /**< Left/Right of track segments in layer 3 */
    TH1F* m_neuroSWTSSW2DInTSLR_Layer4 = nullptr;      /**< Left/Right of track segments in layer 4 */
    TH1F* m_neuroSWTSSW2DInTSLR_Layer5 = nullptr;      /**< Left/Right of track segments in layer 5 */
    TH1F* m_neuroSWTSSW2DInTSLR_Layer6 = nullptr;      /**< Left/Right of track segments in layer 6 */
    TH1F* m_neuroSWTSSW2DInTSLR_Layer7 = nullptr;      /**< Left/Right of track segments in layer 7 */
    TH1F* m_neuroSWTSSW2DInTSLR_Layer8 = nullptr;      /**< Left/Right of track segments in layer 8 */


    // sw TS selected
    TH1F* m_neuroSWTSSW2DSelTSID = nullptr;            /**< ID of incoming track segments */
    TH1F* m_neuroSWTSSW2DSelTSCount = nullptr;         /**< number of TS per SL */
    TH1F* m_neuroSWTSSW2DSelTSPrioT_Layer0 = nullptr;  /**< Priority time of selected track segments in layer 0 */
    TH1F* m_neuroSWTSSW2DSelTSPrioT_Layer1 = nullptr;  /**< Priority time of selected track segments in layer 1 */
    TH1F* m_neuroSWTSSW2DSelTSPrioT_Layer2 = nullptr;  /**< Priority time of selected track segments in layer 2 */
    TH1F* m_neuroSWTSSW2DSelTSPrioT_Layer3 = nullptr;  /**< Priority time of selected track segments in layer 3 */
    TH1F* m_neuroSWTSSW2DSelTSPrioT_Layer4 = nullptr;  /**< Priority time of selected track segments in layer 4 */
    TH1F* m_neuroSWTSSW2DSelTSPrioT_Layer5 = nullptr;  /**< Priority time of selected track segments in layer 5 */
    TH1F* m_neuroSWTSSW2DSelTSPrioT_Layer6 = nullptr;  /**< Priority time of selected track segments in layer 6 */
    TH1F* m_neuroSWTSSW2DSelTSPrioT_Layer7 = nullptr;  /**< Priority time of selected track segments in layer 7 */
    TH1F* m_neuroSWTSSW2DSelTSPrioT_Layer8 = nullptr;  /**< Priority time of selected track segments in layer 8 */
    TH1F* m_neuroSWTSSW2DSelTSFoundT_Layer0 = nullptr; /**< Found time of selected track segments in layer 0 */
    TH1F* m_neuroSWTSSW2DSelTSFoundT_Layer1 = nullptr; /**< Found time of selected track segments in layer 1 */
    TH1F* m_neuroSWTSSW2DSelTSFoundT_Layer2 = nullptr; /**< Found time of selected track segments in layer 2 */
    TH1F* m_neuroSWTSSW2DSelTSFoundT_Layer3 = nullptr; /**< Found time of selected track segments in layer 3 */
    TH1F* m_neuroSWTSSW2DSelTSFoundT_Layer4 = nullptr; /**< Found time of selected track segments in layer 4 */
    TH1F* m_neuroSWTSSW2DSelTSFoundT_Layer5 = nullptr; /**< Found time of selected track segments in layer 5 */
    TH1F* m_neuroSWTSSW2DSelTSFoundT_Layer6 = nullptr; /**< Found time of selected track segments in layer 6 */
    TH1F* m_neuroSWTSSW2DSelTSFoundT_Layer7 = nullptr; /**< Found time of selected track segments in layer 7 */
    TH1F* m_neuroSWTSSW2DSelTSFoundT_Layer8 = nullptr; /**< Found time of selected track segments in layer 8 */

    TH1F* m_neuroSWTSSW2DSelTSPrioB_Layer0 = nullptr;  /**< Priority bits of track segments in layer 0 */
    TH1F* m_neuroSWTSSW2DSelTSPrioB_Layer1 = nullptr;  /**< Priority bits of track segments in layer 1 */
    TH1F* m_neuroSWTSSW2DSelTSPrioB_Layer2 = nullptr;  /**< Priority bits of track segments in layer 2 */
    TH1F* m_neuroSWTSSW2DSelTSPrioB_Layer3 = nullptr;  /**< Priority bits of track segments in layer 3 */
    TH1F* m_neuroSWTSSW2DSelTSPrioB_Layer4 = nullptr;  /**< Priority bits of track segments in layer 4 */
    TH1F* m_neuroSWTSSW2DSelTSPrioB_Layer5 = nullptr;  /**< Priority bits of track segments in layer 5 */
    TH1F* m_neuroSWTSSW2DSelTSPrioB_Layer6 = nullptr;  /**< Priority bits of track segments in layer 6 */
    TH1F* m_neuroSWTSSW2DSelTSPrioB_Layer7 = nullptr;  /**< Priority bits of track segments in layer 7 */
    TH1F* m_neuroSWTSSW2DSelTSPrioB_Layer8 = nullptr;  /**< Priority bits of track segments in layer 8 */

    TH1F* m_neuroSWTSSW2DSelTSLR_Layer0 = nullptr;     /**< Left/Right of track segments in layer 0 */
    TH1F* m_neuroSWTSSW2DSelTSLR_Layer1 = nullptr;     /**< Left/Right of track segments in layer 1 */
    TH1F* m_neuroSWTSSW2DSelTSLR_Layer2 = nullptr;     /**< Left/Right of track segments in layer 2 */
    TH1F* m_neuroSWTSSW2DSelTSLR_Layer3 = nullptr;     /**< Left/Right of track segments in layer 3 */
    TH1F* m_neuroSWTSSW2DSelTSLR_Layer4 = nullptr;     /**< Left/Right of track segments in layer 4 */
    TH1F* m_neuroSWTSSW2DSelTSLR_Layer5 = nullptr;     /**< Left/Right of track segments in layer 5 */
    TH1F* m_neuroSWTSSW2DSelTSLR_Layer6 = nullptr;     /**< Left/Right of track segments in layer 6 */
    TH1F* m_neuroSWTSSW2DSelTSLR_Layer7 = nullptr;     /**< Left/Right of track segments in layer 7 */
    TH1F* m_neuroSWTSSW2DSelTSLR_Layer8 = nullptr;     /**< Left/Right of track segments in layer 8 */


    // hw input vector
    TH1F* m_neuroHWInputID_Layer0 = nullptr;           /**< unpacked id input in layer 0 */
    TH1F* m_neuroHWInputT_Layer0 = nullptr;            /**< unpacked time input in layer 0 */
    TH1F* m_neuroHWInputAlpha_Layer0 = nullptr;        /**< unpacked alpha input in layer 0 */
    TH1F* m_neuroHWInputID_Layer1 = nullptr;           /**< unpacked id input in layer 1 */
    TH1F* m_neuroHWInputT_Layer1 = nullptr;            /**< unpacked time input in layer 1 */
    TH1F* m_neuroHWInputAlpha_Layer1 = nullptr;        /**< unpacked alpha input in layer 1 */
    TH1F* m_neuroHWInputID_Layer2 = nullptr;           /**< unpacked id input in layer 2 */
    TH1F* m_neuroHWInputT_Layer2 = nullptr;            /**< unpacked time input in layer 2 */
    TH1F* m_neuroHWInputAlpha_Layer2 = nullptr;        /**< unpacked alpha input in layer 2 */
    TH1F* m_neuroHWInputID_Layer3 = nullptr;           /**< unpacked id input in layer 3 */
    TH1F* m_neuroHWInputT_Layer3 = nullptr;            /**< unpacked time input in layer 3 */
    TH1F* m_neuroHWInputAlpha_Layer3 = nullptr;        /**< unpacked alpha input in layer 3 */
    TH1F* m_neuroHWInputID_Layer4 = nullptr;           /**< unpacked id input in layer 4 */
    TH1F* m_neuroHWInputT_Layer4 = nullptr;            /**< unpacked time input in layer 4 */
    TH1F* m_neuroHWInputAlpha_Layer4 = nullptr;        /**< unpacked alpha input in layer 4 */
    TH1F* m_neuroHWInputID_Layer5 = nullptr;           /**< unpacked id input in layer 5 */
    TH1F* m_neuroHWInputT_Layer5 = nullptr;            /**< unpacked time input in layer 5 */
    TH1F* m_neuroHWInputAlpha_Layer5 = nullptr;        /**< unpacked alpha input in layer 5 */
    TH1F* m_neuroHWInputID_Layer6 = nullptr;           /**< unpacked id input in layer 6 */
    TH1F* m_neuroHWInputT_Layer6 = nullptr;            /**< unpacked time input in layer 6 */
    TH1F* m_neuroHWInputAlpha_Layer6 = nullptr;        /**< unpacked alpha input in layer 6 */
    TH1F* m_neuroHWInputID_Layer7 = nullptr;           /**< unpacked id input in layer 7 */
    TH1F* m_neuroHWInputT_Layer7 = nullptr;            /**< unpacked time input in layer 7 */
    TH1F* m_neuroHWInputAlpha_Layer7 = nullptr;        /**< unpacked alpha input in layer 7 */
    TH1F* m_neuroHWInputID_Layer8 = nullptr;           /**< unpacked id input in layer 8 */
    TH1F* m_neuroHWInputT_Layer8 = nullptr;            /**< unpacked time input in layer 8 */
    TH1F* m_neuroHWInputAlpha_Layer8 = nullptr;        /**< unpacked alpha input in layer 8 */

    // sw input vector (hw TS hw 2D)
    TH1F* m_neuroSWInputID_Layer0 = nullptr;           /**< simulated id input in layer 0 */
    TH1F* m_neuroSWInputT_Layer0 = nullptr;            /**< simulated time input in layer 0 */
    TH1F* m_neuroSWInputAlpha_Layer0 = nullptr;        /**< simulated alpha input in layer 0 */
    TH1F* m_neuroSWInputID_Layer1 = nullptr;           /**< simulated id input in layer 1 */
    TH1F* m_neuroSWInputT_Layer1 = nullptr;            /**< simulated time input in layer 1 */
    TH1F* m_neuroSWInputAlpha_Layer1 = nullptr;        /**< simulated alpha input in layer 1 */
    TH1F* m_neuroSWInputID_Layer2 = nullptr;           /**< simulated id input in layer 2 */
    TH1F* m_neuroSWInputT_Layer2 = nullptr;            /**< simulated time input in layer 2 */
    TH1F* m_neuroSWInputAlpha_Layer2 = nullptr;        /**< simulated alpha input in layer 2 */
    TH1F* m_neuroSWInputID_Layer3 = nullptr;           /**< simulated id input in layer 3 */
    TH1F* m_neuroSWInputT_Layer3 = nullptr;            /**< simulated time input in layer 3 */
    TH1F* m_neuroSWInputAlpha_Layer3 = nullptr;        /**< simulated alpha input in layer 3 */
    TH1F* m_neuroSWInputID_Layer4 = nullptr;           /**< simulated id input in layer 4 */
    TH1F* m_neuroSWInputT_Layer4 = nullptr;            /**< simulated time input in layer 4 */
    TH1F* m_neuroSWInputAlpha_Layer4 = nullptr;        /**< simulated alpha input in layer 4 */
    TH1F* m_neuroSWInputID_Layer5 = nullptr;           /**< simulated id input in layer 5 */
    TH1F* m_neuroSWInputT_Layer5 = nullptr;            /**< simulated time input in layer 5 */
    TH1F* m_neuroSWInputAlpha_Layer5 = nullptr;        /**< simulated alpha input in layer 5 */
    TH1F* m_neuroSWInputID_Layer6 = nullptr;           /**< simulated id input in layer 6 */
    TH1F* m_neuroSWInputT_Layer6 = nullptr;            /**< simulated time input in layer 6 */
    TH1F* m_neuroSWInputAlpha_Layer6 = nullptr;        /**< simulated alpha input in layer 6 */
    TH1F* m_neuroSWInputID_Layer7 = nullptr;           /**< simulated id input in layer 7 */
    TH1F* m_neuroSWInputT_Layer7 = nullptr;            /**< simulated time input in layer 7 */
    TH1F* m_neuroSWInputAlpha_Layer7 = nullptr;        /**< simulated alpha input in layer 7 */
    TH1F* m_neuroSWInputID_Layer8 = nullptr;           /**< simulated id input in layer 8 */
    TH1F* m_neuroSWInputT_Layer8 = nullptr;            /**< simulated time input in layer 8 */
    TH1F* m_neuroSWInputAlpha_Layer8 = nullptr;        /**< simulated alpha input in layer 8 */

    // sw input vector (sw TS sw 2D)
    TH1F* m_neuroSWTSSW2DInputID_Layer0 = nullptr;     /**< simulated id input in layer 0 */
    TH1F* m_neuroSWTSSW2DInputT_Layer0 = nullptr;      /**< simulated time input in layer 0 */
    TH1F* m_neuroSWTSSW2DInputAlpha_Layer0 = nullptr;  /**< simulated alpha input in layer 0 */
    TH1F* m_neuroSWTSSW2DInputID_Layer1 = nullptr;     /**< simulated id input in layer 1 */
    TH1F* m_neuroSWTSSW2DInputT_Layer1 = nullptr;      /**< simulated time input in layer 1 */
    TH1F* m_neuroSWTSSW2DInputAlpha_Layer1 = nullptr;  /**< simulated alpha input in layer 1 */
    TH1F* m_neuroSWTSSW2DInputID_Layer2 = nullptr;     /**< simulated id input in layer 2 */
    TH1F* m_neuroSWTSSW2DInputT_Layer2 = nullptr;      /**< simulated time input in layer 2 */
    TH1F* m_neuroSWTSSW2DInputAlpha_Layer2 = nullptr;  /**< simulated alpha input in layer 2 */
    TH1F* m_neuroSWTSSW2DInputID_Layer3 = nullptr;     /**< simulated id input in layer 3 */
    TH1F* m_neuroSWTSSW2DInputT_Layer3 = nullptr;      /**< simulated time input in layer 3 */
    TH1F* m_neuroSWTSSW2DInputAlpha_Layer3 = nullptr;  /**< simulated alpha input in layer 3 */
    TH1F* m_neuroSWTSSW2DInputID_Layer4 = nullptr;     /**< simulated id input in layer 4 */
    TH1F* m_neuroSWTSSW2DInputT_Layer4 = nullptr;      /**< simulated time input in layer 4 */
    TH1F* m_neuroSWTSSW2DInputAlpha_Layer4 = nullptr;  /**< simulated alpha input in layer 4 */
    TH1F* m_neuroSWTSSW2DInputID_Layer5 = nullptr;     /**< simulated id input in layer 5 */
    TH1F* m_neuroSWTSSW2DInputT_Layer5 = nullptr;      /**< simulated time input in layer 5 */
    TH1F* m_neuroSWTSSW2DInputAlpha_Layer5 = nullptr;  /**< simulated alpha input in layer 5 */
    TH1F* m_neuroSWTSSW2DInputID_Layer6 = nullptr;     /**< simulated id input in layer 6 */
    TH1F* m_neuroSWTSSW2DInputT_Layer6 = nullptr;      /**< simulated time input in layer 6 */
    TH1F* m_neuroSWTSSW2DInputAlpha_Layer6 = nullptr;  /**< simulated alpha input in layer 6 */
    TH1F* m_neuroSWTSSW2DInputID_Layer7 = nullptr;     /**< simulated id input in layer 7 */
    TH1F* m_neuroSWTSSW2DInputT_Layer7 = nullptr;      /**< simulated time input in layer 7 */
    TH1F* m_neuroSWTSSW2DInputAlpha_Layer7 = nullptr;  /**< simulated alpha input in layer 7 */
    TH1F* m_neuroSWTSSW2DInputID_Layer8 = nullptr;     /**< simulated id input in layer 8 */
    TH1F* m_neuroSWTSSW2DInputT_Layer8 = nullptr;      /**< simulated time input in layer 8 */
    TH1F* m_neuroSWTSSW2DInputAlpha_Layer8 = nullptr;  /**< simulated alpha input in layer 8 */


    // reco
    TH1F* m_RecoZ = nullptr;                     /**< reconstructed z */
    TH1F* m_RecoCosTheta = nullptr;              /**< reconstructed cos(theta) */
    TH1F* m_RecoInvPt = nullptr;                 /**< reconstructed inverse Pt */
    TH1F* m_RecoPhi = nullptr;                   /**< reconstructed phi */
    TH1F* m_RecoD0 = nullptr;                    /**< reconstructed d0 */
    TH1F* m_RecoTrackCount = nullptr;            /**< number of reconstructed tracks per event */

    // reco values for tracks matched to hw NN tracks
    TH1F* m_RecoHWZ = nullptr;                   /**< matched to HW reconstructed z */
    TH1F* m_RecoHWCosTheta = nullptr;            /**< matched to HW reconstructed cos(theta) */
    TH1F* m_RecoHWInvPt = nullptr;               /**< matched to HW reconstructed inverse Pt */
    TH1F* m_RecoHWPhi = nullptr;                 /**< matched to HW reconstructed phi */
    TH1F* m_RecoHWD0 = nullptr;                  /**< matched to HW reconstructed d0 */
    TH2F* m_RecoHWZScatter = nullptr;            /**< matched to HW reconstructed z scatter plot*/

    // hw neuro values for tracks matched to reco tracks
    TH1F* m_neuroRecoHWOutZ = nullptr;           /**< reco matched z distribution from unpacker */
    TH1F* m_neuroRecoHWOutCosTheta = nullptr;    /**< reco matched cos theta distribution from unpacker */
    TH1F* m_neuroRecoHWOutInvPt = nullptr;       /**< reco matched Inverse Pt distribution from unpacker */
    TH1F* m_neuroRecoHWOutPhi0 = nullptr;        /**< reco matched phi distribution from unpacker */
    TH1F* m_neuroRecoHWOutHitPattern = nullptr;  /**< reco matched stereo hit pattern from unpacker */
    TH1F* m_neuroRecoHWOutTrackCount = nullptr;  /**< reco matched number of unpacked and matched tracks per event */
    TH1F* m_neuroRecoHWSector = nullptr;         /**< reco matched NN sector from unpacker */

    // hw accuracy
    TH1F* m_DeltaRecoHWZ = nullptr;              /**< matched to HW reconstructed z */
    TH1F* m_DeltaRecoHWCosTheta = nullptr;       /**< matched to HW reconstructed cos(theta) */
    TH1F* m_DeltaRecoHWInvPt = nullptr;          /**< matched to HW reconstructed inverse Pt */
    TH1F* m_DeltaRecoHWPhi = nullptr;            /**< matched to HW reconstructed phi */

    // reco values for tracks matched to sw NN tracks (hw TS, hw 2D)
    TH1F* m_RecoSWZ = nullptr;                   /**< matched to SW reconstructed z */
    TH1F* m_RecoSWCosTheta = nullptr;            /**< matched to SW reconstructed cos(theta) */
    TH1F* m_RecoSWInvPt = nullptr;               /**< matched to SW reconstructed inverse Pt */
    TH1F* m_RecoSWPhi = nullptr;                 /**< matched to SW reconstructed phi */
    TH1F* m_RecoSWD0 = nullptr;                  /**< matched to SW reconstructed d0 */
    TH2F* m_RecoSWZScatter = nullptr;            /**< matched to SW reconstructed z scatter plot*/

    // sw neuro values for tracks matched to reco tracks
    TH1F* m_neuroRecoSWOutZ = nullptr;           /**< reco matched z distribution from simulation (hw TS hw 2D sw NN) */
    TH1F* m_neuroRecoSWOutCosTheta = nullptr;    /**< reco matched cos theta distribution from simulation (hw TS hw 2D sw NN) */
    TH1F* m_neuroRecoSWOutInvPt = nullptr;       /**< reco matched Inverse Pt distribution from simulation (hw TS hw 2D sw NN) */
    TH1F* m_neuroRecoSWOutPhi0 = nullptr;        /**< reco matched phi distribution from simulation (hw TS hw 2D sw NN) */
    TH1F* m_neuroRecoSWOutHitPattern = nullptr;  /**< reco matched stereo hit pattern of simulated neuro tracks (hw TS hw 2D sw NN) */
    TH1F* m_neuroRecoSWOutTrackCount = nullptr;  /**< reco matched number of simulated tracks per event (hw TS hw 2D sw NN) */
    TH1F* m_neuroRecoSWSector = nullptr;         /**< reco matched NN sector from simulation (hw TS hw 2D sw NN) */

    // sw accuracy (hw TS, hw 2D)
    TH1F* m_DeltaRecoSWZ = nullptr;              /**< matched to SW reconstructed z */
    TH1F* m_DeltaRecoSWCosTheta = nullptr;       /**< matched to SW reconstructed cos(theta) */
    TH1F* m_DeltaRecoSWInvPt = nullptr;          /**< matched to SW reconstructed inverse Pt */
    TH1F* m_DeltaRecoSWPhi = nullptr;            /**< matched to SW reconstructed phi */

    // reco values for tracks matched to sw NN tracks (sw TS, sw 2D)
    TH1F* m_RecoSWTSSW2DZ = nullptr;             /**< matched to SWTSSW2DSWNN reconstructed z */
    TH1F* m_RecoSWTSSW2DCosTheta = nullptr;      /**< matched to SWTSSW2DSWNN reconstructed cos(theta) */
    TH1F* m_RecoSWTSSW2DInvPt = nullptr;         /**< matched to SWTSSW2DSWNN reconstructed inverse Pt */
    TH1F* m_RecoSWTSSW2DPhi = nullptr;           /**< matched to SWTSSW2DSWNN reconstructed phi */
    TH1F* m_RecoSWTSSW2DD0 = nullptr;            /**< matched to SWTSSW2DSWNN reconstructed d0 */
    TH2F* m_RecoSWTSSW2DZScatter = nullptr;      /**< matched to SWTSSW2DSWNN reconstructed z scatter plot*/

    // sw neuro values for tracks matched to reco tracks (sw TS, sw 2D)
    TH1F* m_neuroRecoSWTSSW2DOutZ = nullptr;           /**< reco matched z distribution from simulation (sw TS sw 2D sw NN) */
    TH1F* m_neuroRecoSWTSSW2DOutCosTheta = nullptr;    /**< reco matched cos theta distribution from simulation (sw TS sw 2D sw NN) */
    TH1F* m_neuroRecoSWTSSW2DOutInvPt = nullptr;       /**< reco matched Inverse Pt distribution from simulation (sw TS sw 2D sw NN) */
    TH1F* m_neuroRecoSWTSSW2DOutPhi0 = nullptr;        /**< reco matched phi distribution from simulation (sw TS sw 2D sw NN) */
    TH1F* m_neuroRecoSWTSSW2DOutHitPattern =
      nullptr;  /**< reco matched stereo hit pattern of simulated neuro tracks (sw TS sw 2D sw NN) */
    TH1F* m_neuroRecoSWTSSW2DOutTrackCount = nullptr;  /**< reco matched number of simulated tracks per event (sw TS sw 2D sw NN) */
    TH1F* m_neuroRecoSWTSSW2DSector = nullptr;         /**< reco matched NN sector from simulation (sw TS sw 2D sw NN) */

    // sw accuracy (sw TS, sw 2D)
    TH1F* m_DeltaRecoSWTSSW2DZ = nullptr;        /**< matched to SWTSSW2DSWNN reconstructed z */
    TH1F* m_DeltaRecoSWTSSW2DCosTheta = nullptr; /**< matched to SWTSSW2DSWNN reconstructed cos(theta) */
    TH1F* m_DeltaRecoSWTSSW2DInvPt = nullptr;    /**< matched to SWTSSW2DSWNN reconstructed inverse Pt */
    TH1F* m_DeltaRecoSWTSSW2DPhi = nullptr;      /**< matched to SWTSSW2DSWNN reconstructed phi */
  };

}

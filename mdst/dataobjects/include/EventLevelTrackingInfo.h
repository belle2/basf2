/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tracking Group 2017                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <mdst/dataobjects/HitPatternCDC.h>

#include <framework/datastore/RelationsObject.h>
#include <framework/logging/Logger.h>

#include <algorithm>
#include <bitset>

namespace Belle2 {
  /** Tracking-related info on event-level, for example number of unassigned measurements.
   *
   *  This kind of information has various use-cases, e.g.
   *  - a large number of unassigned measurements can be a hint for unreconstructed charged particles;
   *  - a check, if background conditions are similar to e.g. off-resonance data etc. is possible;
   *  - possibly further merging, clone-removal, resolution estimation depending on this info may make sense;
   */
  class EventLevelTrackingInfo : public TObject {
  private:
    /** Enum to specify meaning of bits in m_flagBlock bitset. */
    enum TrackingErrorFlags {
      c_UnspecifiedError = 0, /**< Indicating abortion of the VXDTF2 due to high combinatorics in the event. */
      c_VXDTF2Abortion = 1, /**< Indicating abortion of the VXDTF2 due to high combinatorics in the event. */
    };

  public:
    //--- CDC related Stuff -------------------------------------------------------------------------------------------
    /** Getter for number of CDC measurements, that are not assigned to any Track. */
    uint16_t getNCDCHitsNotAssigned() const
    {
      return m_nCDCHitsNotAssigned;
    }

    /** Setter for number of CDC measurements, that are not assigned to any Track. */
    void setNCDCHitsNotAssigned(uint16_t const nCDCHitsNotAssigned)
    {
      m_nCDCHitsNotAssigned = nCDCHitsNotAssigned;
    }

    /** Getter for number of CDC measurements, that are not assigned to any Track nor very likely beam-background.
      *
      *  CDC hits, that are close to each other are combined in clusters during the reconstruction,
      *  which are evaluated with respect to the likelihood to stem from beam-background and the like
      *  rather than from an interesting physics object.
      *  Before any real attempt to actually combine CDC hits into a track, a cut on the output of such a classifier
      *  is performed. Here we want to get only the number of those CDC hits, that survive that cut,
      *  but are then not used for an actual Track.
      */
    uint16_t getNCDCHitsNotAssignedPostCleaning() const
    {
      return m_nCDCHitsNotAssignedPostCleaning;
    }

    /** Setter for number of CDC measurements, that are not assigned to any Track nor very likely beam-background. */
    void setNCDCHitsNotAssignedPostCleaning(uint16_t const nCDCHitsNotAssignedPostCleaning)
    {
      m_nCDCHitsNotAssignedPostCleaning = nCDCHitsNotAssignedPostCleaning;
    }

    /** Getter for presence of hit in specific CDC Layer.
     *
     *  This information refers as well to the cleaned hits.
     *  @param  cdcLayer  Specification, which layer in the CDC shall be tested for a hit.
     *  @return true, if a non-assigned hit exists in the specified layer.
     */
    bool hasCDCLayer(uint16_t const cdcLayer) const
    {
      return HitPatternCDC(m_hitPatternCDCInitializer).hasLayer(cdcLayer);
    }

    /** Setter for presence of hit in specific CDC Layer. */
    void setCDCLayer(uint16_t const cdcLayer)
    {
      HitPatternCDC hitPatternCDC(m_hitPatternCDCInitializer);
      hitPatternCDC.setLayer(cdcLayer);
      m_hitPatternCDCInitializer = hitPatternCDC.getInteger();
    }

    /** Getter for the presence of hit in a SuperLayer. */
    bool hasCDCSLayer(uint16_t const cdcSLayer) const
    {
      return HitPatternCDC(m_hitPatternCDCInitializer).hasSLayer(cdcSLayer);
    }

    /** Getter for number of segments not used in Tracks.
     *
     *  During the CDC track finding, we search for segments within one superlayer.
     *  We want to store the number of segments, that we couldn't attach to any Track.
     *  The maximum number of unstored segments is 255.
     */
    uint16_t getNCDCSegments() const
    {
      return HitPatternCDC(m_hitPatternCDCInitializer).getNHits();
    }

    /** Setter for number of Segments not used in Tracks. */
    void setNCDCSegments(uint16_t nHits)
    {
      HitPatternCDC hitPatternCDC(m_hitPatternCDCInitializer);
      hitPatternCDC.setNHits(nHits);
      m_hitPatternCDCInitializer = hitPatternCDC.getInteger();
    }

    //--- VXD related stuff -------------------------------------------------------------------------------------------
    /** Getter for number of clusters in a specific VXD layer, SVD separated by direction.
     *
     *  All clusters, that have been part of a RecoTrack, that was converted into a Track, are removed.
     *  Only clusters, that in principle are considered valid, e.g. that are compatible with the event T0
     *  are counted. The maximum number of clusters per layer and direction, that are considered in the SVD
     *  is 255.
     *  @param layer  1 to 6 for respective VXD layer for which you want to have the remaining clusters.
     *  @param isU    only used for layers 3 to 6, set true for u direction, false for v direction.
     */
    uint16_t getNVXDClustersInLayer(uint16_t const layer, bool const isU)
    {
      if (layer == 1 or layer == 2) {
        return m_nPXDClusters[layer - 1];
      }
      if (layer > 2 and layer < 7) {
        return m_nSVDClusters[layer - 3 + 4 * isU];
      }
      B2FATAL("The VXD only has layer 1 to 6, but you asked for " << layer);
    }

    /** Getter for number of clusters in specific VXD layer, SVD directions are accumulated.
     *
     *  @param layer  1 to 6 for respective VXD layer, for which you want to have the remaining clusters.
     */
    uint16_t getNVXDClustersInLayer(uint16_t const layer)
    {
      if (layer == 1 or layer == 2) {
        return getNVXDClustersInLayer(layer, true);
      }
      return getNVXDClustersInLayer(layer, true) + getNVXDClustersInLayer(layer, false);
    }

    /** Setter for number of clusters in specific VXD layer, SVD directions are separated. */
    void setNVXDClustersInLayer(uint16_t const layer, bool const isU, uint16_t const nClusters)
    {
      if (layer == 1 or layer == 2) {
        m_nPXDClusters[layer - 1] = nClusters;
        return;
      }
      if (layer > 2 and layer < 7) {
        m_nSVDClusters[layer - 3 + 4 * isU] = std::min(nClusters, static_cast<unsigned short>(255U));
        return;
      }
      B2FATAL("The VXD only has layer 1 to 6, but you asked for " << layer);
    }

    /** Getter for time of first SVD sample relative to event T0.
     *
     *  An additional hint on the quality of the SVD reconstruction can be the time,
     *  at which the first sample point for the shape estimation was taken relative to the true
     *  T0 of the event.
     *  We assume, that the event type etc. that ultimately determines the total number of samples,
     *  that were taken, can be gotten from elsewhere.
     *  The minimum and maximum are -128 ns and 127 ns.
     */
    int8_t getSVDFirstSampleTime() const
    {
      return m_sampleTime;
    }

    /** Setter for time of first SVD sample relatvie to event T0. */
    void setSVDFirstSampleTime(int8_t const sampleTime)
    {
      m_sampleTime = sampleTime;
    }

    //--- Flag Block related stuff ------------------------------------------------------------------------------------
    /** Setter and getters for hints of track finding failure.
     *
     *  If we have a reason to assume, that there was a track in the event, that we didn't find or the
     *  track finding process was affected in any way we set a flag, that might be useful for veto purposes.
     */

    /** Check if any flag was set in the event. */
    bool getIfAnyError() const
    {
      return (m_flagBlock.count() > 0);
    }

    /** Getter for unspecified hint of track finding failure. */
    bool getHintForTrackFindingFailure() const
    {
      return m_flagBlock[c_UnspecifiedError];
    }

    /** Setter for unspecified hint of track finding failure. */
    void setHintForTrackFindingFailure()
    {
      m_flagBlock.set(c_UnspecifiedError);
    }

    /** Getter for flag indicating that the VXDTF2 was aborted due to high combinatorics in the event. */
    bool getVXDTF2AbortionFlag() const
    {
      return m_flagBlock[c_VXDTF2Abortion];
    }

    /** Setter for flag indicating that the VXDTF2 was aborted due to high combinatorics in the event. */
    void setVXDTF2AbortionFlag()
    {
      m_flagBlock.set(c_VXDTF2Abortion);
    }

  private:
    /** Number of hits in the CDC, that were not assigned to any Track.
     *
     *  HitPatternCDC saves only up to 255 hits, as its primary use-case is the Track, but
     *  we we can easily have a larger number of hits, that are not assigned. Therefore,
     *  we save this as a separate number.
     */
    uint16_t m_nCDCHitsNotAssigned {0};

    /** Number of unassigned hits in the CDC, that survived the background filter.
     *
     *  During the pattern recognition, a cleaning of hits, that are very likely due to background
     *  is performed. Here we save only the number of unassinged hits, that survive that cut.
     */
    uint16_t m_nCDCHitsNotAssignedPostCleaning {0};

    /** Efficient way of storing, which layers in the CDC have at least a single hit.
      *
      *  Using indirectly the HitPatternCDC. However, as that object has internal data, that isn't
      *  supposed to be written out, we only store an initalizer integer.
      *  As we are expecting a larger number of hits, some functionality can't be used and
      *  the corresponding information is handled with speparate members.
      *  Instead we store the number of segments into the bits reserved for number of hits in the Track.
      *
      *  @sa HitPatternCDC
      */
    uint64_t m_hitPatternCDCInitializer {0};

    /** Storage for number of clusters in the PXD.
     *
     *  We can store up to 65'535 cluster per layer here. This is slightly above the hardware readout limit,
     *  and therefore sufficient.
     */
    uint16_t m_nPXDClusters[2] {0, 0};

    /** Storage for number of clusters in the SVD
     *
     *  We can store up to 255 clusters per layer and direction.
     *  Typically events are much below that, if not, the event is a noisy one.
     *  The first 4 numbers refer to u-direction strips, the second 4 to the v-direction.
     */
    uint8_t m_nSVDClusters[8] {0, 0, 0, 0, 0, 0, 0, 0};

    /** storage for time of first SVD sample.*/
    int8_t m_sampleTime {0};

    /** Set of further flags useful in the context of tracking reconstruction. */
    std::bitset<16> m_flagBlock;

    ClassDef(EventLevelTrackingInfo, 2); /**< ROOTification. */
  };
}

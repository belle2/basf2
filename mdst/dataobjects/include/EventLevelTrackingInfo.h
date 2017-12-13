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

namespace Belle2 {
  /** Tracking-related info on event-level, for example number of unassigned measurements.
   *
   *  This kind of information has various use-cases, e.g.
   *  - a large number of unassigned measurements can be a hint for unreconstructed charged particles;
   *  - a check, if background conditions are similar to e.g. off-resonance data etc. is possible;
   *  - possibly further merging, clone-removal, resolution estimation depending on this info may make sense;
   */
  class EventLevelTrackingInfo : public RelationsObject {
  public:
    //--- CDC related Stuff -------------------------------------------------------------------------------------------
    /** Getter for number of CDC measurements, that are not assigned to any Track. */
    unsigned short getNHitsNotAssigned() const
    {
      return m_nCDCHitsNotAssigned;
    }

    /** Setter for number of CDC measurements, that are not assigned to any Track. */
    void setNHitsNotAssigned(unsigned short nCDCHitsNotAssigned)
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
    unsigned short getNHitsNotAssignedPostCleaning() const
    {
      return m_nCDCHitsNotAssignedPostCleaning;
    }

    /** Setter for number of CDC measurements, that are not assigned to any Track nor very likely beam-background. */
    void setNHitsNotAssignedPostCleaning(unsigned short nCDCHitsNotAssignedPostCleaning)
    {
      m_nCDCHitsNotAssignedPostCleaning = nCDCHitsNotAssignedPostCleaning;
    }

    /** Getter for presence of hit in specific CDC Layer.
     *
     *  This information refers as well to the cleaned hits.
     *  @param  cdcLayer  Specification, which layer in the CDC shall be tested for a hit.
     *  @return true, if a non-assigned hit exists in the specified layer.
     */
    bool hasLayer(unsigned short cdcLayer) const
    {
      return HitPatternCDC(m_hitPatternCDCInitializer).hasLayer(cdcLayer);
    }

    /** Setter for presence of hit in specific CDC Layer. */
    void setLayer(unsigned short cdcLayer)
    {
      HitPatternCDC hitPatternCDC(m_hitPatternCDCInitializer);
      hitPatternCDC.setLayer(cdcLayer);
      m_hitPatternCDCInitializer = hitPatternCDC.getInteger();
    }

    /** Getter for the presence of hit in a SuperLayer. */
    bool hasSLayer(unsigned short cdcSLayer) const
    {
      return HitPatternCDC(m_hitPatternCDCInitializer).hasSLayer(cdcSLayer);
    }

    /** Getter for number of segments not used in Tracks.
     *
     *  During the CDC track finding, we search for segments of within one superlayer.
     *  We want to store the number of segments, that we couldn't attach to any Track.
     *  The maximum number of unstored segments is 255.
     */
    unsigned short getNSegments() const
    {
      return HitPatternCDC(m_hitPatternCDCInitializer).getNHits();
    }

    /** Setter for number of Segments not used in Tracks. */
    void setNSegments(unsigned short nHits)
    {
      HitPatternCDC hitPatternCDC(m_hitPatternCDCInitializer);
      hitPatternCDC.setNHits(nHits);
      m_hitPatternCDCInitializer = hitPatternCDC.getInteger();
    }

    //--- VXD related stuff -------------------------------------------------------------------------------------------
    /** Getter for number of clusters in a specific VXD layer, SVD separated by direction.
     *
     *  All clusters, that have been part of a RecoTrack, that was converted into a Track, are removed.
     *  @param layer  1 to 6 for respective VXD layer for which you want to have the remaining clusters.
     *  @param isU    only used for layers 3 to 6, set true for u direction, false for v direction.
     */
    unsigned short getNClustersInLayer(unsigned short const layer, bool const isU)
    {
      if (layer == 1 or layer == 2) {
        return m_nPXDClusters[layer - 1];
      }
      if (layer > 2 and layer < 7) {
        return m_nSVDClusters[layer - 3 + 4 * isU];
      }
      B2FATAL("The VXD only has layer 1 to 6, but you asked for " << layer);
    }

    /** Getter for number of clsuters in specific VXD layer, SVD directions are accumulated.
     *
     *  @param layer  1 to 6 for respective VXD layer, for which you want to have the remaining clusters.
     */
    unsigned short getNClustersInLayer(unsigned short const layer)
    {
      if (layer == 1 or layer == 2) {
        return getNClustersInLayer(layer, true);
      }
      return getNClustersInLayer(layer, true) + getNClustersInLayer(layer, false);
    }

    /** Setter for number of clusters in specific VXD layer, SVD directions are separated. */
    void setNClustersInLayer(unsigned short const layer, bool const isU, unsigned short const nClusters)
    {
      if (layer == 1 or layer == 2) {
        m_nPXDClusters[layer - 1] = nClusters;
        return;
      }
      if (layer > 2 and layer < 7) {
        m_nSVDClusters[layer - 3 + 4 * isU] = nClusters;
        return;
      }
      B2FATAL("The VXD only has layer 1 to 6, but you asked for " << layer);
    }

  private:
    /** Number of hits in the CDC, that were not assigned to any Track.
     *
     *  HitPatternCDC saves only up to 255 hits, as its primary use-case is the Track, but
     *  we we can easily have a larger number of hits, that are not assigned. Therefore,
     *  we save this as a separate number.
     */
    unsigned short m_nCDCHitsNotAssigned {0};

    /** Number of unassigned hits in the CDC, that survived the background filter.
     *
     *  During the pattern recognition, a cleaning of hits, that are very likely due to background
     *  is performed. Here we save only the number of unassinged hits, that survive that cut.
     */
    unsigned short m_nCDCHitsNotAssignedPostCleaning {0};

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
     *
     */
    uint8_t m_nSVDClusters[8] {0, 0, 0, 0, 0, 0, 0, 0};

    ClassDef(EventLevelTrackingInfo, 1); /**< ROOTification. */
  };
}

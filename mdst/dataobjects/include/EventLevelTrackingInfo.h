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
    /** Setter for number of CDC measurements, that are not assigned to any Track. */
    void setNCDCHitsNotAssigned(unsigned short nCDCHitsNotAssigned)
    {
      m_nCDCHitsNotAssigned = nCDCHitsNotAssigned;
    }

    /** Getter for number of CDC measurements, that are not assigned to any Track. */
    unsigned short getNCDCHitsNotAssigned() const
    {
      return m_nCDCHitsNotAssigned;
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
    unsigned short getNCDCHitsNotAssignedPostCleaning() const
    {
      return m_nCDCHitsNotAssignedPostCleaning;
    }

    /** Setter for number of CDC measurements, that are not assigned to any Track nor very likely beam-background. */
    void setNCDCHitsNotAssignedPostCleaning(unsigned short nCDCHitsNotAssignedPostCleaning)
    {
      m_nCDCHitsNotAssignedPostCleaning = nCDCHitsNotAssignedPostCleaning;
    }

    /** Getter for presence of hit in specific CDC Layer.
     *
     *  @param  cdcLayer  Specification, which layer in the CDC shall be tested for a hit.
     *  @return true, if a non-assigned hit exists in the specified layer.
     */
    bool hasCDCLayer(unsigned short cdcLayer) const
    {
      return HitPatternCDC(m_hitPatternCDCInitializer).hasLayer(cdcLayer);
    }

    /** Setter for presence of hit in specific CDC Layer. */
    void setCDCLayer(unsigned short cdcLayer)
    {
      HitPatternCDC hitPatternCDC(m_hitPatternCDCInitializer);
      hitPatternCDC.setLayer(cdcLayer);
      m_hitPatternCDCInitializer = hitPatternCDC.getInteger();
    }

    /** Getter for the presence of hit in a SuperLayer. */
    bool hasCDCSLayer(unsigned short cdcSLayer) const
    {
      return HitPatternCDC(m_hitPatternCDCInitializer).hasSLayer(cdcSLayer);
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
      *
      *  @sa HitPatternCDC
      */
    uint64_t m_hitPatternCDCInitializer {0};

    ClassDef(EventLevelTrackingInfo, 1); /**< ROOTification. */
  };
}

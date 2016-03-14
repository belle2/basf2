/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <cdc/dataobjects/CDCHit.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <svd/dataobjects/SVDCluster.h>

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {
  /**
   * This class stores additional information to every CDC/SVD/PXD hit stored in a RecoTrack.
   * Every hit information belongs to a single hit and a single RecoTrack (stored with relation).
   * If one hit should belong to more than one track, you have to create more than one RecoHitInformation.
   *
   * The RecoHitInformation stores information on:
   *   - the TrackFinder that added the hit to the track
   *   - RL information (valid for for CDC hits only)
   *   - the sorting parameter of the hit. This is the index of this hit in the reco track.
   *   - additional flags
   *
   * The stored information can be used when transforming a RecoTrack into a genfit::Track or genfit::TrackCand
   */

  class RecoHitInformation : public RelationsObject {
  public:

    /** Define, use of clusters or true hits for SVD.
     *
     *  You have to decide, if you want to use Clusters or true hits at compile-time.
     *  In the real experiment, we want to use clusters without overhead from checking every time,
     *  if we should use true hits instead.
     */
    typedef SVDCluster UsedSVDHit;

    /** Define, use of clusters or true hits for PXD. */
    typedef PXDCluster UsedPXDHit;

    /** Define, use of CDC hits as CDC hits (for symmetry). */
    typedef CDCHit UsedCDCHit;

    /** The RightLeft information of the hit which is only valid for CDC hits */
    enum RightLeftInformation {
      undefinedRightLeftInformation,
      invalidRightLeftInformation,
      right,
      left
    };

    /** The TrackFinder which has added the hit to the track */
    enum OriginTrackFinder {
      undefinedTrackFinder,
      invalidTrackFinder,
      LegendreTrackFinder,
      LocalTrackFinder,
      SegmentTrackCombiner,
      VXDTrackFinder,
      Trasan,
      other
    };

    /** Another flag to be used (currently unused) */
    enum RecoHitFlag {
      undefinedRecoHitFlag
    };

    /** The detector this hit comes from (which is of course also visible in the hit type) */
    enum TrackingDetector {
      undefinedTrackingDetector,
      invalidTrackingDetector,
      SVD,
      PXD,
      CDC
    };

  public:
    /**
     * Empty constructor for root.
     */
    RecoHitInformation() :
      m_trackingDetector(TrackingDetector::undefinedTrackingDetector),
      m_rightLeftInformation(RightLeftInformation::undefinedRightLeftInformation),
      m_sortingParameter(0),
      m_foundByTrackFinder(OriginTrackFinder::undefinedTrackFinder),
      m_flag(RecoHitFlag::undefinedRecoHitFlag)
    {
    }

    /**
     * Create hit information for a CDC hit with the given information. Adds the relation to the hit automatically.
     * @param cdcHit The hit to create this information for.
     * @param rightLeftInformation The RL-information.
     * @param foundByTrackFinder Which track finder has found this hit?
     * @param sortingParameter The sorting parameter of this hit.
     */
    RecoHitInformation(UsedCDCHit* cdcHit, RightLeftInformation rightLeftInformation, OriginTrackFinder foundByTrackFinder,
                       unsigned int sortingParameter) :
      RecoHitInformation(cdcHit, TrackingDetector::CDC, rightLeftInformation, foundByTrackFinder, sortingParameter)
    {
    }

    /**
     * Create hit information for a PXD hit with the given information. Adds the relation to the hit automatically.
     * @param pxdHit The hit to create this information for.
     * @param rightLeftInformation The RL-information (which is probably invalid).
     * @param foundByTrackFinder Which track finder has found this hit?
     * @param sortingParameter The sorting parameter of this hit.
     */
    RecoHitInformation(UsedPXDHit* pxdHit, RightLeftInformation rightLeftInformation, OriginTrackFinder foundByTrackFinder,
                       unsigned int sortingParameter) :
      RecoHitInformation(pxdHit, TrackingDetector::PXD, rightLeftInformation, foundByTrackFinder, sortingParameter)
    {
    }

    /**
     * Create hit information for a SVD hit with the given information. Adds the relation to the hit automatically.
     * @param cdcHit The hit to create this information for.
     * @param rightLeftInformation The RL-information  (which is probably invalid).
     * @param foundByTrackFinder Which track finder has found this hit?
     * @param sortingParameter The sorting parameter of this hit.
     */
    RecoHitInformation(UsedSVDHit* svdHit, RightLeftInformation rightLeftInformation, OriginTrackFinder foundByTrackFinder,
                       unsigned int sortingParameter) :
      RecoHitInformation(svdHit, TrackingDetector::SVD, rightLeftInformation, foundByTrackFinder, sortingParameter)
    {
    }

    /** Get the additional flag */
    RecoHitFlag getFlag() const
    {
      return m_flag;
    }

    /** Set the additional flag */
    void setFlag(RecoHitFlag flag)
    {
      m_flag = flag;
    }

    /** Get which track finder has found the track. */
    OriginTrackFinder getFoundByTrackFinder() const
    {
      return m_foundByTrackFinder;
    }

    /** Set which track finder has found the track. */
    void setFoundByTrackFinder(OriginTrackFinder foundByTrackFinder)
    {
      m_foundByTrackFinder = foundByTrackFinder;
    }

    /** Get the sorting parameter */
    unsigned int getSortingParameter() const
    {
      return m_sortingParameter;
    }

    /** Set the sorting parameter */
    void setSortingParameter(unsigned int sortingParameter)
    {
      m_sortingParameter = sortingParameter;
    }

    /** Get the right-left-information. */
    RightLeftInformation getRightLeftInformation() const
    {
      return m_rightLeftInformation;
    }

    /** Set the right-left-information. */
    void setRightLeftInformation(RightLeftInformation rightLeftInformation)
    {
      m_rightLeftInformation = rightLeftInformation;
    }

    /** Get the detector this hit comes from. (can not be changed once created) */
    TrackingDetector getTrackingDetector() const
    {
      return m_trackingDetector;
    }

  private:
    /// The tracking detector this hit comes from (can not be changed once created)
    TrackingDetector m_trackingDetector;
    /// The right-left-information of the hit. Can be invalid (for VXD hits) or unknown.
    RightLeftInformation m_rightLeftInformation;
    /// The sorting parameter as an index.
    unsigned int m_sortingParameter;
    /// Which track finder has found this hit and added it to the reco track.
    /// Can only be used if creating the RecoTrack in the track finder.
    OriginTrackFinder m_foundByTrackFinder;
    /// An additional flag to be used.
    RecoHitFlag m_flag;

    /**
     * Create hit information for a generic hit with the given information. Adds the relation to the hit automatically.
     * @param hit the hit to create a reco hit information for.
     * @param trackingDetektor The detector the hit comes from.
     * @param rightLeftInformation The right left information (can be invalid)
     * @param foundByTrackFinder Which track finder has found the hit.
     * @param sortingParameter The sorting parameter of the hit.
     */
    template <class HitType>
    RecoHitInformation(HitType* hit, TrackingDetector trackingDetecktor, RightLeftInformation rightLeftInformation,
                       OriginTrackFinder foundByTrackFinder,
                       unsigned int sortingParameter) :
      m_trackingDetector(trackingDetecktor),
      m_rightLeftInformation(rightLeftInformation),
      m_sortingParameter(sortingParameter),
      m_foundByTrackFinder(foundByTrackFinder),
      m_flag(RecoHitFlag::undefinedRecoHitFlag)
    {
      addRelationTo(hit);
    }

    ClassDef(RecoHitInformation, 1); /**< This class implements additional information for hits */
  };
}

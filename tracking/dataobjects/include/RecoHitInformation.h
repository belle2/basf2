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
#include <bklm/dataobjects/BKLMHit2d.h>
#include <eklm/dataobjects/EKLMHit2d.h>

#include <framework/datastore/RelationsObject.h>

namespace genfit {
  class TrackPoint;
}

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

    /** Define, use of BKLMHit2d as BKLM hits. */
    typedef BKLMHit2d UsedBKLMHit;

    /** Define, use of EKLMHit2d as EKLM hits. */
    typedef EKLMHit2d UsedEKLMHit;

    /** The RightLeft information of the hit which is only valid for CDC hits */
    enum RightLeftInformation {
      c_undefinedRightLeftInformation,
      c_invalidRightLeftInformation,
      c_right,
      c_left
    };

    /** The TrackFinder which has added the hit to the track */
    enum OriginTrackFinder {
      c_undefinedTrackFinder,
      c_MCTrackFinder,
      c_invalidTrackFinder,
      c_LegendreTrackFinder,
      c_LocalTrackFinder,
      c_SegmentTrackCombiner,
      c_VXDTrackFinder,
      c_Trasan,
      c_other
    };

    /** Another flag to be used (currently unused) */
    enum RecoHitFlag {
      c_undefinedRecoHitFlag,
      c_dismissedByFit,
      c_pruned,
    };

    /** The detector this hit comes from (which is of course also visible in the hit type) */
    enum RecoHitDetector {
      c_undefinedTrackingDetector,
      c_invalidTrackingDetector,
      c_SVD,
      c_PXD,
      c_CDC,
      c_EKLM,
      c_BKLM
    };

  public:
    /**
     * Empty constructor for root.
     */
    RecoHitInformation() {}

    /**
     * Create hit information for a CDC hit with the given information. Adds the relation to the hit automatically.
     * @param cdcHit The hit to create this information for.
     * @param rightLeftInformation The RL-information.
     * @param foundByTrackFinder Which track finder has found this hit?
     * @param sortingParameter The sorting parameter of this hit.
     */
    RecoHitInformation(const UsedCDCHit* cdcHit, RightLeftInformation rightLeftInformation, OriginTrackFinder foundByTrackFinder,
                       unsigned int sortingParameter) :
      RecoHitInformation(cdcHit, RecoHitDetector::c_CDC, rightLeftInformation, foundByTrackFinder, sortingParameter)
    {
    }

    /**
     * Create hit information for a PXD hit with the given information. Adds the relation to the hit automatically.
     * @param pxdHit The hit to create this information for.
     * @param rightLeftInformation The RL-information (which is probably invalid).
     * @param foundByTrackFinder Which track finder has found this hit?
     * @param sortingParameter The sorting parameter of this hit.
     */
    RecoHitInformation(const UsedPXDHit* pxdHit, RightLeftInformation rightLeftInformation, OriginTrackFinder foundByTrackFinder,
                       unsigned int sortingParameter) :
      RecoHitInformation(pxdHit, RecoHitDetector::c_PXD, rightLeftInformation, foundByTrackFinder, sortingParameter)
    {
    }

    /**
     * Create hit information for a SVD hit with the given information. Adds the relation to the hit automatically.
     * @param svdHit The hit to create this information for.
     * @param rightLeftInformation The RL-information  (which is probably invalid).
     * @param foundByTrackFinder Which track finder has found this hit?
     * @param sortingParameter The sorting parameter of this hit.
     */
    RecoHitInformation(const UsedSVDHit* svdHit, RightLeftInformation rightLeftInformation, OriginTrackFinder foundByTrackFinder,
                       unsigned int sortingParameter) :
      RecoHitInformation(svdHit, RecoHitDetector::c_SVD, rightLeftInformation, foundByTrackFinder, sortingParameter)
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
    RecoHitDetector getTrackingDetector() const
    {
      return m_trackingDetector;
    }

    /** Get the flag, whether this his should be used in a fit or not. */
    bool useInFit() const
    {
      return m_useInFit;
    }

    /** Set the hit to be used (default) or not in the next fit. */
    void setUseInFit(const bool useInFit = true)
    {
      m_useInFit = useInFit;
    }

    /** Get a pointer to the TrackPoint that was created from this hit. Can be a nullptr if no measurement was already created.
     * Please be aware that refitting may or may not recreate the track points and older pointers can be invalid then.
     * Also, pruning a RecoTrack will also delete most of the TrackPoints. */
    const genfit::TrackPoint* getCreatedTrackPoint() const
    {
      return m_createdTrackPoint;
    }

    /** Set the pointer of the created track point related to this hit. */
    void setCreatedTrackPoint(const genfit::TrackPoint* createdTrackPoint)
    {
      m_createdTrackPoint = createdTrackPoint;
    }

  private:
    /// The tracking detector this hit comes from (can not be changed once created)
    RecoHitDetector m_trackingDetector = RecoHitDetector::c_undefinedTrackingDetector;
    /// The right-left-information of the hit. Can be invalid (for VXD hits) or unknown.
    RightLeftInformation m_rightLeftInformation = RightLeftInformation::c_undefinedRightLeftInformation;
    /// The sorting parameter as an index.
    unsigned int m_sortingParameter = 0;
    /// Which track finder has found this hit and added it to the reco track.
    /// Can only be used if creating the RecoTrack in the track finder.
    OriginTrackFinder m_foundByTrackFinder = OriginTrackFinder::c_undefinedTrackFinder;
    /// An additional flag to be used.
    RecoHitFlag m_flag = RecoHitFlag::c_undefinedRecoHitFlag;
    /// Set this flag to falso to not create a measurement out of this hit
    bool m_useInFit = true;
    /// A pointer to the TrackPoint for this hit.
    /// do not store to ROOT file, otherwise the *RecoHits get created when reading in the file
    const genfit::TrackPoint* m_createdTrackPoint = nullptr; //! -> this is the marker for ROOT I/O to ignore this pointer

    /**
     * Create hit information for a generic hit with the given information. Adds the relation to the hit automatically.
     * @param hit the hit to create a reco hit information for.
     * @param trackingDetector The detector the hit comes from.
     * @param rightLeftInformation The right left information (can be invalid)
     * @param foundByTrackFinder Which track finder has found the hit.
     * @param sortingParameter The sorting parameter of the hit.
     */
    template <class HitType>
    RecoHitInformation(const HitType* hit,
                       RecoHitDetector trackingDetector,
                       RightLeftInformation rightLeftInformation,
                       OriginTrackFinder foundByTrackFinder,
                       unsigned int sortingParameter) :
      m_trackingDetector(trackingDetector),
      m_rightLeftInformation(rightLeftInformation),
      m_sortingParameter(sortingParameter),
      m_foundByTrackFinder(foundByTrackFinder),
      m_flag(RecoHitFlag::c_undefinedRecoHitFlag)
    {
      addRelationTo(hit);
    }

    ClassDef(RecoHitInformation, 3); /**< This class implements additional information for hits */
  };
}

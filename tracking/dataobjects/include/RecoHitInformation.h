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
#include <TVector3.h>

namespace Belle2 {
  /**
   * This class stores additional information to every CDC/SVD/PXD hit stored in a RecoTrack.
   * Every hit information belongs to a single hit and a single RecoTrack (stored with relation).
   * If one hits should belong to more than one track, you have to create more than one RecoHitInformation.
   *
   * The RecoHitInformation stores information on:
   *   - the TrackFinder that added the hit to the track
   *   - RL information for CDC hits
   *   - the sorting parameter of the hit. This should be (something like if you can not calculate the correct value) the arc length between -pi and pi.
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

    enum RightLeftInformation {
      undefinedRightLeftInformation,
      invalidRightLeftInformation,
      right,
      left
    };

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

    enum RecoHitFlag {
      undefinedRecoHitFlag
    };

    enum TrackingDetector {
      undefinedTrackingDetector,
      invalidTrackingDetector,
      SVD,
      PCD,
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
     * Create hit information for a CDC hit with the given information.
     * @param cdcHit
     * @param rightLeftInformation
     * @param foundByTrackFinder
     * @param travelS
     * @param reconstructedPosition
     */
    RecoHitInformation(UsedCDCHit* cdcHit, RightLeftInformation rightLeftInformation, OriginTrackFinder foundByTrackFinder,
                       double sortingParameter) :
      RecoHitInformation(cdcHit, TrackingDetector::CDC, rightLeftInformation, foundByTrackFinder, sortingParameter)
    {
    }

    /**
     * Create hit information for a CDC hit with the given information.
     * @param cdcHit
     * @param rightLeftInformation
     * @param foundByTrackFinder
     * @param travelS
     * @param reconstructedPosition
     */
    RecoHitInformation(UsedPXDHit* pxdHit, RightLeftInformation rightLeftInformation, OriginTrackFinder foundByTrackFinder,
                       double sortingParameter) :
      RecoHitInformation(pxdHit, TrackingDetector::CDC, rightLeftInformation, foundByTrackFinder, sortingParameter)
    {
    }

    /**
     * Create hit information for a CDC hit with the given information.
     * @param cdcHit
     * @param rightLeftInformation
     * @param foundByTrackFinder
     * @param travelS
     * @param reconstructedPosition
     */
    RecoHitInformation(UsedSVDHit* svdHit, RightLeftInformation rightLeftInformation, OriginTrackFinder foundByTrackFinder,
                       double sortingParameter) :
      RecoHitInformation(svdHit, TrackingDetector::CDC, rightLeftInformation, foundByTrackFinder, sortingParameter)
    {
    }

    RecoHitFlag getFlag() const
    {
      return m_flag;
    }

    void setFlag(RecoHitFlag flag)
    {
      m_flag = flag;
    }

    OriginTrackFinder getFoundByTrackFinder() const
    {
      return m_foundByTrackFinder;
    }

    void setFoundByTrackFinder(OriginTrackFinder foundByTrackFinder)
    {
      m_foundByTrackFinder = foundByTrackFinder;
    }

    double getSortingParameter() const
    {
      return m_sortingParameter;
    }

    void setSortingParameter(double sortingParameter)
    {
      m_sortingParameter = sortingParameter;
    }

    RightLeftInformation getRightLeftInformation() const
    {
      return m_rightLeftInformation;
    }

    void setRightLeftInformation(RightLeftInformation rightLeftInformation)
    {
      m_rightLeftInformation = rightLeftInformation;
    }

    TrackingDetector getTrackingDetector() const
    {
      return m_trackingDetector;
    }

  private:
    TrackingDetector m_trackingDetector;
    RightLeftInformation m_rightLeftInformation;
    double m_sortingParameter; /**< From -pi to pi */
    OriginTrackFinder m_foundByTrackFinder;
    RecoHitFlag m_flag;

    /**
     * Create hit information for a generic hit hit with the given information.
     * @param hit
     * @param trackingDetektor
     * @param rightLeftInformation
     * @param foundByTrackFinder
     * @param travelS
     * @param reconstructedPosition
     */
    template <class HitType>
    RecoHitInformation(HitType* hit, TrackingDetector trackingDetecktor, RightLeftInformation rightLeftInformation,
                       OriginTrackFinder foundByTrackFinder,
                       double sortingParameter) :
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

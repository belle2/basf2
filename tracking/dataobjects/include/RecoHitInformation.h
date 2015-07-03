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

#include <framework/datastore/RelationsObject.h>
#include <framework/core/FrameworkExceptions.h>
#include <cdc/dataobjects/CDCHit.h>

#include <TVector3.h>
#include <TMath.h>
#include <assert.h>

namespace Belle2 {
  /**
   * This class stores additional information to every CDC/SVD/PXD hit stored in a RecoTrack.
   * Every hit information belongs to a single hit and a single RecoTrack (stored with relation).
   * If one hits should belong to more than one track, you have to create more than one RecoHitInformation.
   *
   * The RecoHitInformation stores information on:
   *   - the TrackFinder that added the hit to the track
   *   - RL information for CDC hits
   *   - the reconstructed s (the travel distance) on the track
   *   - additional flags
   *
   * The stored information can be used when transforming a RecoTrack into a genfit::Track or genfit::TrackCand
   */

  class RecoHitInformation : public RelationsObject {
  public:
    BELLE2_DEFINE_EXCEPTION(InvalidArcLength, "The arc length should be in the range [-pi, pi] but you gave: %1%");

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
      m_reconstructedArcLength(0),
      m_foundByTrackFinder(OriginTrackFinder::undefinedTrackFinder),
      m_flag(RecoHitFlag::undefinedRecoHitFlag)
    {}

    /**
     * Create hit information for a CDC hit with the given information.
     * @param cdcHit
     * @param rightLeftInformation
     * @param foundByTrackFinder
     * @param travelS
     * @param reconstructedPosition
     */
    RecoHitInformation(CDCHit* cdcHit, RightLeftInformation rightLeftInformation, OriginTrackFinder foundByTrackFinder,
                       double reconstructedArcLength) :
      m_trackingDetector(TrackingDetector::CDC),
      m_rightLeftInformation(rightLeftInformation),
      m_reconstructedArcLength(0),
      m_foundByTrackFinder(foundByTrackFinder),
      m_flag(RecoHitFlag::undefinedRecoHitFlag)
    {
      // we set the arc length seperately to catch the error
      setReconstructedArcLength(reconstructedArcLength);
      addRelationTo(cdcHit);
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

    double getReconstructedArcLength() const
    {
      return m_reconstructedArcLength;
    }

    void setReconstructedArcLength(double reconstructedArcLength)
    {
      if (reconstructedArcLength <= TMath::Pi() and reconstructedArcLength >= -TMath::Pi()) {
        m_reconstructedArcLength = reconstructedArcLength;
      } else {
        throw InvalidArcLength() << reconstructedArcLength;
      }
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

    void setTrackingDetector(TrackingDetector trackingDetector)
    {
      m_trackingDetector = trackingDetector;
    }

  private:
    TrackingDetector m_trackingDetector;
    RightLeftInformation m_rightLeftInformation;
    double m_reconstructedArcLength; /**< From -pi to pi */
    OriginTrackFinder m_foundByTrackFinder;
    RecoHitFlag m_flag;

    ClassDef(RecoHitInformation, 1); /**< This class implements additional information for hits */
  };
}

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

#include <cdc/dataobjects/CDCHit.h>

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
   *   - the reconstructed s (the travel distance) on the track
   *   - additional flags
   *
   * The stored information can be used when transforming a RecoTrack into a genfit::Track or genfit::TrackCand
   */

  class RecoHitInformation : public RelationsObject {
  public:
    enum RightLeftInformation {
      undefinedRightLeftInformation,
      right,
      left
    };

    enum OriginTrackFinder {
      undefinedTrackFinder,
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
      m_reconstructedTravelS(0),
      m_reconstructedPosition(),
      m_foundByTrackFinder(OriginTrackFinder::undefinedTrackFinder),
      m_flag(RecoHitFlag::undefinedRecoHitFlag)
    {}

    RecoHitInformation(CDCHit* cdcHit, RightLeftInformation rightLeftInformation, OriginTrackFinder foundByTrackFinder,
                       double travelS, const TVector3& reconstructedPosition) :
      m_trackingDetector(TrackingDetector::CDC),
      m_rightLeftInformation(rightLeftInformation),
      m_reconstructedTravelS(travelS),
      m_reconstructedPosition(reconstructedPosition),
      m_foundByTrackFinder(foundByTrackFinder),
      m_flag(RecoHitFlag::undefinedRecoHitFlag)
    {
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

    double getReconstructedTravelS() const
    {
      return m_reconstructedTravelS;
    }

    void setReconstructedTravelS(double reconstructedTravelS)
    {
      m_reconstructedTravelS = reconstructedTravelS;
    }

    const TVector3& getReconstructedPosition() const
    {
      return m_reconstructedPosition;
    }

    void setReconstructedPosition(const TVector3& reconstructedPosition)
    {
      m_reconstructedPosition = reconstructedPosition;
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
    double m_reconstructedTravelS;
    TVector3 m_reconstructedPosition;
    OriginTrackFinder m_foundByTrackFinder;
    RecoHitFlag m_flag;

    //-----------------------------------------------------------------------------------
    /** Making this class a ROOT class.*/
    ClassDef(RecoHitInformation, 1);
  };
}

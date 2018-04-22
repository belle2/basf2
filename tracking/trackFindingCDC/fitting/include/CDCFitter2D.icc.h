/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/fitting/CDCFitter2D.h>

#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>

#include <tracking/trackFindingCDC/fitting/EFitVariance.h>
#include <tracking/trackFindingCDC/fitting/EFitPos.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    template <class AFitMethod>
    CDCFitter2D<AFitMethod>::CDCFitter2D()
      : m_usePosition(true)
      , m_useOrientation(false)
      , m_fitVariance(EFitVariance::c_Proper)
    {
    }

    template <class AFitMethod>
    CDCFitter2D<AFitMethod>::~CDCFitter2D() = default;

    template <class AFitMethod>
    CDCTrajectory2D CDCFitter2D<AFitMethod>::fit(const CDCObservations2D& observations2D) const
    {
      return fit(CDCObservations2D(observations2D));
    }

    template <class AFitMethod>
    CDCTrajectory2D CDCFitter2D<AFitMethod>::fit(CDCObservations2D&& observations2D) const
    {
      CDCTrajectory2D result;
      update(result, observations2D);
      return result;
    }

    template <class AFitMethod>
    void CDCFitter2D<AFitMethod>::update(CDCTrajectory2D& trajectory2D,
                                         const CDCObservations2D& observations2D) const
    {
      return update(trajectory2D, CDCObservations2D(observations2D));
    }

    template <class AFitMethod>
    void CDCFitter2D<AFitMethod>::update(CDCTrajectory2D& trajectory2D, CDCObservations2D&& observations2D) const
    {
      AFitMethod::update(trajectory2D, observations2D);
    }

    template <class AFitMethod>
    CDCTrajectory2D CDCFitter2D<AFitMethod>::fit(const CDCTrack& track) const
    {
      return fitGeneric(track);
    }

    template <class AFitMethod>
    CDCTrajectory2D CDCFitter2D<AFitMethod>::fit(const CDCSegment3D& segment) const
    {
      return fitGeneric(segment);
    }

    template <class AFitMethod>
    CDCTrajectory2D CDCFitter2D<AFitMethod>::fit(const CDCSegment2D& segment) const
    {
      return fitGeneric(segment);
    }

    template <class AFitMethod>
    CDCTrajectory2D CDCFitter2D<AFitMethod>::fit(const std::vector<const CDCWireHit*>& wireHits) const
    {
      return fitGeneric(wireHits);
    }

    template <class AFitMethod>
    CDCTrajectory2D CDCFitter2D<AFitMethod>::fit(const std::vector<const CDCWire*>& wires) const
    {
      return fitGeneric(wires);
    }

    template <class AFitMethod>
    CDCTrajectory2D CDCFitter2D<AFitMethod>::fit(const CDCWireHitSegment& wireHits) const
    {
      return fitGeneric(wireHits);
    }

    template <class AFitMethod>
    CDCTrajectory2D CDCFitter2D<AFitMethod>::fit(const CDCSegment2D& fromSegment,
                                                 const CDCSegment2D& toSegment) const
    {
      return fitGeneric(fromSegment, toSegment);
    }

    template <class AFitMethod>
    void CDCFitter2D<AFitMethod>::update(CDCTrajectory2D& trajectory2D,
                                         const CDCSegment2D& segment) const
    {
      updateGeneric(trajectory2D, segment);
    }

    template <class AFitMethod>
    void CDCFitter2D<AFitMethod>::update(CDCTrajectory2D& trajectory2D,
                                         const CDCAxialSegmentPair& axialSegmentPair) const
    {
      return updateGeneric(trajectory2D, axialSegmentPair);
    }

    template <class AFitMethod>
    template <class AHits>
    CDCTrajectory2D CDCFitter2D<AFitMethod>::fitGeneric(const AHits& hits) const
    {
      CDCTrajectory2D result;
      updateGeneric(result, hits);
      return result;
    }

    template <class AFitMethod>
    template <class AStartHits, class AEndHits>
    CDCTrajectory2D
    CDCFitter2D<AFitMethod>::fitGeneric(const AStartHits& startHits, const AEndHits& endHits) const
    {
      CDCTrajectory2D result;
      updateGeneric(result, startHits, endHits);
      return result;
    }

    template <class AFitMethod>
    template <class AStartHits, class AEndHits>
    void CDCFitter2D<AFitMethod>::updateGeneric(CDCTrajectory2D& trajectory2D,
                                                const AStartHits& startHits,
                                                const AEndHits& endHits) const
    {
      CDCObservations2D observations2D;
      observations2D.setFitVariance(m_fitVariance);

      if (m_usePosition) {
        observations2D.setFitPos(EFitPos::c_RecoPos);
        observations2D.appendRange(startHits);
      }
      if (m_useOrientation) {
        observations2D.setFitPos(EFitPos::c_RLDriftCircle);
        observations2D.appendRange(startHits);
      }

      if (m_usePosition) {
        observations2D.setFitPos(EFitPos::c_RecoPos);
        observations2D.appendRange(endHits);
      }
      if (m_useOrientation) {
        observations2D.setFitPos(EFitPos::c_RLDriftCircle);
        observations2D.appendRange(endHits);
      }

      if (observations2D.size() < 4) {
        trajectory2D.clear();
      } else {
        AFitMethod::update(trajectory2D, observations2D);
      }
    }

    template <class AFitMethod>
    template <class AHits>
    void
    CDCFitter2D<AFitMethod>::updateGeneric(CDCTrajectory2D& trajectory2D, const AHits& hits) const
    {
      CDCObservations2D observations2D;
      observations2D.setFitVariance(m_fitVariance);

      if (m_usePosition) {
        observations2D.setFitPos(EFitPos::c_RecoPos);
        observations2D.appendRange(hits);
      }
      if (m_useOrientation) {
        observations2D.setFitPos(EFitPos::c_RLDriftCircle);
        observations2D.appendRange(hits);
      }

      if (observations2D.size() < 4) {
        trajectory2D.clear();
      } else {
        AFitMethod::update(trajectory2D, observations2D);
      }
    }

    template <class AFitMethod>
    void CDCFitter2D<AFitMethod>::useOnlyPosition()
    {
      m_usePosition = true;
      m_useOrientation = false;
    }

    template <class AFitMethod>
    void CDCFitter2D<AFitMethod>::useOnlyOrientation()
    {
      m_usePosition = false;
      m_useOrientation = true;
    }

    template <class AFitMethod>
    void CDCFitter2D<AFitMethod>::usePositionAndOrientation()
    {
      m_usePosition = true;
      m_useOrientation = true;
    }

    template <class AFitMethod>
    void CDCFitter2D<AFitMethod>::setFitVariance(EFitVariance fitVariance)
    {
      m_fitVariance = fitVariance;
    }
  }
}

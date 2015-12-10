/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
F * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>
#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    //// Fits segments with the Riemann method.
    class SegmentFitter:
      public Findlet<CDCRecoSegment2D> {

    private:
      /// Type of the base class
      typedef Findlet<CDCRecoSegment2D> Super;

    public:
      /// Short description of the findlet
      virtual std::string getDescription() override
      {
        return "Fits each segment with the Riemann method";
      }

    public:
      /// Main algorithm applying the fit to each segment
      virtual void apply(std::vector<CDCRecoSegment2D>& outputSegments) override final
      {
        for (const CDCRecoSegment2D& segment : outputSegments) {
          CDCTrajectory2D& trajectory2D = segment.getTrajectory2D();
          m_fitter.update(trajectory2D, segment);
        }

      }

    private:
      /// Instance of the fitter to be used.
      CDCRiemannFitter m_fitter;

    }; // end class
  } // end namespace TrackFindingCDC
} // end namespace Belle2

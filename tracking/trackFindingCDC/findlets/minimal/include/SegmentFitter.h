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
      /** Add the parameters of the filter to the module */
      void exposeParameters(ModuleParamList* moduleParamList) override final
      {
        moduleParamList->addParameter("FitSegments",
                                      m_param_fitSegments,
                                      "Switch to indicate if the segments shall be fitted.",
                                      bool(m_param_fitSegments));
      }

    public:
      /// Main algorithm applying the fit to each segment
      virtual void apply(std::vector<CDCRecoSegment2D>& outputSegments) override final
      {
        // Fit the segments if requested
        if (m_param_fitSegments) {
          for (const CDCRecoSegment2D& segment : outputSegments) {
            CDCTrajectory2D& trajectory2D = segment.getTrajectory2D();
            m_fitter.update(trajectory2D, segment);
          }
        }
      }

    private:
      /// Parameter: Switch if the segments shall be fitted after the generation.
      bool m_param_fitSegments = true;

      /// Instance of the fitter to be used.
      CDCRiemannFitter m_fitter;

    }; // end class
  } // end namespace TrackFindingCDC
} // end namespace Belle2

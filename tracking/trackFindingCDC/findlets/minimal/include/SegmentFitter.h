/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCKarimakiFitter.h>

#include <tracking/trackFindingCDC/fitting/EFitPos.h>
#include <tracking/trackFindingCDC/fitting/EFitVariance.h>

#include <tracking/trackFindingCDC/eventdata/utils/DriftLengthEstimator.h>

#include <vector>
#include <string>

namespace Belle2 {
  class ModuleParamList;

  namespace TrackFindingCDC {
    class CDCRecoSegment2D;

    /// Fits segments with the Riemann method.
    class SegmentFitter:
      public Findlet<CDCRecoSegment2D&> {

    private:
      /// Type of the base class
      using Super = Findlet<CDCRecoSegment2D&>;

    public:
      /// Short description of the findlet
      std::string getDescription();

      /// Add the parameters of the fitter to the module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix);

      /// Signals the beginning of the event processing
      void initialize();

    public:
      /// Main algorithm applying the fit to each segment
      void apply(std::vector<CDCRecoSegment2D>& outputSegments);

    private:
      /// Parameter : Switch to use Karimaki fit
      bool m_param_karimakiFit = false;

      /// Parameter : Option string which positional information from the hits should be used
      std::string m_param_fitPosString = "recoPos";

      /// Parameter : Option string which variance information from the hits should be used
      std::string m_param_fitVarianceString = "proper";

      /// Parameter : Switch to reestimate the drift length before the fit
      bool m_param_updateDriftLength = true;

      /// Option which positional information from the hits should be used
      EFitPos m_fitPos = EFitPos::c_RecoPos;

      /// Option which variance information from the hits should be used
      EFitVariance m_fitVariance = EFitVariance::c_Proper;

      /// Instance of the riemann fitter to be used.
      CDCRiemannFitter m_riemannFitter;

      /// Instance of the karimaki fitter to be used.
      CDCKarimakiFitter m_karimakiFitter;

      /// Instance of the drift length estimator to be used.
      DriftLengthEstimator m_driftLengthEstimator;
    };
  }
}

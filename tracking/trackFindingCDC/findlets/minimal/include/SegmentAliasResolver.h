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

#include <tracking/trackFindingCDC/eventdata/utils/DriftLengthEstimator.h>

#include <vector>
#include <string>

namespace Belle2 {
  class ModuleParamList;

  namespace TrackFindingCDC {
    class CDCRecoSegment2D;

    /// Resolves between the potential alias versions of the segments and contained hits
    class SegmentAliasResolver : public Findlet<CDCRecoSegment2D&> {

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
      /// Fit the alias segment
      void refit(CDCRecoSegment2D& segment, bool reestimate);

      /// Parameter : Which alias resolutions should be applied
      std::vector<std::string> m_param_investigate = {"full", "borders", /*"middle"*/};

    private:
      /// Switch whether the complete segment should be aliased.
      bool m_fullAlias = false; // Activated by the parameter

      /// Switch whether the border hits at the beginning and the end should be aliased.
      bool m_borderAliases = false; // Activated by the parameter

      /// Switch whether the hits in the middle with lowest drift radius should be aliased.
      bool m_middleAliases = false; // Activated by the parameter

      /// Parameter : Switch to reestimate the drift length before each fit.
      bool m_param_reestimateDriftLength = true;

      /// Parameter : Switch to restimate the reconstructed positions with a pass of facets
      bool m_param_reestimatePositions = false;

      /// Instance of the riemann fitter to be used.
      CDCRiemannFitter m_riemannFitter;

      /// Instance of the drift length estimator to be used.
      DriftLengthEstimator m_driftLengthEstimator;
    };
  }
}

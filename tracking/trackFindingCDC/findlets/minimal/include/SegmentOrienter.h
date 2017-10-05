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

#include <tracking/trackFindingCDC/findlets/minimal/EPreferredDirection.h>

#include <vector>
#include <string>

namespace Belle2 {


  namespace TrackFindingCDC {
    class CDCSegment2D;

    /// Fixes the orientation of segments by a simple heuristic
    class SegmentOrienter: public Findlet<const CDCSegment2D, CDCSegment2D> {

    private:
      /// Type of the base class
      using Super = Findlet<const CDCSegment2D, CDCSegment2D>;

    public:
      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Signals the beginning of the event processing
      void initialize() final;

      /// Main algorithm applying the adjustment of the orientation.
      void apply(const std::vector<CDCSegment2D>& inputSegments,
                 std::vector<CDCSegment2D>& outputSegments) final;

    private:
      /**
       *  Parameter: String that states the desired segment orientation.
       *  Valid orientations are "none" (unchanged), "outwards", "downwards", "symmetric", "curling"
       */
      std::string m_param_segmentOrientationString = "";

      /**
       *  Encoded desired segment orientation.
       *  Valid orientations are "c_None" (unchanged), "c_Outwards", "c_Downwards", "c_Symmetric", "c_Curling",
       */
      EPreferredDirection m_segmentOrientation = EPreferredDirection::c_None;
    };
  }
}

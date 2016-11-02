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

#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/findlets/minimal/EPreferredDirection.h>
#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Fixes the orientation of segments by a simple heuristic
    class SegmentOrienter:
      public Findlet<const CDCRecoSegment2D, CDCRecoSegment2D> {

    private:
      /// Type of the base class
      using Super = Findlet<const CDCRecoSegment2D, CDCRecoSegment2D>;

    public:
      /// Short description of the findlet
      virtual std::string getDescription() override;

      /// Add the parameters of the filter to the module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix = "") override final;

      /// Signals the beginning of the event processing
      void initialize() override;

    public:
      /**
       *  Set the default output orientation of the segments.
       *  * EPreferredDirection::c_None does not modify the orientation from the concret algorithm.
       *  * EPreferredDirection::c_Outwards flips the orientation of the segment such that they point away from the interaction point.
       *  * EPreferredDirection::c_Downwards flips the orientation of the segment such that they point downwards.
       *  * EPreferredDirection::c_Symmetric makes two copies of each segment with forward and backward to the original orientation.
       *  * EPreferredDirection::c_Curling makes two copies for segments that are likely curlers, fix others to outwards
       *
       *  This properties can also be overridden by the user by the module parameter.
       */
      void setSegmentOrientation(const EPreferredDirection& segmentOrientation);

      /// Get the current output orientation of the segments.
      EPreferredDirection getSegmentOrientation() const;

    public:
      /// Main algorithm applying the adjustment of the orientation.
      virtual void apply(const std::vector<CDCRecoSegment2D>& inputSegments,
                         std::vector<CDCRecoSegment2D>& outputSegments) override final;

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

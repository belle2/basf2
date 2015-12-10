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
      typedef Findlet<const CDCRecoSegment2D, CDCRecoSegment2D> Super;

    public:
      /// Short description of the findlet
      virtual std::string getDescription() override
      {
        return "Fixes the flight direction of segments to a preferred orientation by simple heuristics.";
      }

      /** Add the parameters of the filter to the module */
      void exposeParameters(ModuleParamList* moduleParamList) override final
      {
        moduleParamList->addParameter("SegmentOrientation",
                                      m_param_segmentOrientationString,
                                      "Option which orientation of segments shall be generate. "
                                      "Valid options are '' (default of the finder), "
                                      "'none' (one orientation, algorithm dependent), "
                                      "'symmetric', "
                                      "'outwards', "
                                      "'downwards'.",
                                      std::string(m_param_segmentOrientationString));
      }

      /// Signals the beginning of the event processing
      void initialize() override
      {
        Super::initialize();
        if (m_param_segmentOrientationString != std::string("")) {
          try {
            m_segmentOrientation = getPreferredDirection(m_param_segmentOrientationString);
          } catch (std::invalid_argument& e) {
            B2ERROR("Unexpected 'SegmentOrientation' parameter : '" << m_param_segmentOrientationString);
          }
        }
      }

    public:
      /** Set the default output orientation of the segments.
       *  * EPreferredDirection::c_None does not modify the orientation from the concret algorithm.
       *  * EPreferredDirection::c_Symmetric makes two copies of each segment with forward and backward to the original orientation.
       *  * EPreferredDirection::c_Outwards flips the orientation of the segment such that they point away from the interaction point.
       *  * EPreferredDirection::c_Downwards flips the orientation of the segment such that they point downwards.
       *
       *  This properties can also be overridden by the user by a module parameter.
       */
      void setSegmentOrientation(const EPreferredDirection& segmentOrientation)
      { m_segmentOrientation = segmentOrientation; }

      /// Get the currentl default output orientation of the segments.
      const EPreferredDirection& getSegmentOrientation() const
      { return m_segmentOrientation; }

    public:
      /// Main algorithm applying the adjustment of the orientation.
      virtual void apply(const std::vector<CDCRecoSegment2D>& inputSegments,
                         std::vector<CDCRecoSegment2D>& outputSegments) override final
      {
        /// Copy segments to output fixing their orientation
        if (m_segmentOrientation == EPreferredDirection::c_None) {
          // Copy the segments unchanged.
          outputSegments = inputSegments;
        } else if (m_segmentOrientation == EPreferredDirection::c_Symmetric) {
          outputSegments.reserve(2 * inputSegments.size());
          for (const CDCRecoSegment2D& segment : inputSegments) {
            outputSegments.push_back(segment.reversed());
            outputSegments.push_back(segment);
          }
        } else if (m_segmentOrientation == EPreferredDirection::c_Outwards) {
          outputSegments.reserve(inputSegments.size());
          for (const CDCRecoSegment2D& segment : inputSegments) {
            const CDCRecoHit2D& firstHit = segment.front();
            const CDCRecoHit2D& lastHit = segment.back();
            if (lastHit.getRecoPos2D().cylindricalR() < firstHit.getRecoPos2D().cylindricalR()) {
              outputSegments.push_back(segment.reversed());
            } else {
              outputSegments.push_back(segment);
            }
          }
        } else if (m_segmentOrientation == EPreferredDirection::c_Downwards) {
          outputSegments.reserve(inputSegments.size());
          for (const CDCRecoSegment2D& segment : inputSegments) {
            const CDCRecoHit2D& firstHit = segment.front();
            const CDCRecoHit2D& lastHit = segment.back();
            if (lastHit.getRecoPos2D().y() > firstHit.getRecoPos2D().y()) {
              outputSegments.push_back(segment.reversed());
            } else {
              outputSegments.push_back(segment);
            }
          }
        } else {
          B2WARNING("Unexpected 'SegmentOrientation' parameter of segment finder module : '" <<
                    m_param_segmentOrientationString <<
                    "'. No segments are put out.");
        }
      }

    private:
      /** Parameter: String that states the desired segment orientation.
       *  Valid orientations are "none" (unchanged), "symmetric", "outwards", "downwards".
       */
      std::string m_param_segmentOrientationString = "";

      /** Encoded desired segment orientation.
       *  Valid orientations are "c_None" (unchanged), "c_Symmetric", "c_Outwards", "c_Downwards.
       */
      EPreferredDirection m_segmentOrientation = EPreferredDirection::c_None;

    }; // end class
  } // end namespace TrackFindingCDC
} // end namespace Belle2

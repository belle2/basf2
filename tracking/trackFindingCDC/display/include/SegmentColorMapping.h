/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/display/ColorMapping.h>
#include <tracking/trackFindingCDC/display/Mapping.h>

#include <vector>
#include <string>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCSegment2D;

    /// Interface class for segments to color map functional objects.
    using SegmentColorMapping = Mapping<const CDCSegment2D>;

    /// Color mapping using the default color cycle list
    using SegmentListColorsColorMap = DefaultColorCycleMapping<const CDCSegment2D>;

    /// Segment to color map based on the matched MCTrackId.
    class SegmentMCTrackIdColorMap : public SegmentColorMapping {

    public:
      /// Constructor
      SegmentMCTrackIdColorMap();

      /// Function call to map a segments object from the local finder to a color.
      std::string map(int index, const CDCSegment2D& segment) override;

    private:
      /// List of colors to be cycled through
      std::vector<std::string> m_colors;
    };

    /// Segment to color map based on the forward or backward alignment relative to the match Monte Carlo track.
    class SegmentFBInfoColorMap: public SegmentColorMapping {
    public:
      /// Function call to map a segments object from the local finder to a color.
      std::string map(int index, const CDCSegment2D& segment) override;
    };

    /// Segment to color map by the in track id of the first hit.
    class SegmentFirstInTrackIdColorMap : public SegmentColorMapping {
    public:
      /// Function call to map a segments object from the local finder to a color.
      std::string map(int index, const CDCSegment2D& segment) override;
    };

    /// Segment to color map by the in track id of the last hit.
    class SegmentLastInTrackIdColorMap: public SegmentColorMapping {
    public:
      /// Function call to map a segments object from the local finder to a color.
      std::string map(int index, const CDCSegment2D& segment) override;
    };

    /// Segment to color map by the number of passed superlayers of the first hit.
    class SegmentFirstNPassedSuperLayersColorMap: public SegmentColorMapping {
    public:
      /// Function call to map a segments object from the local finder to a color.
      std::string map(int index, const CDCSegment2D& segment) override;
    };

    /// Segment to color map by the number of passed superlayers of the last hit.
    class SegmentLastNPassedSuperLayersColorMap: public SegmentColorMapping {
    public:
      /// Function call to map a segments object from the local finder to a color.
      std::string map(int index, const CDCSegment2D& segment) override;
    };

  }
}

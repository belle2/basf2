/*
 * CDCSegmentColorMaps.h
 *
 *  Created on: Jun 8, 2015
 *      Author: dschneider
 */

#ifndef CDCSEGMENTCOLORMAPS_H_
#define CDCSEGMENTCOLORMAPS_H_

#include <tracking/trackFindingCDC/display/ColorMaps.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCRecoSegment2D;

    /**
     * Base class for Segments to color map functional objects.
     */
    class CDCSegmentColorMap {
    public:
      /**
       * Function call to map a segments object from the local finder to a color.
       */
      virtual std::string map(const int, const CDCRecoSegment2D&)
      {
        return "";
      }
      /**
       * Informal string summarizing the translation variables to colors.
       */
      virtual std::string info() {return "";}

      /**
       * Virtual Destructor.
       */
      virtual ~CDCSegmentColorMap() = default;

    protected:
      /**
       * Default color to be used.
       */
      std::string m_bkgSegmentColor = "orange";
    };

    /**
     * Segment to color map based on the matched MCTrackId.
     */
    class SegmentMCTrackIdColorMap : public CDCSegmentColorMap {
    public:
      /**
       * Function call to map a segments object from the local finder to a color.
       */
      std::string map(const int iSegment, const CDCRecoSegment2D& segment) override;

    private:
      /// List of colors to be cycled through
      ListColors m_listColors;
    };

    /**
     * Segment to color map based on the forward or backward alignment relative to the match Monte Carlo track.
     */
    class SegmentFBInfoColorMap: public CDCSegmentColorMap {
    public:
      /**
       * Function call to map a segments object from the local finder to a color.
       */
      std::string map(const int iSegment, const CDCRecoSegment2D& segment) override;
    };

    /**
     * Segment to color map by the in track id of the first hit.
     */
    class SegmentFirstInTrackIdColorMap : public CDCSegmentColorMap {
    public:
      /**
       * Function call to map a segments object from the local finder to a color.
       */
      std::string map(const int iSegment, const CDCRecoSegment2D& segment) override;
    };

    /**
     * Segment to color map by the in track id of the last hit.
     */
    class SegmentLastInTrackIdColorMap: public CDCSegmentColorMap {
    public:
      /**
       * Function call to map a segments object from the local finder to a color.
       */
      std::string map(const int iSegment, const CDCRecoSegment2D& segment) override;
    };

    /**
     * Segment to color map by the number of passed superlayers of the first hit.
     */
    class SegmentFirstNPassedSuperLayersColorMap: public CDCSegmentColorMap {
    public:
      /**
       * Function call to map a segments object from the local finder to a color.
       */
      std::string map(const int iSegment, const CDCRecoSegment2D& segment) override;
    };

    /**
     * Segment to color map by the number of passed superlayers of the last hit.
     */
    class SegmentLastNPassedSuperLayersColorMap: public CDCSegmentColorMap {
    public:
      /**
       * Function call to map a segments object from the local finder to a color.
       */
      std::string map(const int iSegment, const CDCRecoSegment2D& segment) override;
    };

    /**
     * Segment to color map using the ListColor colors.
     */
    class SegmentListColorsColorMap : public CDCSegmentColorMap {
    public:
      /**
       * Function call to map a segments object from the local finder to a color.
       */
      std::string map(const int iSegment, const CDCRecoSegment2D& segment) override;
    private:
      /// List of colors to be cycled through
      ListColors m_listColors;
    };

  }//namespace TrackFindingCDC
}//namespace Belle2



#endif /* CDCSEGMENTCOLORMAPS_H_ */

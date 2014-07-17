#pragma once


#include <tracking/cdcLegendreTracking/CDCLegendreTrackCreator.h>
#include <tracking/cdcLegendreTracking/CDCLegendreTrackCandidate.h>
#include <tracking/cdcLegendreTracking/CDCLegendreTrackHit.h>

#include <mdst/dataobjects/HitPatternCDC.h>

namespace Belle2 {

  namespace TrackFinderCDCLegendre {

    class TrackCreator;

    /** Class for working with hit patterns. */
    /* TODO: Check whether track overlaps with other tracks; may be try to sort tracks according to number of inresections/overlappings, some weights might be applied
     * if track sharing more than, for example, 50% of hits, that track should be definitely splitted into few*/
    class PatternChecker {
    public:
      PatternChecker(TrackCreator* cdcLegendreTrackCreator) : m_cdcLegendreTrackCreator(cdcLegendreTrackCreator) {};

      ~PatternChecker() {};

      /** Check pattern of curlers*/
      void checkCurler(std::pair<std::vector<TrackHit*>, std::pair<double, double> >* track);

      /** Check pattern of hits:
       * in between of innermost and outermost SLayers should be no empty SLayers;
       * argument of function allows define minimal number of axial hits in each SLayer
       */
      bool checkCandidate(TrackCandidate* track, int minNHitsSLayer = 2);

      /**
       * Checks given pattern; in development
       */
      void checkPattern(HitPatternCDC* pattern);

      /** Gets number of innermost superlayer*/
      int getMinSLayer(HitPatternCDC* pattern);

      /** Gets number of outermost superlayer*/
      int getMaxSLayer(HitPatternCDC* pattern);

      /** Makes hits cleanup in candidate*/
      void clearBadHits(std::pair<std::vector<TrackHit*>, std::pair<double, double> >* track, std::pair<double, double>& ref_point);

    private:

      TrackCreator* m_cdcLegendreTrackCreator; /**< object for track candidate creating */

      static constexpr double m_PI = 3.1415926535897932384626433832795; /**< pi is exactly three*/
      static constexpr double m_rc = 0.0176991150442477874; /**< threshold of r, which defines curlers*/


    };
  }
}

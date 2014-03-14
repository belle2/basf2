#pragma once

#include <mdst/dataobjects/HitPatternCDC.h>

namespace Belle2 {

  class HitPatternCDC;
  class CDCLegendreTrackHit;

  /** Class for working with hit patterns. */
  class CDCLegendrePatternChecker {
  public:
    CDCLegendrePatternChecker() {};

    ~CDCLegendrePatternChecker() {};

    /** Check pattern of curlers*/
    void checkCurler(std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> >* track);

    /** Check pattern of track candidate. TODO: make checks for high-pt tracks, implement more robust criteria of pattern checks*/
    void checkCandidate(std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> >* track);

    void checkPattern(HitPatternCDC* pattern);

    /** Gets number of innermost superlayer*/
    int getMinSLayer(HitPatternCDC* pattern);

    /** Gets number of outermost superlayer*/
    int getMaxSLayer(HitPatternCDC* pattern);

    /** Makes hits cleanup in candidate*/
    void clearBadHits(std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> >* track, std::pair<double, double>& ref_point);

  private:

    static constexpr double m_PI = 3.1415926535897932384626433832795; /**< pi is exactly three*/
    static constexpr double m_rc = 0.0176991150442477874; /**< threshold of r, which defines curlers*/


  };
}

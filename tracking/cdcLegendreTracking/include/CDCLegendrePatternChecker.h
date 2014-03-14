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

    void checkCurler(std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> >* track);

    void checkCandidate(std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> >* track);

    void checkPattern(HitPatternCDC* pattern, double radius);

    int getMinSLayer(HitPatternCDC* pattern);

    int getMaxSLayer(HitPatternCDC* pattern);

    void clearBadHits(std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> >* track, std::pair<double, double>& ref_point);

  private:

//    double m_rMin; /**< Minimum in r direction, initialized in initializer list of the module*/
//    double m_rMax; /**< Maximum in r direction, initialized in initializer list of the module*/

    static constexpr double m_PI = 3.1415926535897932384626433832795; /**< pi is exactly three*/
    static constexpr double m_rc = 0.0176991150442477874; /**< threshold of r, which defines curlers*/
//    int m_nbinsTheta; /**< Number of bins in theta, derived from m_maxLevel*/
//    int m_initialAxialHits; /**< Initial number of axial hits in the stepped hough algorithm*/


  };
}

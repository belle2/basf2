#ifndef SECTORARC_H
#define SECTORARC_H
#include <tracking/modules/datareduction/PXDLadder.h>

#include <tracking/modules/datareduction/SectorBasic.h>
#include <tracking/modules/datareduction/TrackerHit.h>

#include <cmath>
#include "TVector2.h"

namespace Belle2 {

  class SectorArc : public SectorBasic {
  public:

    SectorArc(double RotationAngle, double DownShift, double SectorWidth, double RadiusSmall, double RadiusBig);
    virtual ~SectorArc();
    double getSmallRadius() { return _RadiusSmall; };
    double getBigRadius() { return _RadiusBig; };

  protected:

    double _RadiusSmall;
    double _RadiusBig;
    TVector2 _CenterSmall;
    TVector2 _CenterBig;

    virtual void setIntersection(LadderEntry& ladderEntry);
    virtual int checkPoint(double x, double y);

    std::pair<double, double> intersectCircle(PXDLadder& ladder, TVector2 center, double radius, bool in);

#ifdef CAIRO_OUTPUT
    virtual void draw(cairo_t* cairo);
#endif

  private:

  };
}
#endif /* SECTORARC_H */

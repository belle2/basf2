#ifndef SECTORSTRAIGHT_H
#define SECTORSTRAIGHT_H

#include <tracking/modules/datareduction/PXDLadder.h>
#include <tracking/modules/datareduction/TrackerHit.h>
#include <tracking/modules/datareduction/SectorBasic.h>

#include <cmath>



namespace Belle2 {

  class SectorStraight : public SectorBasic {
  public:

    SectorStraight(double RotationAngle, double DownShift, double SectorWidth, double OpeningAngle);
    virtual ~SectorStraight();


  protected:

    double _OpeningAngle;

    std::pair<double, double> intersectLine(PXDLadder &ladder, TVector2 pos, TVector2 dir, bool bigger);
    virtual void setIntersection(LadderEntry& ladderEntry);
    virtual int checkPoint(double x, double y);

#ifdef CAIRO_OUTPUT
    virtual void draw(cairo_t* cairo);
#endif

  private:

  };
}
#endif /* SECTORSTRAIGHT_H */

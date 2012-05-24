#ifndef HOUGHTRANSFORMSTRAIGHT_H
#define HOUGHTRANSFORMSTRAIGHT_H
#include <tracking/modules/datareduction/HoughTransformBasic.h>

#include <tracking/modules/datareduction/TrackerHit.h>
#include <cmath>


//Parameterization: z = m*r + t
// parameter: a=m, b=t

namespace Belle2 {

  class HoughTransformStraight : public HoughTransformBasic {
  public:

    HoughTransformStraight();
    virtual ~HoughTransformStraight();

    void setAcceptanceAngles(double minAngle, double maxAngle); //in radians
    void setZRange(double minZ, double maxZ);


  protected:

    virtual void getInitialHoughBoxParams(double& min_a, double& max_a, double& min_b, double& max_b);
    virtual bool isHitInHoughBox(HoughTransformBox& houghBox, rzHit& hit);
    virtual double getZValue(double a, double b, double r);

#ifdef CAIRO_OUTPUT
    virtual void drawResultItem(cairo_t* cairo, ResultItem& ri) {
      double min_z = -300;
      double max_z = 350;
      double min_r = 0;
      double max_r = 150;
      double m = 1 / ri.a;
      double t = -ri.b / ri.a;
      cairo_straight(cairo, m, t, min_z, max_z, min_r, max_r);
      cairo_stroke_abs(cairo);
    }

    virtual void drawHits(double min_a, double max_a, double min_b, double max_b) {
      if (cairo) {
        cairo_set_source_rgb(cairo, 0, 0, 1);
        cairo_set_line_width(cairo, 0.5);
        for (std::list<rzHit*>::iterator hitIter = _rzHits.begin(); hitIter != _rzHits.end(); ++hitIter) {
          rzHit& rzhit = **hitIter;
          cairo_straight(cairo, -rzhit.r, rzhit.z, min_a, max_a, min_b, max_b);
          cairo_stroke_abs(cairo);
        }
      }
    };
#endif

  private:

    double _acceptanceAngles[2]; //minAngle, maxAngle
    double _zRange[2]; //minZ, maxZ

  };
}

#endif /* HOUGHTRANSFORMSTRAIGHT_H */

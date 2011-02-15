#ifndef HOUGHTRANSFORMSINE_H_
#define HOUGHTRANSFORMSINE_H_

#include "lcio.h"
#include <EVENT/MCParticle.h>
#include <EVENT/TrackerHit.h>
#include <cmath>

#include "HoughTransformBasic.h"



class HoughTransformSine : public HoughTransformBasic {
public:

  HoughTransformSine();
  virtual ~HoughTransformSine();

  void setAcceptanceAngles(double minAngle, double maxAngle); //in radians
  void setRadius(double minRadius, double maxRadius);


protected:

  virtual void getInitialHoughBoxParams(double& min_a, double& max_a, double& min_b, double& max_b);
  virtual bool isHitInHoughBox(HoughTransformBox& houghBox, rzHit& hit);
  virtual double getZValue(double a, double b, double r);


private:

  double _acceptanceAngles[2]; //minAngle, maxAngle
  double _radius[2]; //minRadius, maxRadius

#ifdef CAIRO_OUTPUT
  virtual void drawResultItem(cairo_t* cairo, ResultItem &ri) {
    double min_z = std::max(-M_PI / 2 / ri.a, -300.0);
    double max_z = std::min(M_PI / 2 / ri.a, 350.0);
    cairo_sine(cairo, 1 / ri.b, ri.a, 0, min_z, max_z);
    cairo_stroke_abs(cairo);
  }

  virtual void drawHits(double min_a, double max_a, double min_b, double max_b) {
    if (cairo) {
      cairo_set_source_rgb(cairo, 0, 0, 1);
      cairo_set_line_width(cairo, 0.5);
      for (std::list<rzHit*>::iterator hitIter = _rzHits.begin(); hitIter != _rzHits.end(); ++hitIter) {
        rzHit &rzhit = **hitIter;
        cairo_sine(cairo, 1 / rzhit.r, rzhit.z, 0, min_a, max_a);
        cairo_stroke_abs(cairo);
      }
    }
  };
#endif

};

#endif /* HOUGHTRANSFORMSINE_H_ */

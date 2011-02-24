#ifndef HOUGHTRANSFORMBASIC_H
#define HOUGHTRANSFORMBASIC_H
#include <tracking/modules/datareduction/cairo-utils.h>

#include <tracking/modules/datareduction/TrackerHit.h>
#include <tracking/modules/datareduction/SectorBasic.h>

#include <list>

enum box_state {INITIAL, ITERATION, FINAL};

namespace Belle2 {

  struct rzHit {
    rzHit(double r, double z) : r(r), z(z) {};

    double r;
    double z;
  };


  struct HoughTransformBox {
    HoughTransformBox(double left, double right, double bottom, double top) :
        left(left), right(right), top(top), bottom(bottom) {};

    double left; //left border of box
    double right;
    double top;
    double bottom;
    std::list<rzHit*> containedHits;
    void setHits(std::list<rzHit*> source) {
      containedHits.clear();
      containedHits.insert(
        containedHits.end(),
        source.begin(),
        source.end()
      );
    }
  };


  struct ResultItem {
    ResultItem(double a, double b, double error_a, double error_b) :
        a(a), b(b), error_a(error_a), error_b(error_b) {};

    double a; //parameter a
    double b; //parameter b
    double error_a; //error of parameter a
    double error_b; //error of parameter b
  };


  class HoughTransformBasic {
  public:

    HoughTransformBasic();
    virtual ~HoughTransformBasic();

    void setMinHoughBoxSizeParamA(double minSize);
    void setMinHoughBoxSizeParamB(double minSize);
    void setMinNumberTrackerHits(unsigned int minNumber);

    void setRegionEnlargement(double widthStart, double widthEnd, double lengthStart, double lengthEnd);

    void doHoughSearch(SectorBasic& sector);

#ifdef CAIRO_OUTPUT
    void setCairo(cairo_t* cairo) {this->cairo = cairo;};
    void drawRZ(cairo_t* cairo);
#endif

  protected:

    double _minHoughBoxSizeParamA;
    double _minHoughBoxSizeParamB;

    double _regionEnlWidthStart;
    double _regionEnlWidthEnd;
    double _regionEnlLengthStart;
    double _regionEnlLengthEnd;

    unsigned int _minNumberTrackerHits; //minimum number of tracker hits (and fcts. in the hough space).
    std::list<rzHit*> _rzHits;
    std::list<HoughTransformBox*> _houghBoxes;
    std::list<ResultItem*> _resultSet;

    void createRegionsOfInterest(SectorBasic& sector);

    virtual void getInitialHoughBoxParams(double& min_a, double& max_a, double& min_b, double& max_b) {};
    virtual bool isHitInHoughBox(HoughTransformBox& houghBox, rzHit& hit) {return false; };
    virtual double getZValue(double a, double b, double r) {return 0.0; };

#ifdef CAIRO_OUTPUT
    cairo_t *cairo;
    void drawBox(HoughTransformBox &box, box_state state);
    virtual void drawResultItem(cairo_t* cairo, ResultItem &ri) {};
    virtual void drawHits(double min_a, double max_a, double min_b, double max_b) {};
#endif

  private:

    void clearLists();

  };
}
#endif /* HOUGHTRANSFORMBASIC_H */

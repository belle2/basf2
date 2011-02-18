#ifndef PXDLADDER_H_
#define PXDLADDER_H_

#ifdef CAIRO_OUTPUT
#include <tracking/modules/datareduction/cairo-utils.h>
#endif

#include <list>

//ROOT
#include "TVector3.h"
namespace Belle2 {

  struct RegionOfInterest {

    RegionOfInterest(double widthStart, double widthEnd, double lengthStart, double lengthEnd) :
        widthStart(widthStart), widthEnd(widthEnd), lengthStart(lengthStart), lengthEnd(lengthEnd) {};

    double widthStart;
    double widthEnd;
    double lengthStart;
    double lengthEnd;
#ifdef CAIRO_OUTPUT
    cairo_color color;
#endif
  };


  class PXDLadder {
  public:

    PXDLadder(TVector3& position, TVector3& normal, TVector3& size);
    virtual ~PXDLadder();

    TVector3 getPosition() {return _position; };
    TVector3 getNormal() {return _normal; };
    TVector3 getSize() {return _size; };

    PXDLadder& addRegion(double widthStart, double widthEnd, double lengthStart, double lengthEnd);
    std::list<RegionOfInterest*>& getRegionList() {return _regions; };
    void clearRegions();

    TVector3 convertToLocal(const double* pos);
    TVector2 convertToRelative(const double* pos);
    bool isInLadder(const double* pos);


  protected:

    TVector3 _position;
    TVector3 _normal;
    TVector3 _size;

    std::list<RegionOfInterest*> _regions;


  private:

  };
}
#endif /* PXDLADDER_H_ */

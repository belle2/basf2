//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : THoughPlaneBoolean.cc
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a Hough parameter plane with simple
//               counter.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------


#include <string.h>
#include "tracking/modules/trasan/THoughPlaneBoolean.h"
#include "tracking/modules/trasan/TPoint2D.h"
#ifdef TRASAN_DEBUG
#include "tracking/modules/trasan/TDebugUtilities.h"
#endif


namespace Belle {

  THoughPlaneBoolean::THoughPlaneBoolean(const std::string& name,
                                         unsigned nX,
                                         float xMin,
                                         float xMax,
                                         unsigned nY,
                                         float yMin,
                                         float yMax)
    : THoughPlaneBase(name, nX, xMin, xMax, nY, yMin, yMax),
      _n(nX* nY / 32 + 1),
//    _cell(new unsigned[nX * nY / 32 + 1]) {
      _cell(new unsigned[_n]),
      _nPatterns(0),
      _patterns(0),
      _nActive(0)
  {
    clear();
  }

  THoughPlaneBoolean::~THoughPlaneBoolean()
  {
    for (unsigned i = 0; i < _nPatterns; i++)
      delete [] _patterns[i];
    delete [] _patterns;
    delete [] _nActive;
    delete [] _cell;
  }

  void
  THoughPlaneBoolean::vote(float rx,
                           float ry,
                           float targetCharge,
                           const THoughTransformation& hough,
                           int weight)
  {

    const HepGeom::Point3D<double>  r(rx, ry, 0);

    //...phi loop...
    for (unsigned i = 0; i < nX(); i++) {
      const float x0 = xSize() * float(i);
      const HepGeom::Point3D<double>  phi(cos(x0), sin(x0), 0);
      float charge = r.cross(phi).z();
      if (targetCharge != 0)
        if (targetCharge * charge > 0)
          continue;

      const float y0 = hough.y(rx, ry, x0);
      const float x1 = xSize() * float(i + 1);
      const float y1 = hough.y(rx, ry, x1);

      //...Location in the plane...
      int iY0 = int((y0 - yMin()) / ySize());
      int iY1 = int((y1 - yMin()) / ySize());

      //...This is special implementation for Circle Hough...
      if (hough.diverge(rx, ry, x0, x1)) {
        if (iY0 > 0) {
          if (iY0 >= (int) nY()) continue;
          iY1 = nY() - 1;
        } else {
          if (iY1 >= (int) nY()) continue;
          iY0 = iY1;
          iY1 = nY() - 1;
        }
      }

      //...Sorting...
      if (iY0 > iY1) {
        const int tmp = iY0;
        iY0 = iY1;
        iY1 = tmp;
      }

      //...Both out of region ?...
      if (iY1 < 0) continue;
      if (iY0 >= (int) nY()) continue;

      //...In region ?...
      if (iY0 < 0) iY0 = 0;
      if (iY0 >= (int) nY()) iY0 = nY() - 1;
      if (iY1 < 0) iY1 = 0;
      if (iY1 >= (int) nY()) iY1 = nY() - 1;

      //...Voting...
      for (unsigned j = (unsigned) iY0; j < (unsigned)(iY1 + 1); j++) {
        const unsigned k = i * nY() + j;
        const unsigned b0 = k / 32;
        const unsigned b1 = k % 32;
        if (weight > 0)
          _cell[b0] |= (1 << b1);
        else
          _cell[b0] &= (~(1 << b1));

//      _cell[i * nY() + j] += weight;
//      if (_cell[i * nY() + j] < 0)
//    _cell[i * nY() + j] = 0;
      }
    }
  }

  void
  THoughPlaneBoolean::registerPattern(unsigned id)
  {
    if (_patterns[id]) {
      std::cout << "THoughPlaneBoolean::registerPattern !!! "
                << "a pattern(id=" << id << ") was already registered"
                << std::endl;
      return;
    }
    if (id >= _nPatterns) {
      std::cout << "THoughPlaneBoolean::registerPattern !!! "
                << "given id(" << id << ") is over range(" << _nPatterns << ")"
                << std::endl;
      return;
    }
    const unsigned n = nX() * nY();

    //...Check # of active cells...
    unsigned nActive = 0;
    for (unsigned i = 0; i < n; i++) {
      const unsigned j = i / 32;
      const unsigned k = i % 32;
      if ((_cell[j] >> k) & 1)
        ++nActive;
    }
    _nActive[id] = nActive;

//  std::cout << "    id=" << id << ",nActive=" << nActive << std::endl;

    //...Create array...
    _patterns[id] = new unsigned[nActive];

    //...Store them...
    unsigned l = 0;
    for (unsigned i = 0; i < n; i++) {
      const unsigned j = i / 32;
      const unsigned k = i % 32;
      if ((_cell[j] >> k) & 1) {
        _patterns[id][l] = i;
        l++;
      }
    }
  }

  void
  THoughPlaneBoolean::preparePatterns(unsigned nPatterns)
  {
    if (_nPatterns) {
      std::cout << "THoughPlaneBoolean::preparePatterns !!! "
                << "patterns were already allocated" << std::endl;
      return;
    }

    _nPatterns = nPatterns;
    _patterns = new unsigned * [_nPatterns];
    _nActive = new unsigned[_nPatterns];
    memset(_patterns, 0, _nPatterns * sizeof(unsigned*));
  }

  void
  THoughPlaneBoolean::vote(unsigned patternId, int weight)
  {
    const unsigned n = _nActive[patternId];
    for (unsigned i = 0; i < n; i++) {
      add(_patterns[patternId][i], weight);
    }
  }

} // namespace Belle

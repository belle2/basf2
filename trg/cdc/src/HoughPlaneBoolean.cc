/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : HoughPlaneBoolean.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a Hough parameter plane with simple
//               counter.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRGCDC_SHORT_NAMES

#include <iostream>
#include "trg/cdc/HoughPlaneBoolean.h"

using namespace std;

namespace Belle2 {

  TRGCDCHoughPlaneBoolean::TRGCDCHoughPlaneBoolean(const std::string& name,
                                                   const TCHTransformation& trans,
                                                   unsigned nX,
                                                   float xMin,
                                                   float xMax,
                                                   unsigned nY,
                                                   float yMin,
                                                   float yMax)
    : TRGCDCHoughPlaneBase(name, trans, nX, xMin, xMax, nY, yMin, yMax),
      _n(nX * nY / 32 + 1),
      _cell(new unsigned[_n]),
      _nPatterns(0),
      _patterns(0),
      _nActive(0),
      _reverse(0)
  {
    clear();
  }

  TRGCDCHoughPlaneBoolean::~TRGCDCHoughPlaneBoolean()
  {
    for (unsigned i = 0; i < _nPatterns; i++)
      delete [] _patterns[i];
    delete [] _patterns;
    delete [] _nActive;
    delete [] _cell;
    delete [] _reverse;
  }

  void
  TRGCDCHoughPlaneBoolean::vote(float rx,
                                float ry,
                                float targetCharge,
                                int weight)
  {

    const HepGeom::Point3D<double> r(rx, ry, 0);

//
//  cout << "yMax=" << yMax() << endl;
//

    //...phi loop...
    for (unsigned i = 0; i < nX(); i++) {
      const float x0 = xSize() * float(i);
      const HepGeom::Point3D<double> center(cos(x0), sin(x0), 0);
      float charge = r.cross(center).z();
      if (targetCharge != 0)
        if (targetCharge * charge > 0)
          continue;

      float y0 = transformation().y(rx, ry, x0);
      const float x1 = xSize() * float(i + 1);
      float y1 = transformation().y(rx, ry, x1);

//
//      cout << "x0,x1,y0,y1=" << x0 << "," << x1 << "," << y0 << "," << y1
//           << endl;
//

      //...Check y position...
      if ((y0 == 0) && (y1 == 0))
        continue;
      else if ((y0 > yMax()) && (y1 > yMax()))
        continue;
      else if ((y0 < yMin()) && (y1 < yMin()))
        continue;

      //...Divergence here...
      if ((y0 == 0) && (y1 != 0))
        y0 = yMax();
      else if ((y0 != 0) && (y1 == 0))
        y1 = yMax();

      //...Adjust location...
      if (y0 < yMin())
        y0 = yMin();
      if (y1 < yMin())
        y1 = yMin();
      if (y0 > yMax())
        y0 = yMax();
      if (y1 > yMax())
        y1 = yMax();

      //...Location in the plane...
      int iY0 = int((y0 - yMin()) / ySize());
      int iY1 = int((y1 - yMin()) / ySize());

//
//      cout << "x0,x1,y0,y1=" << x0 << "," << x1 << "," << y0 << "," << y1
//           << "," << iY0 << "," << iY1 << endl;
//

      //...Sorting...
      if (iY0 > iY1) {
        const int tmp = iY0;
        iY0 = iY1;
        iY1 = tmp;
      }

      //...Voting...
      for (unsigned j = (unsigned) iY0; j < (unsigned)(iY1 + 1); j++) {
        const unsigned k = i * nY() + j;
        const unsigned b0 = k / 32;
        const unsigned b1 = k % 32;
        if (weight > 0)
          _cell[b0] |= (1 << b1);
        else
          _cell[b0] &= (~(1 << b1));
      }
    }
  }

  void
  TRGCDCHoughPlaneBoolean::voteUsedInTrasan(float rx,
                                            float ry,
                                            float targetCharge,
                                            int weight)
  {

    const HepGeom::Point3D<double> r(rx, ry, 0);

    //...phi loop...
    for (unsigned i = 0; i < nX(); i++) {
      const float x0 = xSize() * float(i);
      const HepGeom::Point3D<double>  phi(cos(x0), sin(x0), 0);
      float charge = r.cross(phi).z();
      if (targetCharge != 0)
        if (targetCharge * charge > 0)
          continue;

      const float y0 = transformation().y(rx, ry, x0);
      const float x1 = xSize() * float(i + 1);
      const float y1 = transformation().y(rx, ry, x1);

      //...Location in the plane...
      int iY0 = int((y0 - yMin()) / ySize());
      int iY1 = int((y1 - yMin()) / ySize());

      //...This is special implementation for Circle Hough...
      if (transformation().diverge(rx, ry, x0, x1)) {
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
      // if (iY1 < 0) iY1 = 0; //redundant
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
      }
    }
  }

  void
  TRGCDCHoughPlaneBoolean::registerPattern(unsigned id)
  {

    //...Check status...
    if (_patterns[id]) {
      std::cout << "TRGCDCHoughPlaneBoolean::registerPattern !!! "
                << "a pattern(id=" << id << ") was already registered"
                << std::endl;
      return;
    }
    if (id >= _nPatterns) {
      std::cout << "TRGCDCHoughPlaneBoolean::registerPattern !!! "
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
      if ((_cell[j] >> k) & 1) {
        ++nActive;
        _reverse[i].push_back(id);
      }
    }
    _nActive[id] = nActive;

    //...Create an array...
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
  TRGCDCHoughPlaneBoolean::preparePatterns(unsigned nPatterns)
  {
    if (_nPatterns) {
      std::cout << "TRGCDCHoughPlaneBoolean::preparePatterns !!! "
                << "patterns were already allocated" << std::endl;
      return;
    }

    _nPatterns = nPatterns;
    _patterns = new unsigned * [_nPatterns];
    _nActive = new unsigned[_nPatterns];
    memset(_patterns, 0, _nPatterns * sizeof(unsigned*));
    _reverse = new vector<unsigned>[nX() * nY()];
  }

  void
  TRGCDCHoughPlaneBoolean::vote(unsigned patternId, int weight)
  {
    const unsigned n = _nActive[patternId];
    for (unsigned i = 0; i < n; i++) {
      add(_patterns[patternId][i], weight);
    }
  }

} // namespace Belle2

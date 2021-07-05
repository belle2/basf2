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
// Filename : HoughPlaneBase.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A base class to represent a Hough parameter plane
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRGCDC_SHORT_NAMES

#include "trg/cdc/HoughPlaneBase.h"

namespace Belle2 {

  TRGCDCHoughPlaneBase::TRGCDCHoughPlaneBase(const std::string& name,
                                             const TCHTransformation& trans,
                                             unsigned nX,
                                             float xMin,
                                             float xMax,
                                             unsigned nY,
                                             float yMin,
                                             float yMax)
    : _name(name),
      _trans(trans),
      _charge(0),
      _nX(nX),
      _xMin(xMin),
      _xMax(xMax),
      _xSize((xMax - xMin) / float(nX)),
      _nY(nY),
      _yMin(yMin),
      _yMax(yMax),
      _ySize((yMax - yMin) / float(nY)),
      _area(TRGPoint2D(xMin, yMin), TRGPoint2D(xMax, yMax))
  {
  }

  TRGCDCHoughPlaneBase::~TRGCDCHoughPlaneBase()
  {
    // HepAListDeleteAll(_regions);
    clearRegions();
  }

  void
  TRGCDCHoughPlaneBase::locationInPlane(float x0, float y0, float x1, float y1,
                                        unsigned& nFound,
                                        unsigned& X0, unsigned& Y0,
                                        unsigned& X1, unsigned& Y1) const
  {

    const TRGPoint2D p(x0, y0);
    const TRGPoint2D q(x1, y1);

    //...Boundary check...
    if (_area.inArea(p) && _area.inArea(q)) {
      X0 = unsigned((x0 - _xMin) / _xSize);
      Y0 = unsigned((y0 - _yMin) / _ySize);
      X1 = unsigned((x1 - _xMin) / _xSize);
      Y1 = unsigned((y1 - _yMin) / _ySize);
      nFound = 2;
      return;
    }

    nFound = 0;
    TRGPoint2D c[2];
    _area.cross(p, q, nFound, c);
    if (nFound == 2) {
      X0 = unsigned((c[0].x() - _xMin) / _xSize);
      Y0 = unsigned((c[0].y() - _yMin) / _ySize);
      X1 = unsigned((c[1].x() - _xMin) / _xSize);
      Y1 = unsigned((c[1].y() - _yMin) / _ySize);
    }
  }

// void
// TRGCDCHoughPlaneBase::smooth(void) {
//     unsigned * newCell = new unsigned[_nX * _nY];

//     for (unsigned i = 0; i < _nX; i++) {
//         for (unsigned j = 0; j < _nY; j++) {
//             unsigned il = i - 1;
//             if (i == 0) il = _nX - 1;
//             unsigned ir = i + 1;
//             if (ir == _nX) ir = 0;
//             unsigned jt = j + 1;
//             if (jt == _nY) jt = j;
//             unsigned jb = j - 1;
//             if (j == 0) jb = 0;

// //             const unsigned sum
// //                 = entry(il, jt) + entry(i, jt) + entry(ir, jt)
// //                 + entry(il, j) + entry(i, j) + entry(ir, j)
// //                 + entry(il, jb) + entry(i, jb) + entry(ir, jb);
// //            const unsigned average = sum / 9;
//             const unsigned sum
//                 = entry(i, jt) + entry(i, j) + entry(i, jb);
//             const unsigned average = sum / 3;

//             newCell[_nY * i + j] = average;
//         }
//     }

//     for (unsigned i = 0; i < _nX * _nY; i++) {
//         std::cout << "??? " << _cell[i] << " -> " << newCell[i] << std::endl;
//         _cell[i] = newCell[i];
//     }

//     delete[] newCell;
// }

  int
  TRGCDCHoughPlaneBase::maxEntryInRegion(unsigned targetId) const
  {
#ifdef TRASAN_DEBUG
    const std::string stage = "THghPlnBase::maxEntryInRegion";
    EnterStage(stage);
#endif
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "target id=" << targetId << ",#regions="
              << _regions.length() << std::endl;
#endif
//  for (unsigned i = 0; i < (unsigned) _regions.length(); i++) {
    for (unsigned i = 0; i < (unsigned) _regions.size(); i++) {
      const std::vector<unsigned>& region = * _regions[i];
      unsigned maxEntry = 0;
      bool idFound = false;
//        for (unsigned j = 0; j < (unsigned) region.length(); j++) {
      for (unsigned j = 0; j < (unsigned) region.size(); j++) {
//            const unsigned id = * region[j];
        const unsigned id = region[j];
        if (id == targetId) idFound = true;
        if (maxEntry < entry(id))
          maxEntry = entry(id);
      }
      if (idFound) {
#ifdef TRASAN_DEBUG
        LeaveStage(stage);
#endif
        return maxEntry;
      }
    }

#ifdef TRASAN_DEBUG
    LeaveStage(stage);
#endif
    return 0;
  }

  void
  TRGCDCHoughPlaneBase::vote(float rx,
                             float ry,
                             int targetCharge,
                             int weight)
  {

    const HepGeom::Point3D<double>  r(rx, ry, 0);

    //...phi loop...
    for (unsigned i = 0; i < _nX; i++) {
      const float x0 = xSize() * float(i);
      const HepGeom::Point3D<double>  phi(cos(x0), sin(x0), 0);
      float charge = r.cross(phi).z();
      if (targetCharge != 0)
        if (targetCharge * charge > 0)
          continue;

      const float y0 = _trans.y(rx, ry, x0);
      const float x1 = xSize() * float(i + 1);
      const float y1 = _trans.y(rx, ry, x1);

      //...Location in the plane...
      int iY0 = int((y0 - yMin()) / ySize());
      int iY1 = int((y1 - yMin()) / ySize());

      //...This is special implementation for Circle Hough...
      if (_trans.diverge(rx, ry, x0, x1)) {
        if (iY0 > 0) {
          if (iY0 >= (int) _nY) continue;
          iY1 = _nY - 1;
        } else {
          if (iY1 >= (int) _nY) continue;
          iY0 = iY1;
          iY1 = _nY - 1;
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
      if (iY0 >= (int) _nY) continue;

      //...In region ?...
      if (iY0 < 0) iY0 = 0;
      if (iY0 >= (int) _nY) iY0 = _nY - 1;
      //if (iY1 < 0) iY1 = 0; //redundant condition
      if (iY1 >= (int) _nY) iY1 = _nY - 1;

      //...Voting...
      for (unsigned j = (unsigned) iY0; j < (unsigned)(iY1 + 1); j++) {
//            _cell[i * _nY + j] += weight;
        add(i * _nY + j, weight);
//             if (_cell[i * _nY + j] < 0)
//                 _cell[i * _nY + j] = 0;
      }
    }
  }

  void
  TRGCDCHoughPlaneBase::dump(const std::string& message,
                             const std::string& prefix) const
  {
    std::cout << prefix << "dump of " << name() << ":" << message;
    if (message != "region") {
      bool first = true;
      const unsigned n = _nX * _nY;
      unsigned nDump = 0;
      for (unsigned i = 0; i < n; i++) {
        if (entry(i)) {
          if (first)
            first = false;
          else
            std::cout << ",";
          if (!(nDump % 10)) std::cout << std::endl;
          std::cout << i << "-" << entry(i);
          ++nDump;
        }
      }
      if (first)
        std::cout << "no active cell";
    }
    std::cout << std::endl;
//  for (unsigned i = 0; i < _regions.length(); i++) {
    for (unsigned i = 0; i < _regions.size(); i++) {
      std::cout << prefix << "    region " << i << ":";
//        for (unsigned j = 0; j < _regions[i]->length(); j++) {
      for (unsigned j = 0; j < _regions[i]->size(); j++) {
//            const unsigned id = * (* _regions[i])[j];
        const unsigned id = (* _regions[i])[j];
        std::cout << id << "(" << entry(id) << "),";
      }
      std::cout << std::endl;
    }
//  if (_regions.length())
    if (_regions.size())
      std::cout << std::endl;
  }

} // namespace Belle

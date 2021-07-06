/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//-----------------------------------------------------------------------------
// Description : A base class to represent a Hough parameter plane
//-----------------------------------------------------------------------------

#define TRGCDC_SHORT_NAMES

#include "trg/cdc/HoughPlane.h"

namespace Belle2 {

  TRGCDCHoughPlane::TRGCDCHoughPlane(const std::string& name,
                                     const TCHTransformation& trans,
                                     unsigned nX,
                                     float xMin,
                                     float xMax,
                                     unsigned nY,
                                     float yMin,
                                     float yMax)
    : TRGCDCHoughPlaneBase(name, trans, nX, xMin, xMax, nY, yMin, yMax),
//    _cell(new unsigned[nX * nY * sizeof(unsigned)]),
      _cell(new int[nX * nY]),
      _patterns(0),
      _nPatterns(0)
  {
    clear();
  }

  TRGCDCHoughPlane::~TRGCDCHoughPlane()
  {
    delete [] _cell;
    if (_patterns)
      delete [] _patterns;
  }

// void
// TRGCDCHoughPlane::vote(float x, float y) {
//     if (x < xMin()) return;
//     if (x > xMax()) return;
//     if (y < yMin()) return;
//     if (y > yMax()) return;

//     const unsigned iX = unsigned((x - xMin()) / xSize());
//     const unsigned iY = unsigned((y - yMin()) / ySize());
//     const unsigned id = iX * nY() + iY;
//     ++_cell[id];
// }

// void
// TRGCDCHoughPlane::vote(float x0, float y0, float x1, float y1) {

//     //...Cell positions...
//     unsigned iX0, iY0, iX1, iY1;
//     unsigned nFound;
//     locationInPlane(x0, y0, x1, y1, nFound, iX0, iY0, iX1, iY1);
// #ifdef TRASAN_DEBUG_DETAIL
//     std::cout << "(x0,y0)=(" << x0 << "," << y0 << "),(iX0,iY0)=(" << iX0
//               << "," << iY0 << ")" << std::endl;
//     std::cout << "(x1,y1)=(" << x1 << "," << y1
//               << "),(iX1,iY1)=(" << iX1 << "," << iY1 << ")" << std::endl;
// #endif

//     //...No cell...
//     if (nFound < 2) return;

//     //...Same cell...
//     if ((iX0 == iX1) && (iY0 == iY1)) {
//         ++_cell[iX0 * nY() + iY0];
//         return;
//     }

//     //...X...
//     else if (iX0 == iX1) {
//         int iMin = iY0;
//         int iMax = iY1;
//         if (iY0 > iY1) {
//             iMin = iY1;
//             iMax = iY0;
//         }
//         for (int i = iMin; i < (iMax + 1); i++)
//             ++_cell[iX0 * nY() + i];
//         return;
//     }

//     //...Y...
//     else if (iY0 == iY1) {
//         int iMin = iX0;
//         int iMax = iX1;
//         if (iX0 > iX1) {
//             iMin = iX1;
//             iMax = iX0;
//         }
//         for (int i = iMin; i < (iMax + 1); i++)
//             ++_cell[i * nY() + iY0];
//         return;
//     }

//     //...Line...
//     const float xDiff = x1 - x0;
//     const float yDiff = y1 - y0;
//     const float slope = yDiff / xDiff;
//     const float offset = y0 - slope * x0;

//     //...Direction to move : (x0, y0) -> (x1, y1)...
//     unsigned direction = 0;
//     if ((xDiff < 0) && (yDiff > 0)) direction = 1;
//     else if ((xDiff < 0) && (yDiff < 0)) direction = 2;
//     else if ((xDiff > 0) && (yDiff < 0)) direction = 3;

//     //...Grid offset to search a cross-point...
//     int iXGrid = 1;
//     int iYGrid = 1;
//     if (direction == 1) {
//         iXGrid = 0;
//         iYGrid = 1;
//     }
//     else if (direction == 2) {
//         iXGrid = 0;
//         iYGrid = 0;
//     }
//     else if (direction == 3) {
//         iXGrid = 1;
//         iYGrid = 0;
//     }

//     std::cout << "direction=" << direction << std::endl;
//     std::cout << "iXGrid,iYGrid=" << iXGrid << "," << iYGrid << std::endl;

//     //...Steps...
//     const int xStep = iX1 - iX0;
//     const int yStep = iY1 - iY0;
//     const unsigned maxStep = abs(xStep) + abs(yStep);

//     unsigned iX = iX0;
//     unsigned iY = iY0;
//     unsigned steps = 0;
//     while (1) {

//         //...Vote this cell...
//         ++_cell[iX * nY() + iY];
//         if ((iX == iX1) && (iY == iY1))
//             break;

//         //...Center of the cell...
//         TPoint2D center((float(iX) + 0.5) * xSize() + xMin(),
//                         (float(iY) + 0.5) * ySize() + yMin());

//         //...Set grids...
//         const float xGrid = (iX + iXGrid) * xSize() + xMin();
//         const float yGrid = (iY + iYGrid) * ySize() + yMin();

//         //...Cal. position of cross-points with grids...
//         TPoint2D crs[2];
//         crs[0] = TPoint2D((yGrid - offset) / slope, yGrid);
//         crs[1] = TPoint2D(xGrid, slope * xGrid + offset);

//         //...Select points on a cell...
//         unsigned best = 0;
//         if ((crs[1] - center).mag2() < (crs[0] - center).mag2()) best = 1;

// #ifdef TRASAN_DEBUG_DETAIL
//         std::cout << "    step " << steps << ":(iX,iY)=" << iX << "," << iY
//                   << "):center=(" << center.x() << "," << center.y()
//                   << "):best=" << best << std::endl;
//         std::cout << "        crs0=(" << crs[0].x() << "," << crs[0].y()
//                   << "),crs1=(" << crs[1].x() << "," << crs[1].y() << "),"
//                   << std::endl;
// #endif

//         //...Move...
//         if (best == 0) {
//             if (iYGrid == 1)
//                 ++iY;
//             else
//                 --iY;
//         }
//         else {
//             if (iXGrid == 1)
//                 ++iX;
//             else
//                 --iX;
//         }

//         //...End ?...
//         ++steps;
//         if (steps > maxStep) break;
//     }
// }

  void
  TRGCDCHoughPlane::registerPattern(unsigned)
  {
    if (_patterns) {
      delete [] _patterns;
      _nPatterns = 0;
    }
    const unsigned n = nX() * nY();

    //...Check # of active cells...
    for (unsigned i = 0; i < n; i++)
      if (_cell[i]) ++_nPatterns;

    //...Create array...
    _patterns = new unsigned[_nPatterns];

    //...Store them...
    unsigned j = 0;
    for (unsigned i = 0; i < n; i++)
      if (_cell[i]) _patterns[j++] = i;
  }

  void
  TRGCDCHoughPlane::voteByPattern(float xOffset, int weight)
  {
#ifdef TRASAN_DEBUG
    if (_patterns == 0)
      std::cout << "TRGCDCHoughPlane::vote !!! pattern is note defined" << std::endl;
    if ((xOffset < 0) || (xOffset > 1))
      std::cout << "TRGCDCHoughPlane::vote !!! xOffset should be (0 - 1). xOffset="
                << xOffset << std::endl;
#endif

    const unsigned x = unsigned(nX() * xOffset);
    const unsigned p = x ? (x - 1) * nY() : 0;
    const unsigned n = nX() * nY();

    for (unsigned i = 0; i < _nPatterns; i++) {
      unsigned id = _patterns[i] + p;
      if (id > n) id -= n;
      _cell[id] += weight;
    }

//     const unsigned x = unsigned(nX() * xOffset);
//     const unsigned p = x ? (x - 1) * nY() : 0;
//     const unsigned n = nX() * nY();
//     unsigned j = 0;
//     for (unsigned i = p; i < n; i++)
//         _cell[i] += _pattern[j++] * weight;
//     for (unsigned i = 0; i < p; i++)
//         _cell[i] += _pattern[j++] * weight;

//    std::cout << "--------------------------------" << std::endl;
  }

} // namespace Belle2


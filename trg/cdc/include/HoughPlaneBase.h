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
// Filename : HoughPlaneBase.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A base class to represent a Hough parameter plane
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGCDCHoughPlaneBase_FLAG_
#define TRGCDCHoughPlaneBase_FLAG_

#include <string>
#include <vector>
#include "trg/trg/Point2D.h"
#include "trg/trg/Area2D.h"
#include "trg/cdc/HoughTransformation.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCHPlaneBase TRGCDCHoughPlaneBase
#endif

namespace Belle2 {

/// A class to represent a Hough parameter plane.
  class TRGCDCHoughPlaneBase {

  public:

    /// Contructor.
    TRGCDCHoughPlaneBase(const std::string& name,
                         const TRGCDCHoughTransformation& transformation,
                         unsigned nX,
                         float xMin,
                         float xMax,
                         unsigned nY,
                         float yMin,
                         float yMax);

    /// Destructor
    virtual ~TRGCDCHoughPlaneBase();

  public:// Selectors

    /// returns name.
    std::string name(void) const;

    /// returns Hough transformation object.
    const TRGCDCHoughTransformation& transformation(void) const;

    /// returns charge for this plane.
    float charge(void) const;

    /// returns \# of x bins.
    unsigned nX(void) const;

    /// returns min. of x.
    float xMin(void) const;

    /// returns max. of x.
    float xMax(void) const;

    /// returns size of x bin.
    float xSize(void) const;

    /// return \# of y bins.
    unsigned nY(void) const;

    /// returns min. of y.
    float yMin(void) const;

    /// returns max. of y.
    float yMax(void) const;

    /// returns size of y bin.
    float ySize(void) const;

    /// returns count of a cell.
    virtual unsigned entry(unsigned id) const = 0;

    /// returns count of a cell.
    virtual unsigned entry(unsigned x, unsigned y) const = 0;

    /// returns max. count in a plane.
    virtual int maxEntry(void) const = 0;

    /// returns max. count in region.
    int maxEntryInRegion(unsigned id) const;

    /// returns serial ID for position (x, y).
    unsigned serialId(unsigned x, unsigned y) const;

    /// returns serial ID for position p.
    unsigned serialId(const TRGPoint2D& p) const;

    /// returns x and y for serialID.
    void id(unsigned serialId, unsigned& x, unsigned& y) const;

    /// returns position in Hough plain for a cell (x, y)..
    TRGPoint2D position(unsigned x, unsigned y) const;

    /// returns neighbor cell.
    unsigned neighbor(unsigned serialID, unsigned direction) const;

    /// returns neighbors.
    std::vector<unsigned> neighbors(unsigned serialID,
                                    unsigned windowSize = 1) const;

    /// returns regions.
    const std::vector<std::vector<unsigned> *>& regions(void) const;

    /// returns cell positions in the region.
    void locationInPlane(float x0, float y0, float x1, float y1,
                         unsigned& nFound,
                         unsigned& iX0, unsigned& iY0,
                         unsigned& iX1, unsigned& iY1) const;

    /// dumps debug information.
    virtual void dump(const std::string& message = std::string(""),
                      const std::string& prefix = std::string("")) const;

  public:// Modifiers

    /// sets and returns charge for this plane.
    float charge(float charge);

    /// sets and returns min. of x.
    float xMin(float newXMin);

    /// sets and returns max. of x.
    float xMax(float newXMax);

    /// sets and returns min. of y.
    float yMin(float newYMin);

    /// sets and returns max. of y.
    float yMax(float newYMax);

    /// Voring.
    virtual void vote(float rx,
                      float ry,
                      int weight = 1);

    /// Voring.
    virtual void vote(float rx,
                      float ry,
                      int charge,
                      int weight = 1);

    /// Votes using a pattern.
    virtual void vote(float xOffset, int weight = 1);

    /// registers a pattern..
    virtual void registerPattern(unsigned id) = 0;

    /// smoothing
//  void smooth(void);

    /// Sets entry.
    virtual unsigned setEntry(unsigned serialId, unsigned n) = 0;

    /// Sets region.
    void setRegion(std::vector<unsigned>*);

    /// Clears regions.
    void clearRegions(void);

    /// Clears all entries.
    virtual void clear(void) = 0;

  protected:

    /// Add to a cell.
    virtual void add(unsigned cellId, int weight) = 0;

  private:

    /// Name.
    const std::string _name;

    /// Hough transformation.
    const TRGCDCHoughTransformation& _trans;

    /// Track charge for this plane.
    float _charge;

    /// \# of x bins.
    const unsigned _nX;

    /// x min.
    float _xMin;

    /// x max.
    float _xMax;

    /// Size of x bin.
    float _xSize;

    /// \# of y bins.
    const unsigned _nY;

    /// y min.
    float _yMin;

    /// y max.
    float _yMax;

    /// Size of y bin.
    float _ySize;

    /// Area.
    const TRGArea2D _area;

    /// Regions.
    std::vector<std::vector<unsigned> *> _regions;
  };

//-----------------------------------------------------------------------------

  inline
  float
  TRGCDCHoughPlaneBase::charge(void) const
  {
    return _charge;
  }

  inline
  float
  TRGCDCHoughPlaneBase::charge(float a)
  {
    return _charge = a;
  }

  inline
  std::string
  TRGCDCHoughPlaneBase::name(void) const
  {
    return _name;
  }

  inline
  unsigned
  TRGCDCHoughPlaneBase::nX(void) const
  {
    return _nX;
  }

  inline
  float
  TRGCDCHoughPlaneBase::xMin(void) const
  {
    return _xMin;
  }

  inline
  float
  TRGCDCHoughPlaneBase::xMin(float a)
  {
    _xMin = a;
    _xSize = (_xMax - _xMin) / float(_nX);
    return _xMin;
  }

  inline
  float
  TRGCDCHoughPlaneBase::xMax(void) const
  {
    return _xMax;
  }

  inline
  float
  TRGCDCHoughPlaneBase::xMax(float a)
  {
    _xMax = a;
    _xSize = (_xMax - _xMin) / float(_nX);
    return _xMax;
  }

  inline
  float
  TRGCDCHoughPlaneBase::xSize(void) const
  {
    return _xSize;
  }

  inline
  unsigned
  TRGCDCHoughPlaneBase::nY(void) const
  {
    return _nY;
  }

  inline
  float
  TRGCDCHoughPlaneBase::yMin(void) const
  {
    return _yMin;
  }

  inline
  float
  TRGCDCHoughPlaneBase::yMin(float a)
  {
    _yMin = a;
    _ySize = (_yMax - _yMin) / float(_nY);
    return _yMin;
  }

  inline
  float
  TRGCDCHoughPlaneBase::yMax(void) const
  {
    return _yMax;
  }

  inline
  float
  TRGCDCHoughPlaneBase::yMax(float a)
  {
    _yMax = a;
    _ySize = (_yMax - _yMin) / float(_nY);
    return _yMax;
  }

  inline
  float
  TRGCDCHoughPlaneBase::ySize(void) const
  {
    return _ySize;
  }

  inline
  unsigned
  TRGCDCHoughPlaneBase::serialId(unsigned x, unsigned y) const
  {
    return _nY * x + y;
  }

  inline
  unsigned
  TRGCDCHoughPlaneBase::serialId(const TRGPoint2D& p) const
  {
    static const unsigned invalidPoint = _nX * _nY;
    if ((p.x() < _xMin) || (p.x() > _xMax)) return invalidPoint;
    if ((p.y() < _yMin) || (p.y() > _yMax)) return invalidPoint;
//    unsigned x = (p.x() - _xMin) / _xSize;
//    unsigned y = (p.y() - _yMin) / _ySize;
    unsigned x = unsigned((p.x() - _xMin) / _xSize);
    unsigned y = unsigned((p.y() - _yMin) / _ySize);
    return serialId(x, y);
  }

  inline
  TRGPoint2D
  TRGCDCHoughPlaneBase::position(unsigned x, unsigned y) const
  {
#ifdef TRASAN_DEBUG_DETAIL
//   std::cout << "x,y=" << x << "," << y
//             << ":_xMin,_yMin=" << _xMin << "," << _yMin
//               << ":_xSize,_ySize=" << _xSize << "," << _ySize << std::endl;
#endif

    return TRGPoint2D(_xMin + (float(x) + 0.5) * _xSize,
                      _yMin + (float(y) + 0.5) * _ySize);
  }

  inline
  std::vector<unsigned>
  TRGCDCHoughPlaneBase::neighbors(unsigned a, unsigned windowSize) const
  {
    const int maxi = _nY * _nX;
    std::vector<unsigned> tmp;

    const int xmin = - (int) windowSize;
    const int xmax = (int) windowSize;
    int ymin = - (int) windowSize;
    int ymax = (int) windowSize;

    unsigned x = 0;
    unsigned y = 0;
    id(a, x, y);
    if (((int) y + ymin) < 0) ymin = - (int) y;
    if (((int) y + ymax) >= (int) _nY) ymax = _nY - (int) y;

    for (int i = xmin; i <= xmax; i++) {
      for (int j = ymin; j <= ymax; j++) {
        int ii = a + i * _nY + j;
        if (ii == (int) a) continue;
        if (ii < 0) ii += maxi;
        if (ii >= maxi) ii -= maxi;
//            tmp.append(ii);
        tmp.push_back(ii);
      }
    }
    return tmp;
  }

  inline
  void
  TRGCDCHoughPlaneBase::setRegion(std::vector<unsigned>* a)
  {
//  _regions.append(a);
    _regions.push_back(a);
  }

  inline
  const std::vector<std::vector<unsigned> *>&
  TRGCDCHoughPlaneBase::regions(void) const
  {
    return _regions;
  }

  inline
  unsigned
  TRGCDCHoughPlaneBase::neighbor(unsigned id, unsigned dir) const
  {
    const unsigned maxi = _nY * _nX;
    if (dir == 0) { // top
      if ((id % _nY) != _nY - 1)
        return id + 1;
    } else if (dir == 1) { // top right
      if ((id % _nY) != _nY - 1) {
        const unsigned i = id + _nY + 1;
        if (i < maxi)
          return i;
        else
          return i - maxi;
      }
    } else if (dir == 2) { // right
      const unsigned i = id + _nY;
      if (i < maxi)
        return i;
      else
        return i - maxi;
    } else if (dir == 3) { // bottom right
      if ((id % _nY) != 0) {
        const unsigned i = id + _nY - 1;
        if (i < maxi)
          return i;
        else
          return i - maxi;
      }
    } else if (dir == 4) { // bottom
      if ((id % _nY) != 0)
        return id - 1;
    } else if (dir == 5) { // bottom left
      if ((id % _nY) != 0) {
        const int i = id + _nY - 1;
        if (i > 0)
          return (unsigned) i;
        else
          return (unsigned) i + maxi;
      }
    } else if (dir == 6) { // left
      const int i = id - _nY;
      if (i > 0)
        return (unsigned) i;
      else
        return (unsigned) i + maxi;
    } else if (dir == 7) { // top left
      if ((id % _nY) != _nY - 1) {
        const int i = id - _nY + 1;
        if (i > 0)
          return (unsigned) i;
        else
          return (unsigned) i + maxi;
      }
    }

    return id;
  }

  inline
  void
  TRGCDCHoughPlaneBase::id(unsigned serialId, unsigned& x, unsigned& y) const
  {
    x = serialId / _nY;
    y = serialId % _nY;
  }

  inline
  void
  TRGCDCHoughPlaneBase::clearRegions(void)
  {
    // HepAListDeleteAll(_regions);
    _regions.clear();
  }

  inline
  void
  TRGCDCHoughPlaneBase::clear(void)
  {
    clearRegions();
  }

  inline
  void
  TRGCDCHoughPlaneBase::vote(float rx,
                             float ry,
                             int weight)
  {
    vote(rx, ry, 0, weight);
  }

  inline
  void
  TRGCDCHoughPlaneBase::vote(float, int)
  {
// do nothing
  }

  inline
  const  TRGCDCHoughTransformation&
  TRGCDCHoughPlaneBase::transformation(void) const
  {
    return _trans;
  }

} // namespace Belle

#endif

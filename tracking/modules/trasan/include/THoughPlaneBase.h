//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : THoughPlaneBase.h
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A base class to represent a Hough parameter plane
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef THoughPlaneBase_FLAG_
#define THoughPlaneBase_FLAG_

#include <string>

#define HEP_SHORT_NAMES
#include "tracking/modules/trasan/CList.h"
#include "tracking/modules/trasan/TArea.h"
#include "tracking/modules/trasan/THoughTransformation.h"
#ifdef TRASAN_WINDOW
#include "tracking/modules/trasan/TWindow.h"
#endif

namespace Belle {

/// A class to represent a Hough parameter plane.
  class THoughPlaneBase {

  public:
    /// Contructor.
    THoughPlaneBase(const std::string& name,
                    unsigned nX,
                    float xMin,
                    float xMax,
                    unsigned nY,
                    float yMin,
                    float yMax);

    /// Destructor
    virtual ~THoughPlaneBase();

  public:// Selectors
    /// returns name.
    std::string name(void) const;

    unsigned nX(void) const;
    float xMin(void) const;
    float xMin(float newXMin);
    float xMax(void) const;
    float xMax(float newXMax);
    float xSize(void) const;
    unsigned nY(void) const;
    float yMin(void) const;
    float yMin(float newYMin);
    float yMax(void) const;
    float yMax(float newYMax);
    float ySize(void) const;
    virtual unsigned entry(unsigned id) const = 0;
    virtual unsigned entry(unsigned x, unsigned y) const = 0;
    virtual int maxEntry(void) const = 0;
    int maxEntryInRegion(unsigned id) const;
    unsigned serialID(unsigned x, unsigned y) const;
    unsigned serialID(const TPoint2D& p) const;
    void id(unsigned serialId, unsigned& x, unsigned& y) const;
    TPoint2D position(unsigned x, unsigned y) const;
    unsigned neighbor(unsigned serialID, unsigned direction) const;
    CList<unsigned> neighbors(unsigned serialID,
                              unsigned windowSize = 1) const;
    const AList<CList<unsigned> > & regions(void) const;

    /// returns cell positions in the region.
    void locationInPlane(float x0, float y0, float x1, float y1,
                         unsigned& nFound,
                         unsigned& iX0, unsigned& iY0,
                         unsigned& iX1, unsigned& iY1) const;

    virtual void dump(const std::string& message = std::string(""),
                      const std::string& prefix = std::string("")) const;

  public:// Modifiers
//    virtual void vote(float x, float y) = 0;
//    virtual void vote(float x0, float y0, float x1, float y1) = 0;
    virtual void vote(float rx,
                      float ry,
                      const THoughTransformation& hough,
                      int weight = 1);
    virtual void vote(float rx,
                      float ry,
                      int charge,
                      const THoughTransformation& hough,
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
    void setRegion(CList<unsigned> *);
    void clearRegion(void);

    /// Clears all entries.
    virtual void clear(void) = 0;

  protected:
    /// Add to a cell.
    virtual void add(unsigned cellId, int weight) = 0;

  private:
    const std::string _name;
    const unsigned _nX;
    float _xMin;
    float _xMax;
    float _xSize;
    const unsigned _nY;
    float _yMin;
    float _yMax;
    float _ySize;
    const TArea _area;
    AList<CList<unsigned> > _regions;

    friend class THoughPlaneMulti;
  };

  inline
  std::string
  THoughPlaneBase::name(void) const
  {
    return _name;
  }

  inline
  unsigned
  THoughPlaneBase::nX(void) const
  {
    return _nX;
  }

  inline
  float
  THoughPlaneBase::xMin(void) const
  {
    return _xMin;
  }

  inline
  float
  THoughPlaneBase::xMin(float a)
  {
    _xMin = a;
    _xSize = (_xMax - _xMin) / float(_nX);
    return _xMin;
  }

  inline
  float
  THoughPlaneBase::xMax(void) const
  {
    return _xMax;
  }

  inline
  float
  THoughPlaneBase::xMax(float a)
  {
    _xMax = a;
    _xSize = (_xMax - _xMin) / float(_nX);
    return _xMax;
  }

  inline
  float
  THoughPlaneBase::xSize(void) const
  {
    return _xSize;
  }

  inline
  unsigned
  THoughPlaneBase::nY(void) const
  {
    return _nY;
  }

  inline
  float
  THoughPlaneBase::yMin(void) const
  {
    return _yMin;
  }

  inline
  float
  THoughPlaneBase::yMin(float a)
  {
    _yMin = a;
    _ySize = (_yMax - _yMin) / float(_nY);
    return _yMin;
  }

  inline
  float
  THoughPlaneBase::yMax(void) const
  {
    return _yMax;
  }

  inline
  float
  THoughPlaneBase::yMax(float a)
  {
    _yMax = a;
    _ySize = (_yMax - _yMin) / float(_nY);
    return _yMax;
  }

  inline
  float
  THoughPlaneBase::ySize(void) const
  {
    return _ySize;
  }

  inline
  unsigned
  THoughPlaneBase::serialID(unsigned x, unsigned y) const
  {
    return _nY * x + y;
  }

  inline
  unsigned
  THoughPlaneBase::serialID(const TPoint2D& p) const
  {
    static const unsigned invalidPoint = _nX * _nY;
    if ((p.x() < _xMin) || (p.x() > _xMax)) return invalidPoint;
    if ((p.y() < _yMin) || (p.y() > _yMax)) return invalidPoint;
//    unsigned x = (p.x() - _xMin) / _xSize;
//    unsigned y = (p.y() - _yMin) / _ySize;
    unsigned x = unsigned((p.x() - _xMin) / _xSize);
    unsigned y = unsigned((p.y() - _yMin) / _ySize);
    return serialID(x, y);
  }

  inline
  TPoint2D
  THoughPlaneBase::position(unsigned x, unsigned y) const
  {
#ifdef TRASAN_DEBUG_DETAIL
//   std::cout << "x,y=" << x << "," << y
//             << ":_xMin,_yMin=" << _xMin << "," << _yMin
//         << ":_xSize,_ySize=" << _xSize << "," << _ySize << std::endl;
#endif

    return TPoint2D(_xMin + (float(x) + 0.5) * _xSize,
                    _yMin + (float(y) + 0.5) * _ySize);
  }

  inline
  CList<unsigned>
  THoughPlaneBase::neighbors(unsigned a, unsigned windowSize) const
  {
    const int maxi = _nY * _nX;
    CList<unsigned> tmp;

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
        tmp.append(ii);
      }
    }
    return tmp;
  }

  inline
  void
  THoughPlaneBase::setRegion(CList<unsigned> * a)
  {
    _regions.append(a);
  }

  inline
  const AList<CList<unsigned> > &
  THoughPlaneBase::regions(void) const
  {
    return _regions;
  }

  inline
  unsigned
  THoughPlaneBase::neighbor(unsigned id, unsigned dir) const
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
  THoughPlaneBase::id(unsigned serialId, unsigned& x, unsigned& y) const
  {
    x = serialId / _nY;
    y = serialId % _nY;
  }

  inline
  void
  THoughPlaneBase::clearRegion(void)
  {
    HepAListDeleteAll(_regions);
  }

  inline
  void
  THoughPlaneBase::clear(void)
  {
    clearRegion();
  }

  inline
  void
  THoughPlaneBase::vote(float rx,
                        float ry,
                        const THoughTransformation& hough,
                        int weight)
  {
    vote(rx, ry, 0, hough, weight);
  }

  inline
  void
  THoughPlaneBase::vote(float xOffset, int weight)
  {
// do nothing
  }

} // namespace Belle

#endif

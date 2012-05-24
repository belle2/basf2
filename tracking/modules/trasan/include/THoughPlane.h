//-----------------------------------------------------------------------------
// $Id: THoughPlane.h 10305 2007-12-05 05:19:24Z yiwasaki $
//-----------------------------------------------------------------------------
// Filename : THoughPlane.h
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a Hough parameter plane
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.4  2005/11/03 23:20:35  yiwasaki
// Trasan 3.18 : Hough finder 0.12
//
// Revision 1.3  2005/04/18 23:42:04  yiwasaki
// Trasan 3.17 : Only Hough finder is modified
//
// Revision 1.2  2005/03/11 03:58:34  yiwasaki
// Trasan 3.15 : Only Hough finder is modified
//
// Revision 1.1  2004/02/18 04:07:46  yiwasaki
// Trasan 3.09 : Option to simualte the small cell CDC with the normal cell CDC added, new classes for Hough finder added
//
//-----------------------------------------------------------------------------

#ifndef THoughPlane_FLAG_
#define THoughPlane_FLAG_

#include <strings.h>
#include <string>

#include "tracking/modules/trasan/THoughPlaneBase.h"

namespace Belle {

/// A class to represent a Hough parameter plane.
  class THoughPlane : public THoughPlaneBase {

  public:
    /// Contructor.
    THoughPlane(const std::string& name,
                unsigned nX,
                float xMin,
                float xMax,
                unsigned nY,
                float yMin,
                float yMax);

    /// Destructor
    virtual ~THoughPlane();

  public:// Selectors
    unsigned entry(unsigned id) const;
    unsigned entry(unsigned x, unsigned y) const;
    int maxEntry(void) const;

    /// returns # of active cells in the pattern.
    virtual unsigned nActiveCellsInPattern(void) const;

  public:// Modifiers
    /// Sets entry.
    unsigned setEntry(unsigned serialId, unsigned n);

    /// clear all entries.
    void clear(void);

    /// Clears entires only.
    void clearCells(void);

    /// Registers a pattern on a Hough plane with (r, phi=0).
    void registerPattern(unsigned id);

    /// Votes using a pattern.
    virtual void voteByPattern(float xOffset, int weight = 1);

  protected:
    /// Add to a cell.
    void add(unsigned cellId, int weight);

  private:
    int* const _cell;
    unsigned* _patterns;
    unsigned _nPatterns;

    friend class THoughPlaneMulti;
  };

//-----------------------------------------------------------------------------

  inline
  unsigned
  THoughPlane::setEntry(unsigned serialId, unsigned n)
  {
    return _cell[serialId] = n;
  }

  inline
  void
  THoughPlane::clear(void)
  {
    bzero(_cell, nX() * nY() * sizeof(int));
//     for (unsigned i = 0; i < nX() * nY(); i++)
//  _cell[i] = 0;
    THoughPlaneBase::clear();
  }

  inline
  void
  THoughPlane::clearCells(void)
  {
    bzero(_cell, nX() * nY() * sizeof(int));
  }

  inline
  unsigned
  THoughPlane::entry(unsigned id) const
  {
    if (id < nX() * nY()) return _cell[id];
    return 0;
  }

  inline
  unsigned
  THoughPlane::entry(unsigned x, unsigned y) const
  {
    if (x < nX())
      if (y < nY())
        return _cell[nY() * x + y];
    return 0;
  }

  inline
  int
  THoughPlane::maxEntry(void) const
  {
    int m = 0;
    for (unsigned i = 0; i < nX() * nY(); i++)
      if (_cell[i] > m)
        m = _cell[i];
    return m;
  }

  inline
  unsigned
  THoughPlane::nActiveCellsInPattern(void) const
  {
    return _nPatterns;
  }

  inline
  void
  THoughPlane::add(unsigned a, int w)
  {
    _cell[a] += w;
    if (_cell[a] < 0)
      _cell[a] = 0;
  }

} // namespace Belle

#endif

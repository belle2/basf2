//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : HoughPlane.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a Hough parameter plane
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGCDCHoughPlane_FLAG_
#define TRGCDCHoughPlane_FLAG_

#include <string.h>
#include "trg/cdc/HoughPlaneBase.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCHPlane TRGCDCHoughPlane
#endif

namespace Belle2 {

/// A class to represent a Hough parameter plane.
class TRGCDCHoughPlane : public TRGCDCHoughPlaneBase {

  public:
    /// Contructor.
    TRGCDCHoughPlane(const std::string & name,
                     unsigned nX,
                     float xMin,
                     float xMax,
                     unsigned nY,
                     float yMin,
                     float yMax);

    /// Destructor
    virtual ~TRGCDCHoughPlane();

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
    int * const _cell;
    unsigned * _patterns;
    unsigned _nPatterns;

    friend class TRGCDCHoughPlaneMulti;
};

//-----------------------------------------------------------------------------

inline
unsigned
TRGCDCHoughPlane::setEntry(unsigned serialId, unsigned n) {
    return _cell[serialId] = n;
}

inline
void
TRGCDCHoughPlane::clear(void) {
//  bzero(_cell, nX() * nY() * sizeof(int));
    memset(_cell, 0, nX() * nY() * sizeof(int));
//     for (unsigned i = 0; i < nX() * nY(); i++)
//         _cell[i] = 0;
    TRGCDCHoughPlaneBase::clear();
}

inline
void
TRGCDCHoughPlane::clearCells(void) {
//  bzero(_cell, nX() * nY() * sizeof(int));
    memset(_cell, 0, nX() * nY() * sizeof(int));
}

inline
unsigned
TRGCDCHoughPlane::entry(unsigned id) const {
    if (id < nX() * nY()) return _cell[id];
    return 0;
}

inline
unsigned
TRGCDCHoughPlane::entry(unsigned x, unsigned y) const {
    if (x < nX())
        if (y < nY())
            return _cell[nY() * x + y];
    return 0;
}

inline
int
TRGCDCHoughPlane::maxEntry(void) const {
    int m = 0;
    for (unsigned i = 0; i < nX() * nY(); i++)
        if (_cell[i] > m)
            m = _cell[i];
    return m;
}

inline
unsigned
TRGCDCHoughPlane::nActiveCellsInPattern(void) const {
    return _nPatterns;
}

inline
void
TRGCDCHoughPlane::add(unsigned a, int w) {
    _cell[a] += w;
    if (_cell[a] < 0)
        _cell[a] = 0;
}

} // namespace Belle2

#endif

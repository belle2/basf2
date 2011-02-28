//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : HoughPlaneBoolean.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a Hough parameter plane with simple
//               counter.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGCDCHoughPlaneBoolean_FLAG_
#define TRGCDCHoughPlaneBoolean_FLAG_

#include <string.h>
#include "trg/cdc/HoughPlaneBase.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCHPlaneBoolean TRGCDCHoughPlaneBoolean
#endif

namespace Belle2 {

/// A class to represent a Hough parameter plane.
class TRGCDCHoughPlaneBoolean : public TRGCDCHoughPlaneBase {

  public:
    /// Contructor.
    TRGCDCHoughPlaneBoolean(const std::string & name,
                            const TRGCDCHoughTransformation & transformation,
                            unsigned nX,
                            float xMin,
                            float xMax,
                            unsigned nY,
                            float yMin,
                            float yMax);

    /// Destructor
    virtual ~TRGCDCHoughPlaneBoolean();

  public:// Selectors
    unsigned entry(unsigned id) const;
    unsigned entry(unsigned x, unsigned y) const;
    int maxEntry(void) const;

  public:// Modifiers
    /// Sets entry.
    unsigned setEntry(unsigned serialId, unsigned n);

    /// clear all entries.
    void clear(void);

    /// Votes.
    void vote(float rx,
              float ry,
              int weight = 1);

    /// Votes with charge decision.
    void vote(float rx,
              float ry,
              float charge,
              int weight = 1);

    /// Votes with charge decision.
    void voteUsedInTrasan(float rx,
                          float ry,
                          float charge,
                          int weight = 1);

    /// Votes using registered pattern.
    void vote(unsigned patternId, int weight);

    /// registers a pattern..
    void registerPattern(unsigned id);

    /// allocate memory for patterns.
    void preparePatterns(unsigned nPatterns);

  protected:
    /// Add to a cell.
    void add(unsigned cellId, int weight);

  private:
    unsigned _n;
    unsigned * const _cell;
    unsigned _nPatterns;
    unsigned ** _patterns;
    unsigned * _nActive;

    friend class TRGCDCHoughPlaneMulti2;
};

inline
unsigned
TRGCDCHoughPlaneBoolean::setEntry(unsigned serialId, unsigned n) {
    const unsigned b0 = serialId / 32;
    const unsigned b1 = serialId % 32;

#ifdef TRASAN_DEBUG
    if (b0 >= _n)
        std::cout << "TRGCDCHoughPlaneBoolean !!! given serialId is too large : "
               << "max=" << _n * 32 << ",serialId=" << serialId << std::endl;
#endif

    if (n > 0)
        _cell[b0] |= (1 << b1);
    else
        _cell[b0] &= (~(1 << b1));

    return (_cell[b0] >> b1) & 1;
}

inline
void
TRGCDCHoughPlaneBoolean::clear(void) {
//  bzero(_cell, _n * sizeof(unsigned));
    memset(_cell, 0, _n * sizeof(unsigned));
    TRGCDCHoughPlaneBase::clear();
}

inline
void
TRGCDCHoughPlaneBoolean::vote(float rx,
                              float ry,
                              int weight) {
    vote(rx, ry, 0, weight);
}

inline
unsigned
TRGCDCHoughPlaneBoolean::entry(unsigned serialId) const {
    const unsigned b0 = serialId / 32;
    const unsigned b1 = serialId % 32;

#ifdef TRASAN_DEBUG
    if (b0 >= _n)
        std::cout << "TRGCDCHoughPlaneBoolean::entry !!! given serialId is too large"
               << " : "
               << "max=" << _n * 32 << ",serialId=" << serialId << std::endl;
#endif

    return (_cell[b0] >> b1) & 1;
}

inline
unsigned
TRGCDCHoughPlaneBoolean::entry(unsigned x, unsigned y) const {
    return entry(nY() * x + y);
}

inline
int
TRGCDCHoughPlaneBoolean::maxEntry(void) const {
#ifdef TRASAN_DEBUG
    std::cout << "TRGCDCHoughPlaneBoolean::maxEntry !!! "
           << " this function has no meaning for TRGCDCHoughPlaneBooolean object"
           << std::endl;
#endif
    return 1;
}

inline
void
TRGCDCHoughPlaneBoolean::add(unsigned a, int b) {
    if (b > 0)
        setEntry(a, 1);
    else
        setEntry(a, 0);
}

} // namespace Belle2

#endif

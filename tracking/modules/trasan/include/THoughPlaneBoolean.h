//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : THoughPlaneBoolean.h
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

#ifndef THoughPlaneBoolean_FLAG_
#define THoughPlaneBoolean_FLAG_

#include <strings.h>
#include <string>

#include "tracking/modules/trasan/THoughPlaneBase.h"

namespace Belle {

/// A class to represent a Hough parameter plane.
  class THoughPlaneBoolean : public THoughPlaneBase {

  public:
    /// Contructor.
    THoughPlaneBoolean(const std::string& name,
                       unsigned nX,
                       float xMin,
                       float xMax,
                       unsigned nY,
                       float yMin,
                       float yMax);

    /// Destructor
    virtual ~THoughPlaneBoolean();

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
              const THoughTransformation& hough,
              int weight = 1);
    void vote(float rx,
              float ry,
              float charge,
              const THoughTransformation& hough,
              int weight = 1);
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
    unsigned* const _cell;
    unsigned _nPatterns;
    unsigned** _patterns;
    unsigned* _nActive;

    friend class THoughPlaneMulti2;
  };

  inline
  unsigned
  THoughPlaneBoolean::setEntry(unsigned serialId, unsigned n)
  {
    const unsigned b0 = serialId / 32;
    const unsigned b1 = serialId % 32;

#ifdef TRASAN_DEBUG
    if (b0 >= _n)
      std::cout << "THoughPlaneBoolean !!! given serialId is too large : "
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
  THoughPlaneBoolean::clear(void)
  {
    bzero(_cell, _n * sizeof(unsigned));
    THoughPlaneBase::clear();
  }

  inline
  void
  THoughPlaneBoolean::vote(float rx,
                           float ry,
                           const THoughTransformation& hough,
                           int weight)
  {
    vote(rx, ry, 0, hough, weight);
  }

  inline
  unsigned
  THoughPlaneBoolean::entry(unsigned serialId) const
  {
    const unsigned b0 = serialId / 32;
    const unsigned b1 = serialId % 32;

#ifdef TRASAN_DEBUG
    if (b0 >= _n)
      std::cout << "THoughPlaneBoolean::entry !!! given serialId is too large"
                << " : "
                << "max=" << _n * 32 << ",serialId=" << serialId << std::endl;
#endif

    return (_cell[b0] >> b1) & 1;
  }

  inline
  unsigned
  THoughPlaneBoolean::entry(unsigned x, unsigned y) const
  {
    return entry(nY() * x + y);
  }

  inline
  int
  THoughPlaneBoolean::maxEntry(void) const
  {
#ifdef TRASAN_DEBUG
    std::cout << "THoughPlaneBoolean::maxEntry !!! "
              << " this function has no meaning for THoughPlaneBooolean object"
              << std::endl;
#endif
    return 1;
  }

  inline
  void
  THoughPlaneBoolean::add(unsigned a, int b)
  {
    if (b > 0)
      setEntry(a, 1);
    else
      setEntry(a, 0);
  }

} // namespace Belle

#endif

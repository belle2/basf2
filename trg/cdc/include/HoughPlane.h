/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//-----------------------------------------------------------------------------
// Description : A class to represent a Hough parameter plane
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
    TRGCDCHoughPlane(const std::string& name,
                     const TRGCDCHoughTransformation& transformation,
                     unsigned nX,
                     float xMin,
                     float xMax,
                     unsigned nY,
                     float yMin,
                     float yMax);

    /// Destructor
    virtual ~TRGCDCHoughPlane();

    /// Copy construtor, deleted
    TRGCDCHoughPlane(TRGCDCHoughPlane&) = delete;

    /// Assignment operator, deleted
    TRGCDCHoughPlane& operator=(TRGCDCHoughPlane&) = delete;

  public:// Selectors

    /// returns entry in a cell.
    unsigned entry(unsigned id) const override;

    /// returns entry in a cell.
    unsigned entry(unsigned x, unsigned y) const override;

    /// returns max. entry in a plane.
    int maxEntry(void) const override;

    /// returns # of active cells in the pattern.
    virtual unsigned nActiveCellsInPattern(void) const;

  public:// Modifiers

    /// Sets entry.
    unsigned setEntry(unsigned serialId, unsigned n) override;

    /// clear all entries.
    // cppcheck-suppress virtualCallInConstructor
    void clear(void) override;

    /// Clears entires only.
    void clearCells(void);

    /// Registers a pattern on a Hough plane with (r, phi=0).
    void registerPattern(unsigned id) override;

    /// Votes using a pattern.
    virtual void voteByPattern(float xOffset, int weight = 1);

  protected:

    /// Add to a cell.
    void add(unsigned cellId, int weight) override;

  private:

    /// Counters.
    int* const _cell;

    /// Curve patterns.
    unsigned* _patterns;

    /// \# of curve patterns.
    unsigned _nPatterns;

    friend class TRGCDCHoughPlaneMulti;
  };

//-----------------------------------------------------------------------------

  inline
  unsigned
  TRGCDCHoughPlane::setEntry(unsigned serialId, unsigned n)
  {
    return _cell[serialId] = n;
  }

  inline
  void
  TRGCDCHoughPlane::clear(void)
  {
    memset(_cell, 0, nX() * nY() * sizeof(int));
    TRGCDCHoughPlaneBase::clear();
  }

  inline
  void
  TRGCDCHoughPlane::clearCells(void)
  {
    memset(_cell, 0, nX() * nY() * sizeof(int));
  }

  inline
  unsigned
  TRGCDCHoughPlane::entry(unsigned id) const
  {
    if (id < nX() * nY()) return _cell[id];
    return 0;
  }

  inline
  unsigned
  TRGCDCHoughPlane::entry(unsigned x, unsigned y) const
  {
    if (x < nX())
      if (y < nY())
        return _cell[nY() * x + y];
    return 0;
  }

  inline
  int
  TRGCDCHoughPlane::maxEntry(void) const
  {
    int m = 0;
    for (unsigned i = 0; i < nX() * nY(); i++)
      if (_cell[i] > m)
        m = _cell[i];
    return m;
  }

  inline
  unsigned
  TRGCDCHoughPlane::nActiveCellsInPattern(void) const
  {
    return _nPatterns;
  }

  inline
  void
  TRGCDCHoughPlane::add(unsigned a, int w)
  {
    _cell[a] += w;
    if (_cell[a] < 0)
      _cell[a] = 0;
  }

} // namespace Belle2

#endif

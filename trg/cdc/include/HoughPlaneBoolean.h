/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//-----------------------------------------------------------------------------
// Description : A class to represent a Hough parameter plane with simple
//               counter.
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
    TRGCDCHoughPlaneBoolean(const std::string& name,
                            const TRGCDCHoughTransformation& transformation,
                            unsigned nX,
                            float xMin,
                            float xMax,
                            unsigned nY,
                            float yMin,
                            float yMax);

    /// Destructor
    virtual ~TRGCDCHoughPlaneBoolean();

    /// Copy constructor, deleted
    TRGCDCHoughPlaneBoolean(TRGCDCHoughPlaneBoolean&) = delete;

    /// Assignement operator, delete
    TRGCDCHoughPlaneBoolean& operator=(TRGCDCHoughPlaneBoolean&) = delete;

  public:// Selectors

    /// returns entry in a cell.
    unsigned entry(unsigned id) const override;

    unsigned entry(unsigned x, unsigned y) const override;
    int maxEntry(void) const override;

    /// returns pattern ID which activates specified cell.
    const std::vector<unsigned>& patternId(unsigned cellId) const;

  public:// Modifiers

    /// Sets entry.
    unsigned setEntry(unsigned serialId, unsigned n) override;

    /// clear all entries.
    // cppcheck-suppress virtualCallInConstructor
    void clear(void) override;

    /// Votes.
    void vote(float rx,
              float ry,
              int weight = 1) override;

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-virtual"
#endif

    /// Votes with charge decision.
    // using TRGCDCHoughPlaneBase::vote; // to be checked
    void vote(float rx,
              float ry,
              float charge,
              int weight = 1);

    /// Votes using registered pattern.
    // using TRGCDCHoughPlaneBase::vote; // to be checked
    void vote(unsigned patternId, int weight);

#ifdef __clang__
#pragma clang diagnostic pop
#endif

    /// Votes with charge decision.
    void voteUsedInTrasan(float rx,
                          float ry,
                          float charge,
                          int weight = 1);

    /// registers a pattern..
    void registerPattern(unsigned id) override;

    /// allocate memory for patterns.
    void preparePatterns(unsigned nPatterns);

  protected:

    /// Add to a cell.
    void add(unsigned cellId, int weight) override;

  private:
    /// nX * nY / 32 + 1
    unsigned _n;
    /// cell
    unsigned* const _cell;
    /// number of patterns
    unsigned _nPatterns;
    /// patterns
    unsigned** _patterns;
    /// number of active cells
    unsigned* _nActive;

    /// Pattern ID's for each cell
    std::vector<unsigned>* _reverse;

    friend class TRGCDCHoughPlaneMulti2;
  };

//-----------------------------------------------------------------------------

  inline
  unsigned
  TRGCDCHoughPlaneBoolean::setEntry(unsigned serialId, unsigned n)
  {
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
  TRGCDCHoughPlaneBoolean::clear(void)
  {
    memset(_cell, 0, _n * sizeof(unsigned));
    TRGCDCHoughPlaneBase::clear();
  }

  inline
  void
  TRGCDCHoughPlaneBoolean::vote(float rx,
                                float ry,
                                int weight)
  {
    vote(rx, ry, 0, weight);
  }

  inline
  unsigned
  TRGCDCHoughPlaneBoolean::entry(unsigned serialId) const
  {
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
  TRGCDCHoughPlaneBoolean::entry(unsigned x, unsigned y) const
  {
    return entry(nY() * x + y);
  }

  inline
  int
  TRGCDCHoughPlaneBoolean::maxEntry(void) const
  {
#ifdef TRASAN_DEBUG
    std::cout << "TRGCDCHoughPlaneBoolean::maxEntry !!! "
              << " this function has no meaning for TRGCDCHoughPlaneBooolean object"
              << std::endl;
#endif
    return 1;
  }

  inline
  void
  TRGCDCHoughPlaneBoolean::add(unsigned a, int b)
  {
    if (b > 0)
      setEntry(a, 1);
    else
      setEntry(a, 0);
  }

  inline
  const std::vector<unsigned>&
  TRGCDCHoughPlaneBoolean:: patternId(unsigned cellId) const
  {
    return _reverse[cellId];
  }

} // namespace Belle2

#endif

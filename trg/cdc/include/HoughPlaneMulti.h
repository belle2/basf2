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
// Filename : HoughPlaneMulti.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent multi Hough parameter planes
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGCDCHoughPlaneMulti_FLAG_
#define TRGCDCHoughPlaneMulti_FLAG_

#include "trg/cdc/HoughPlane.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCHPlaneMulti TRGCDCHoughPlaneMulti
#endif

namespace Belle2 {

//#define N_LAYERS 64
/// number of layers
#define N_LAYERS 6

/// A class to represent a Hough parameter plane.
  class TRGCDCHoughPlaneMulti : public TRGCDCHoughPlane {

  public:
    /// Contructor.
    TRGCDCHoughPlaneMulti(const std::string& name,
                          const TRGCDCHoughTransformation& transformation,
                          unsigned nX,
                          float xMin,
                          float xMax,
                          unsigned nY,
                          float yMin,
                          float yMax,
                          unsigned nLayers);

    /// Destructor
    virtual ~TRGCDCHoughPlaneMulti();

  public:// Selectors
    /// returns # of active cells in the pattern.
    using TRGCDCHoughPlane::nActiveCellsInPattern; // to be checked
    /// # of active cells in the pattern.
    virtual unsigned nActiveCellsInPattern(unsigned layerId) const;

    /// dump debug info
    void dump(const std::string& message = std::string(""),
              const std::string& prefix = std::string("")) const override;

  public:// Modifiers
    /// Clears all entries.
    void clear(void) override;

    /// vote
    using TRGCDCHoughPlaneBase::vote; // to be checked
    /// vote
    void vote(float rx,
              float ry,
              float charge,
              const TRGCDCHoughTransformation& hough,
              unsigned weight,
              unsigned layerId);
    // using TRGCDCHoughPlaneBase::vote; // to be checked
    /// vote
    void vote(float phi, unsigned layerId, int weight);
    /// merge function
    void merge(void);

    /// registers a pattern..
    void registerPattern(unsigned id) override;

  private:
//  AList<TRGCDCHoughPlane> _layers;
    /// number of layers
    unsigned _nLayers;
    /// layers of TRGCDCHoughPlane
    TRGCDCHoughPlane* _layers[N_LAYERS];
    /// usage of each layer
    bool _usage[N_LAYERS];
  };

  inline
  void
  TRGCDCHoughPlaneMulti::clear(void)
  {
    for (unsigned i = 0; i < N_LAYERS; i++)
      if (_usage[i])
        _layers[i]->clear();
    TRGCDCHoughPlane::clear();
  }

  inline
  void
  TRGCDCHoughPlaneMulti::registerPattern(unsigned id)
  {
    _layers[id]->registerPattern(0);
  }

  inline
  unsigned
  TRGCDCHoughPlaneMulti::nActiveCellsInPattern(unsigned) const
  {
//    return _layers[id]->nActiveCellsInPattern();
    return 999;
  }

  inline
  void
  TRGCDCHoughPlaneMulti::dump(const std::string& a, const std::string& b) const
  {
    TRGCDCHoughPlaneBase::dump(a, b);
  }

} // namespace Belle2

#endif

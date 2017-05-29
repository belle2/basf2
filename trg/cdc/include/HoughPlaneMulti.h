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
    virtual unsigned nActiveCellsInPattern(unsigned layerId) const;

    void dump(const std::string& message = std::string(""),
              const std::string& prefix = std::string("")) const;

  public:// Modifiers
    /// Clears all entries.
    void clear(void);

    /// vote
    void vote(float rx,
              float ry,
              float charge,
              const TRGCDCHoughTransformation& hough,
              unsigned weight,
              unsigned layerId);
    void vote(float phi, unsigned layerId, int weight);
    void merge(void);

    /// registers a pattern..
    void registerPattern(unsigned id);

  private:
//  AList<TRGCDCHoughPlane> _layers;
    unsigned _nLayers;
    TRGCDCHoughPlane* _layers[N_LAYERS];
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

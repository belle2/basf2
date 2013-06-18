//-----------------------------------------------------------------------------
// $Id: THoughPlaneMulti.h 10305 2007-12-05 05:19:24Z yiwasaki $
//-----------------------------------------------------------------------------
// Filename : THoughPlaneMulti.h
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent multi Hough parameter planes
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.1  2005/04/18 23:42:04  yiwasaki
// Trasan 3.17 : Only Hough finder is modified
//
//-----------------------------------------------------------------------------

#ifndef THoughPlaneMulti_FLAG_
#define THoughPlaneMulti_FLAG_


#include "tracking/modules/trasan/THoughPlane.h"

namespace Belle {

#define N_LAYERS 64

/// A class to represent a Hough parameter plane.
  class THoughPlaneMulti : public THoughPlane {

  public:
    /// Contructor.
    THoughPlaneMulti(const std::string& name,
                     unsigned nX,
                     float xMin,
                     float xMax,
                     unsigned nY,
                     float yMin,
                     float yMax,
                     unsigned nLayers);

    /// Destructor
    virtual ~THoughPlaneMulti();

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
              const THoughTransformation& hough,
              unsigned weight,
              unsigned layerId);
    void vote(float phi, unsigned layerId, int weight);
    void merge(void);

    /// registers a pattern..
    void registerPattern(unsigned id);

  private:
//  AList<THoughPlane> _layers;
    unsigned _nLayers;
    THoughPlane* _layers[N_LAYERS];
    bool _usage[N_LAYERS];
  };

  inline
  void
  THoughPlaneMulti::clear(void)
  {
    for (unsigned i = 0; i < N_LAYERS; i++)
      if (_usage[i])
        _layers[i]->clear();
    THoughPlane::clear();
  }

  inline
  void
  THoughPlaneMulti::registerPattern(unsigned id)
  {
    _layers[id]->registerPattern(0);
  }

inline
unsigned
THoughPlaneMulti::nActiveCellsInPattern(unsigned) const {
//THoughPlaneMulti::nActiveCellsInPattern(unsigned id) const
//    return _layers[id]->nActiveCellsInPattern();
    return 0;
}

  inline
  void
  THoughPlaneMulti::dump(const std::string& a, const std::string& b) const
  {
    THoughPlaneBase::dump(a, b);
  }

} // namespace Belle

#endif

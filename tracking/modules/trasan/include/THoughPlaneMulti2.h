//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : THoughPlaneMulti2.h
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent multi Hough parameter planes (version 2)
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef THoughPlaneMulti2_FLAG_
#define THoughPlaneMulti2_FLAG_


#include "tracking/modules/trasan/THoughPlane.h"
#include "tracking/modules/trasan/THoughPlaneBoolean.h"
#define N_LAYERS 64

namespace Belle {

/// A class to represent a Hough parameter plane.
  class THoughPlaneMulti2 : public THoughPlane {

  public:
    /// Contructor.
    THoughPlaneMulti2(const std::string& name,
                      unsigned nX,
                      float xMin,
                      float xMax,
                      unsigned nY,
                      float yMin,
                      float yMax,
                      unsigned nLayers);

    /// Destructor
    virtual ~THoughPlaneMulti2();

  public:// Selectors
    void dump(unsigned layerId) const;
    void dump(const std::string& message = std::string(""),
              const std::string& prefix = std::string("")) const;

  public:// Modifiers
    /// Clears all entries and regions.
    void clear(void);
    void clear(unsigned layerId);

    /// Clears entires only.
    void clearCells(void);

    /// vote
    void vote(float rx,
              float ry,
              int charge,
              const THoughTransformation& hough,
              unsigned layerId,
              int weight = 1);
    void vote(unsigned layerId, unsigned localId, int weight = 1);
    void vote(float rx,
              float ry,
              const THoughTransformation& hough,
              unsigned layerId,
              int weight = 1);
    void merge(void);

    /// registers a pattern..
//  void registerPattern(unsigned id);
    void registerPattern(unsigned layerId, unsigned id);

    /// allocate memory for patterns.
    void preparePatterns(unsigned layerId, unsigned nPatterns);

  protected:
    /// Add to a cell.
//    void add(unsigned cellId, int weight);

  private:
    unsigned _nLayers;
    THoughPlaneBoolean* _layers[N_LAYERS];
    bool _usage[N_LAYERS];
  };

  inline
  void
  THoughPlaneMulti2::clear(void)
  {
    for (unsigned i = 0; i < N_LAYERS; i++)
      if (_usage[i]) {
        _layers[i]->clear();
        _usage[i] = false;
      }
    THoughPlane::clear();
  }

  inline
  void
  THoughPlaneMulti2::clearCells(void)
  {
    for (unsigned i = 0; i < N_LAYERS; i++)
      if (_usage[i]) {
        _layers[i]->clear();
        _usage[i] = false;
      }
    THoughPlane::clearCells();
  }

  inline
  void
  THoughPlaneMulti2::clear(unsigned a)
  {
    _layers[a]->clear();
    _usage[a] = false;
  }

// inline
// void
// THoughPlaneMulti2::registerPattern(unsigned id) {
//     _layers[id]->registerPattern();
// }

// void
// THoughPlaneMulti2::registerPattern(void) {
//     return;
// }

// inline
// void
// THoughPlaneMulti2::add(unsigned a, int weight) {

// }

  inline
  void
  THoughPlaneMulti2::vote(float rx,
                          float ry,
                          int charge,
                          const THoughTransformation& hough,
                          unsigned layerId,
                          int weight)
  {
    _usage[layerId] = true;
    _layers[layerId]->vote(rx, ry, charge, hough, weight);
  }

  inline
  void
  THoughPlaneMulti2::vote(float rx,
                          float ry,
                          const THoughTransformation& hough,
                          unsigned layerId,
                          int weight)
  {
    _usage[layerId] = true;
    _layers[layerId]->vote(rx, ry, hough, weight);
  }

  inline
  void
  THoughPlaneMulti2::preparePatterns(unsigned layerId, unsigned nPatterns)
  {
    _layers[layerId]->preparePatterns(nPatterns);
  }

  inline
  void
  THoughPlaneMulti2::dump(unsigned a) const
  {
    _layers[a]->dump();
  }

  inline
  void
  THoughPlaneMulti2::vote(unsigned a, unsigned b, int c)
  {
    _usage[a] = true;
    _layers[a]->vote(b, c);
  }

  inline
  void
  THoughPlaneMulti2::dump(const std::string& a, const std::string& b) const
  {
    if (a == "merged") {
      THoughPlaneBase::dump(a, b);
    } else if (a == "region") {
      THoughPlaneBase::dump(a, b);
    } else {
      for (unsigned i = 0; i < _nLayers; i++) {
        std::cout << b << name() << " : layer " << i << std::endl;
        _layers[i]->dump();
      }
      std::cout << b << name() << " : merged plane " << std::endl;
      THoughPlaneBase::dump(a, b);
    }
  }

} // namespace Belle

#endif

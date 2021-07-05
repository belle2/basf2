/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//-----------------------------------------------------------------------------
// Description : A class to represent multi Hough parameter planes (version 2)
//-----------------------------------------------------------------------------

#ifndef TRGCDCHoughPlaneMulti2_FLAG_
#define TRGCDCHoughPlaneMulti2_FLAG_

#include "trg/cdc/HoughPlane.h"
#include "trg/cdc/HoughPlaneBoolean.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCHPlaneMulti2 TRGCDCHoughPlaneMulti2
#endif

/// number of layers
#define N_LAYERS 6

namespace Belle2 {

/// A class to represent a Hough parameter plane.
  class TRGCDCHoughPlaneMulti2 : public TRGCDCHoughPlane {

  public:

    /// Contructor.
    TRGCDCHoughPlaneMulti2(const std::string& name,
                           const TRGCDCHoughTransformation& transformation,
                           unsigned nX,
                           float xMin,
                           float xMax,
                           unsigned nY,
                           float yMin,
                           float yMax,
                           unsigned nLayers);

    /// Destructor
    virtual ~TRGCDCHoughPlaneMulti2();

  public:// Selectors

    /// returns \# of Hough Boolean layers.
    unsigned nLayers(void) const;

    /// returns pattern ID which activates specified cell.
    const std::vector<unsigned>& patternId(unsigned cellId) const;

    /// returns pattern ID in a layer which activates specified cell.
    const std::vector<unsigned>& patternId(unsigned layer,
                                           unsigned cellId) const;
    /// Dumps debug information.
    void dump(unsigned layerId) const;

    /// Dumps debug information.
    void dump(const std::string& message = std::string(""),
              const std::string& prefix = std::string("")) const override;

  public:// Modifiers

    /// Clears all entries and regions.
    void clear(void) override;

    /// Clears only specified layer ID.
    void clear(unsigned layerId);

    /// Clears entires only.
    void clearCells(void);

    /// Voting.
    using TRGCDCHoughPlaneBase::vote; // to be checked
    /// Voting.
    void vote(float rx,
              float ry,
              int charge,
              unsigned layerId,
              int weight = 1);

    /// Voting.
    // using TRGCDCHoughPlaneBase::vote; // to be checked
    /// Voting.
    void vote(unsigned layerId, unsigned localId, int weight = 1);

    /// Voting.
    // using TRGCDCHoughPlaneBase::vote; // to be checked
    /// Voting.
    void vote(float rx,
              float ry,
              unsigned layerId,
              int weight = 1);

    /// Sets entry.
    using TRGCDCHoughPlane::setEntry; // to be checked
    /// Sets entry.
    unsigned setEntry(unsigned serialId, unsigned layerId, unsigned n);

    /// Merge layers into one.
    void merge(void);

    /// Merge outer layers into one.
    void mergeOuters(void);

    /// allocate memory for patterns.
    void preparePatterns(unsigned layerId, unsigned nPatterns);

    /// registers a pattern..
    using TRGCDCHoughPlane::registerPattern; // to be checked
    /// registers a pattern..
    void registerPattern(unsigned layerId, unsigned id);

    /// finalize patterns.
    void finalizePatterns(void);

  private:

    /// \# of layers.
    unsigned _nLayers;

    /// Hough plane for each layer.
    TRGCDCHoughPlaneBoolean* _layers[N_LAYERS];

    /// Used or not.
    bool _usage[N_LAYERS];

    /// Pattern ID's for each cell
    std::vector<unsigned>* _reverse;
  };

//-----------------------------------------------------------------------------

  inline
  void
  TRGCDCHoughPlaneMulti2::clear(void)
  {
    for (unsigned i = 0; i < N_LAYERS; i++)
      if (_usage[i]) {
        _layers[i]->clear();
        _usage[i] = false;
      }
    TRGCDCHoughPlane::clear();
  }

  inline
  void
  TRGCDCHoughPlaneMulti2::clearCells(void)
  {
    for (unsigned i = 0; i < N_LAYERS; i++)
      if (_usage[i]) {
        _layers[i]->clear();
        _usage[i] = false;
      }
    TRGCDCHoughPlane::clearCells();
  }

  inline
  void
  TRGCDCHoughPlaneMulti2::clear(unsigned a)
  {
    _layers[a]->clear();
    _usage[a] = false;
  }

  inline
  void
  TRGCDCHoughPlaneMulti2::vote(float rx,
                               float ry,
                               int charge,
                               unsigned layerId,
                               int weight)
  {
    _usage[layerId] = true;
    _layers[layerId]->vote(rx, ry, charge, weight);
  }

  inline
  void
  TRGCDCHoughPlaneMulti2::vote(float rx,
                               float ry,
                               unsigned layerId,
                               int weight)
  {
    _usage[layerId] = true;
    _layers[layerId]->vote(rx, ry, weight);
  }

  inline
  void
  TRGCDCHoughPlaneMulti2::dump(unsigned a) const
  {
    _layers[a]->dump();
  }

  inline
  void
  TRGCDCHoughPlaneMulti2::vote(unsigned a, unsigned b, int c)
  {
    _usage[a] = true;
    _layers[a]->vote(b, c);
  }

  inline
  void
  TRGCDCHoughPlaneMulti2::dump(const std::string& a, const std::string& b) const
  {
    if (a == "merged") {
      TRGCDCHoughPlaneBase::dump(a, b);
    } else if (a == "region") {
      TRGCDCHoughPlaneBase::dump(a, b);
    } else {
      for (unsigned i = 0; i < _nLayers; i++) {
        std::cout << b << name() << " : layer " << i << std::endl;
        _layers[i]->dump();
      }
      std::cout << b << name() << " : merged plane " << std::endl;
      TRGCDCHoughPlaneBase::dump(a, b);
    }
  }

  inline
  void
  TRGCDCHoughPlaneMulti2::registerPattern(unsigned layerId, unsigned id)
  {
    _layers[layerId]->registerPattern(id);
  }

  inline
  const std::vector<unsigned>&
  TRGCDCHoughPlaneMulti2:: patternId(unsigned layer,
                                     unsigned cellId) const
  {
    return _layers[layer]->patternId(cellId);
  }

  inline
  void
  TRGCDCHoughPlaneMulti2::preparePatterns(unsigned layerId, unsigned nPatterns)
  {
    _layers[layerId]->preparePatterns(nPatterns);
  }

  inline
  unsigned
  TRGCDCHoughPlaneMulti2::nLayers(void) const
  {
    return _nLayers;
  }

  inline
  unsigned
  TRGCDCHoughPlaneMulti2::setEntry(unsigned serialId,
                                   unsigned layerId,
                                   unsigned n)
  {
    _usage[layerId] = true;
    _layers[layerId]->setEntry(serialId, n);
    return n;
  }

} // namespace Belle2

#endif

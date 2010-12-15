//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : HoughPlaneMulti2.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent multi Hough parameter planes (version 2)
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGCDCHoughPlaneMulti2_FLAG_
#define TRGCDCHoughPlaneMulti2_FLAG_

#include "trg/cdc/HoughPlane.h"
#include "trg/cdc/HoughPlaneBoolean.h"

#ifdef TRGCDC_SHORT_NAMES
#define CTHPlaneMulti2 TRGCDCHoughPlaneMulti2
#endif

#define N_LAYERS 6

namespace Belle2 {

/// A class to represent a Hough parameter plane.
class TRGCDCHoughPlaneMulti2 : public TRGCDCHoughPlane {

  public:
    /// Contructor.
    TRGCDCHoughPlaneMulti2(const std::string & name,
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
    void dump(unsigned layerId) const;
    void dump(const std::string & message = std::string(""),
	      const std::string & prefix = std::string("")) const;

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
	      const TRGCDCHoughTransformation & hough,
	      unsigned layerId,
	      int weight = 1);
    void vote(unsigned layerId, unsigned localId, int weight = 1);
    void vote(float rx,
	      float ry,
	      const TRGCDCHoughTransformation & hough,
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
    TRGCDCHoughPlaneBoolean * _layers[N_LAYERS];
    bool _usage[N_LAYERS];
};

inline
void
TRGCDCHoughPlaneMulti2::clear(void) {
    for (unsigned i = 0; i < N_LAYERS; i++)
	if (_usage[i]) {
	    _layers[i]->clear();
	    _usage[i] = false;
	}
    TRGCDCHoughPlane::clear();
}

inline
void
TRGCDCHoughPlaneMulti2::clearCells(void) {
    for (unsigned i = 0; i < N_LAYERS; i++)
	if (_usage[i]) {
	    _layers[i]->clear();
	    _usage[i] = false;
	}
    TRGCDCHoughPlane::clearCells();
}

inline
void
TRGCDCHoughPlaneMulti2::clear(unsigned a) {
    _layers[a]->clear();
    _usage[a] = false;
}

// inline
// void
// TRGCDCHoughPlaneMulti2::registerPattern(unsigned id) {
//     _layers[id]->registerPattern();
// }

// void
// TRGCDCHoughPlaneMulti2::registerPattern(void) {
//     return;
// }

// inline
// void
// TRGCDCHoughPlaneMulti2::add(unsigned a, int weight) {

// }

inline
void
TRGCDCHoughPlaneMulti2::vote(float rx,
				 float ry,
				 int charge,
				 const TRGCDCHoughTransformation & hough,
				 unsigned layerId,
				 int weight) {
    _usage[layerId] = true;
    _layers[layerId]->vote(rx, ry, charge, hough, weight);
}

inline
void
TRGCDCHoughPlaneMulti2::vote(float rx,
			float ry,
			const TRGCDCHoughTransformation & hough,
			unsigned layerId,
			int weight) {
    _usage[layerId] = true;
    _layers[layerId]->vote(rx, ry, hough, weight);
}

inline
void
TRGCDCHoughPlaneMulti2::preparePatterns(unsigned layerId, unsigned nPatterns) {
    _layers[layerId]->preparePatterns(nPatterns);
}

inline
void
TRGCDCHoughPlaneMulti2::dump(unsigned a) const {
    _layers[a]->dump();
}

inline
void
TRGCDCHoughPlaneMulti2::vote(unsigned a, unsigned b, int c) {
    _usage[a] = true;
    _layers[a]->vote(b, c);
}

inline
void
TRGCDCHoughPlaneMulti2::dump(const std::string & a, const std::string & b) const {
    if (a == "merged") {
	TRGCDCHoughPlaneBase::dump(a, b);
    }
    else if (a == "region") {
	TRGCDCHoughPlaneBase::dump(a, b);
    }
    else {
	for (unsigned i = 0; i < _nLayers; i++) {
	    std::cout << b << name() << " : layer " << i << std::endl;
	    _layers[i]->dump();
	}
	std::cout << b << name() << " : merged plane " << std::endl;
	TRGCDCHoughPlaneBase::dump(a, b);
    }
}

} // namespace Belle2

#endif

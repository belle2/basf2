//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : HoughPlaneMulti2.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent multi Hough parameter planes (version 2)
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRGCDCHoughPlaneMulti2_INLINE_DEFINE_HERE

#include "trg/cdc/TRGCDC.h"
#include "trg/cdc/HoughPlaneMulti2.h"

namespace Belle2 {

TRGCDCHoughPlaneMulti2::TRGCDCHoughPlaneMulti2(const std::string & name,
				     unsigned nX,
				     float xMin,
				     float xMax,
				     unsigned nY,
				     float yMin,
				     float yMax,
				     unsigned nLayers)
: TRGCDCHoughPlane(name, nX, xMin, xMax, nY, yMin, yMax),
  _nLayers(nLayers) {
//  bzero(_usage, N_LAYERS * sizeof(unsigned));
    for (unsigned i = 0; i < N_LAYERS; i++)
	_usage[i] = false;
    for (unsigned i = 0; i < _nLayers; i++)
	_layers[i] = new TRGCDCHoughPlaneBoolean(name + ":layer" + TRGCDC::itostring(i),
					    nX, xMin, xMax,
					    nY, yMin, yMax);
    if (nLayers > N_LAYERS)
	std::cout
	    << "Too many layers requested(" << _nLayers << ") : "
	    << "max #layers = "	<< N_LAYERS << std::endl;
}

TRGCDCHoughPlaneMulti2::~TRGCDCHoughPlaneMulti2() {
    for (unsigned i = 0; i < _nLayers; i++)
	delete _layers[i];
}

void
TRGCDCHoughPlaneMulti2::merge(void) {
    const unsigned n = _layers[0]->_n;
    for (unsigned i = 0; i < _nLayers; i++) {

	//...Use _usage here...
	if (! _usage[i]) continue;
	
	for (unsigned j = 0; j < n; j++) {
	    if (! _layers[i]->_cell[j]) continue;
	    for (unsigned k = 0; k < 32; k++) {
		if ((_layers[i]->_cell[j] >> k) & 1) {
		    add(j * 32 + k, 1);
		}
	    }
	}
    }
}

void
TRGCDCHoughPlaneMulti2::registerPattern(unsigned layerId, unsigned id) {
    _layers[layerId]->registerPattern(id);
}

} // namespace Belle2

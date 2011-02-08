//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : HoughPlaneMulti.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent multi Hough parameter planes
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRGCDCHoughPlaneMulti_INLINE_DEFINE_HERE

#include "trg/cdc/HoughPlaneMulti.h"

namespace Belle2 {

TRGCDCHoughPlaneMulti::TRGCDCHoughPlaneMulti(const std::string & name,
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
//  for (unsigned i = 0; i < nLayers; i++)
//         _layers.append(new TRGCDCHoughPlane(name, nX, xMin, xMax, nY, yMin, yMax));
    for (unsigned i = 0; i < _nLayers; i++)
        _layers[i] = new TRGCDCHoughPlane(name, nX, xMin, xMax, nY, yMin, yMax);
    if (nLayers > N_LAYERS)
        std::cout << "Too many layers requested("
                                                << _nLayers << ") : "
                                                << "max #layers = "
                                                << N_LAYERS << std::endl;
}

TRGCDCHoughPlaneMulti::~TRGCDCHoughPlaneMulti() {
//  HepAListDeleteAll(_layers);
    for (unsigned i = 0; i < _nLayers; i++)
        delete _layers[i];
}

void
TRGCDCHoughPlaneMulti::vote(float rx,
                       float ry,
                       float targetCharge,
                       const TRGCDCHoughTransformation & hough,
                       unsigned weight,
                       unsigned layerId) {

#ifdef TRASAN_DEBUG
    if (layerId >= N_LAYERS)
        std::cout << "TRGCDCHoughPlaneMulti::vote !!! (" << name()
               << ") given layerId(" << layerId << ") is too big : max="
               << _nLayers - 1 << std::endl;
#endif

//     const unsigned a = layerId / 32;
//     _usage[a] |= (1 << (layerId % 32));
    _usage[layerId] = true;
    _layers[layerId]->vote(rx, ry, (int) targetCharge, hough, weight);
}

void
TRGCDCHoughPlaneMulti::merge(void) {
    const unsigned nCells = nX() * nY();

//     for (unsigned i = 0; i < nCells; i++) {
//          unsigned n = 0;
//          for (unsigned j = 0; j < _layers.length(); j++)
// //            if (_layers[j]->entry(i))
//              if (_layers[j]->_cell[i])
//                  ++n;
//          setEntry(i, n);
//      }

    bzero(_cell, nCells * sizeof(unsigned));
//  for (unsigned j = 0; j < _layers.length(); j++) {
    for (unsigned j = 0; j < _nLayers; j++) {
//         const unsigned a = j / 32;
//         if (! (_usage[a] & (1 << (j % 32)))) continue;
        if (! _usage[j]) continue;
         for (unsigned i = 0; i < nCells; i++) {
             if (_layers[j]->_cell[i])
                 ++_cell[i];
         }
    }
}

void
TRGCDCHoughPlaneMulti::vote(float phi, unsigned layerId, int weight) {
    _layers[layerId]->vote(phi, weight);
}

} // namespace Belle2

//-----------------------------------------------------------------------------
// $Id: THoughPlaneMulti.cc 10305 2007-12-05 05:19:24Z yiwasaki $
//-----------------------------------------------------------------------------
// Filename : THoughPlaneMulti.cc
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent multi Hough parameter planes
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.1  2005/04/18 23:41:46  yiwasaki
// Trasan 3.17 : Only Hough finder is modified
//
//-----------------------------------------------------------------------------


#include <strings.h>
#include <iostream>

#include "tracking/modules/trasan/THoughPlaneMulti.h"


namespace Belle {

  THoughPlaneMulti::THoughPlaneMulti(const std::string& name,
                                     unsigned nX,
                                     float xMin,
                                     float xMax,
                                     unsigned nY,
                                     float yMin,
                                     float yMax,
                                     unsigned nLayers)
    : THoughPlane(name, nX, xMin, xMax, nY, yMin, yMax),
      _nLayers(nLayers)
  {
//  bzero(_usage, N_LAYERS * sizeof(unsigned));
    for (unsigned i = 0; i < N_LAYERS; i++)
      _usage[i] = false;
//  for (unsigned i = 0; i < nLayers; i++)
//  _layers.append(new THoughPlane(name, nX, xMin, xMax, nY, yMin, yMax));
    for (unsigned i = 0; i < _nLayers; i++)
      _layers[i] = new THoughPlane(name, nX, xMin, xMax, nY, yMin, yMax);
    if (nLayers > N_LAYERS)
      std::cout << "Too many layers requested("
                << _nLayers << ") : "
                << "max #layers = "
                << N_LAYERS << std::endl;
  }

  THoughPlaneMulti::~THoughPlaneMulti()
  {
//  HepAListDeleteAll(_layers);
    for (unsigned i = 0; i < _nLayers; i++)
      delete _layers[i];
  }

  void
  THoughPlaneMulti::vote(float rx,
                         float ry,
                         float targetCharge,
                         const THoughTransformation& hough,
                         unsigned weight,
                         unsigned layerId)
  {

#ifdef TRASAN_DEBUG
    if (layerId >= N_LAYERS)
      std::cout << "THoughPlaneMulti::vote !!! (" << name()
                << ") given layerId(" << layerId << ") is too big : max="
                << _nLayers - 1 << std::endl;
#endif

//     const unsigned a = layerId / 32;
//     _usage[a] |= (1 << (layerId % 32));
    _usage[layerId] = true;
    _layers[layerId]->vote(rx, ry, (int) targetCharge, hough, weight);
  }

  void
  THoughPlaneMulti::merge(void)
  {
    const unsigned nCells = nX() * nY();

//     for (unsigned i = 0; i < nCells; i++) {
//    unsigned n = 0;
//    for (unsigned j = 0; j < _layers.length(); j++)
// //     if (_layers[j]->entry(i))
//        if (_layers[j]->_cell[i])
//      ++n;
//    setEntry(i, n);
//      }

    bzero(_cell, nCells * sizeof(unsigned));
//  for (unsigned j = 0; j < _layers.length(); j++) {
    for (unsigned j = 0; j < _nLayers; j++) {
//  const unsigned a = j / 32;
//  if (! (_usage[a] & (1 << (j % 32)))) continue;
      if (! _usage[j]) continue;
      for (unsigned i = 0; i < nCells; i++) {
        if (_layers[j]->_cell[i])
          ++_cell[i];
      }
    }
  }

  void
  THoughPlaneMulti::vote(float phi, unsigned layerId, int weight)
  {
    _layers[layerId]->vote(phi, weight);
  }

} // namespace Belle

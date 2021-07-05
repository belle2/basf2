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
// Filename : HoughPlaneMulti.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent multi Hough parameter planes
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRGCDC_SHORT_NAMES

#include "trg/cdc/HoughPlaneMulti.h"

namespace Belle2 {

  TRGCDCHoughPlaneMulti::TRGCDCHoughPlaneMulti(const std::string& name,
                                               const TCHTransformation& trans,
                                               unsigned nX,
                                               float xMin,
                                               float xMax,
                                               unsigned nY,
                                               float yMin,
                                               float yMax,
                                               unsigned nLayers)
    : TRGCDCHoughPlane(name, trans, nX, xMin, xMax, nY, yMin, yMax),
      _nLayers(nLayers)
  {

    for (unsigned i = 0; i < N_LAYERS; i++)
      _usage[i] = false;
    for (unsigned i = 0; i < _nLayers; i++)
      _layers[i] = new TRGCDCHoughPlane(name,
                                        trans,
                                        nX,
                                        xMin,
                                        xMax,
                                        nY,
                                        yMin,
                                        yMax);
    if (nLayers > N_LAYERS)
      std::cout << "Too many layers requested("
                << _nLayers << ") : "
                << "max #layers = "
                << N_LAYERS << std::endl;
  }

  TRGCDCHoughPlaneMulti::~TRGCDCHoughPlaneMulti()
  {
    for (unsigned i = 0; i < _nLayers; i++)
      delete _layers[i];
  }

  void
  TRGCDCHoughPlaneMulti::vote(float rx,
                              float ry,
                              float targetCharge,
                              const TRGCDCHoughTransformation&,
                              unsigned weight,
                              unsigned layerId)
  {

#ifdef TRASAN_DEBUG
    if (layerId >= N_LAYERS)
      std::cout << "TRGCDCHoughPlaneMulti::vote !!! (" << name()
                << ") given layerId(" << layerId << ") is too big : max="
                << _nLayers - 1 << std::endl;
#endif

//     const unsigned a = layerId / 32;
//     _usage[a] |= (1 << (layerId % 32));
    // cppcheck-suppress arrayIndexOutOfBoundsCond
    _usage[layerId] = true;
    // cppcheck-suppress arrayIndexOutOfBoundsCond
    _layers[layerId]->vote(rx, ry, (int) targetCharge, weight);
  }

  void
  TRGCDCHoughPlaneMulti::merge(void)
  {
    const unsigned nCells = nX() * nY();

//     for (unsigned i = 0; i < nCells; i++) {
//          unsigned n = 0;
//          for (unsigned j = 0; j < _layers.length(); j++)
// //            if (_layers[j]->entry(i))
//              if (_layers[j]->_cell[i])
//                  ++n;
//          setEntry(i, n);
//      }

    memset(_cell, 0, nCells * sizeof(unsigned));
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
  TRGCDCHoughPlaneMulti::vote(float phi, unsigned layerId, int weight)
  {
    _layers[layerId]->vote(phi, weight);
  }

} // namespace Belle2

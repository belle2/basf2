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
// Filename : HoughPlaneMulti2.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent multi Hough parameter planes (version 2)
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGCDC_SHORT_NAMES

#include "trg/trg/Utilities.h"
#include "trg/cdc/HoughPlaneMulti2.h"

namespace Belle2 {

  TRGCDCHoughPlaneMulti2::TRGCDCHoughPlaneMulti2(const std::string& name,
                                                 const TCHTransformation& trans,
                                                 unsigned nX,
                                                 float xMin,
                                                 float xMax,
                                                 unsigned nY,
                                                 float yMin,
                                                 float yMax,
                                                 unsigned nLayers)
    : TRGCDCHoughPlane(name, trans, nX, xMin, xMax, nY, yMin, yMax),
      _nLayers(nLayers),
      _reverse{} // 2019/07/31 by ytlai
  {

    for (unsigned i = 0; i < N_LAYERS; i++)
      _usage[i] = false;
    for (unsigned i = 0; i < _nLayers; i++)
      _layers[i] = new TCHPlaneBoolean(name +
                                       ":layer" +
                                       TRGUtil::itostring(i),
                                       trans,
                                       nX,
                                       xMin,
                                       xMax,
                                       nY,
                                       yMin,
                                       yMax);
    if (nLayers > N_LAYERS)
      std::cout
          << "Too many layers requested(" << _nLayers << ") : "
          << "max #layers = "        << N_LAYERS << std::endl;
  }

  TRGCDCHoughPlaneMulti2::~TRGCDCHoughPlaneMulti2()
  {
    for (unsigned i = 0; i < _nLayers; i++)
      delete _layers[i];
  }

  void
  TRGCDCHoughPlaneMulti2::merge(void)
  {
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
  TRGCDCHoughPlaneMulti2::mergeOuters(void)
  {
    const unsigned n = _layers[0]->_n;
    for (unsigned i = 1; i < _nLayers; i++) {

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

} // namespace Belle2

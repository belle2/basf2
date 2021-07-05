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
// Filename : Layer.cc
// Section  : Tracking CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a wire layer.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGCDC_SHORT_NAMES

#include <iostream>
#include "trg/cdc/Cell.h"
#include "trg/cdc/Layer.h"

using namespace std;

namespace Belle2 {

  /// TRGCDCUndefinedLayer declaration
  const TRGCDCLayer* TRGCDCUndefinedLayer = new TRGCDCLayer(9999,
                                                            9999,
                                                            9999,
                                                            9999,
                                                            9999,
                                                            0,
                                                            0,
                                                            9999,
                                                            9999,
                                                            0,
                                                            0);

  TRGCDCLayer::TRGCDCLayer(unsigned id,
                           unsigned superLayerId,
                           unsigned localLayerId,
                           unsigned axialStereoLayerId,
                           unsigned axialStereoSuperLayerId,
                           float offset,
                           int nShifts,
                           float cellSize,
                           unsigned nCells,
                           float innerRadius,
                           float outerRadius)
    :  _name("unknown"),
       _id(id),
       _superLayerId(superLayerId),
       _localLayerId(localLayerId),
       _axialStereoLayerId(axialStereoLayerId),
       _axialStereoSuperLayerId(axialStereoSuperLayerId),
       _offset(offset),
       _nShifts(nShifts),
       _cellSize(cellSize),
       _nCells(nCells),
       _innerRadius(innerRadius),
       _outerRadius(outerRadius)
  {
  }

  TRGCDCLayer::TRGCDCLayer(unsigned id,
                           const TRGCDCCell& w)
    :  _id(id),
       _superLayerId(w.superLayerId()),
       _localLayerId(0),
       _axialStereoLayerId(0),
       _axialStereoSuperLayerId(0),
       _offset(w.layer().offset()),
       _nShifts(w.layer().nShifts()),
       _cellSize(w.cellSize()),
       _nCells(w.layer().nCells()),
       _innerRadius(), // 2019/07/31 by ytlai
       _outerRadius()
  {
  }

  TRGCDCLayer::~TRGCDCLayer()
  {
  }

  void
  TRGCDCLayer::dump(const string&, const string& pre) const
  {
    cout << pre;
    cout << "layer " << _id;
    cout << ", " << stereoType();
    cout << ", super layer " << _superLayerId;
    cout << ", local layer " << _localLayerId;
    if (axial()) cout << ", axial ";
    else         cout << ", stereo ";
    cout << _axialStereoLayerId;
    if (axial()) cout << ", axial super ";
    else         cout << ", stereo super ";
    cout << _axialStereoSuperLayerId;
    cout << ", " << _nCells << " wires";
    cout << endl;
    //    for (int i=0;i<_nCells;++i) wire(i)->dump(pre);
  }

  const TRGCDCCell&
  TRGCDCLayer::cell(int id) const
  {
    if (_nCells == 0) {
      cout << "TRGCDCLayer !!! This has no cell member : " << name() << endl;
      return * dynamic_cast<const TRGCDCCell*>(TRGCDCUndefinedLayer);
    }

    if (id < 0)
      while (id < 0)
        id += _nCells;

    if (id >= (int) _nCells)
      id %= (int) _nCells;

    return * (* this)[id];
  }

} // namespace Belle2


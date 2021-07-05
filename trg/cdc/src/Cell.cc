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
// Filename : Cell.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a cell (a wire or a track segment) in CDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGCDC_SHORT_NAMES

#include "trg/trg/Utilities.h"
#include "trg/cdc/Cell.h"
#include "trg/cdc/Segment.h"

#define P3D HepGeom::Point3D<double>

using namespace std;

namespace Belle2 {

  TRGCDCCell::TRGCDCCell(unsigned id,
                         unsigned localId,
                         const TCLayer& l,
                         const P3D& fp,
                         const P3D& bp)
    : _id(id),
      _localId(localId),
      _layer(l),
      _xyPosition(0.5 * (fp + bp)),
      _forwardPosition(fp),
      _backwardPosition(bp),
      _direction((fp - bp).unit()),
      _state(0),
      _hit(0)
  {
  }

  TRGCDCCell::~TRGCDCCell()
  {
  }

  void
  TRGCDCCell::dump(const string& msg, const string& pre) const
  {
    cout << pre;
    cout << "w " << _id;
    cout << ",local " << _localId;
    cout << ",layer " << layerId();
    cout << ",super layer " << superLayerId();
    cout << ",local layer " << localLayerId();
    cout << endl;
    if (msg.find("neighbor") != string::npos) {
//        for (unsigned i = 0; i < 7; i++)
//             if (neighbor(i))
//                 neighbor(i)->dump("", pre + TRGUtil::itostring(i) + "   ");
    }
  }

  int
  TRGCDCCell::localIdDifference(const TRGCDCCell& a) const
  {

    if (superLayerId() != a.superLayerId()) {
      cout << "TRGCDCCell::localIdDifference !!!";
      cout << "super layer assumption violation" << endl;
    }

    int diff = int(a.localId()) - int(localId());
    unsigned nCells = layer().nCells();
    if (diff > 0) {
      int difR = nCells - diff;
      if (diff < difR) return diff;
      else return - difR;
    } else {
      int difR = nCells + diff;
      if (- diff < difR) return diff;
      else return difR;
    }
  }

} // namespace Belle2


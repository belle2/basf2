/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/dbobjects/TOPGeoPMTArray.h>
#include <math.h>
#include <iostream>
#include <TRandom.h>

using namespace std;

namespace Belle2 {

  unsigned TOPGeoPMTArray::getPmtID(unsigned row, unsigned col) const
  {
    if (row == 0 or row > m_numRows) return 0;
    if (col == 0 or col > m_numColumns) return 0;
    return (row - 1) * m_numColumns + col;
  }

  unsigned TOPGeoPMTArray::getRow(unsigned pmtID) const
  {
    if (pmtID == 0 or pmtID > getSize()) return 0;
    return (pmtID - 1) / m_numColumns + 1;
  }

  unsigned TOPGeoPMTArray::getColumn(unsigned pmtID) const
  {
    if (pmtID == 0 or pmtID > getSize()) return 0;
    return (pmtID - 1) % m_numColumns + 1;
  }

  int TOPGeoPMTArray::getPixelID(double x, double y, unsigned pmtID) const
  {
    if (pmtID == 0 or pmtID > getSize()) return 0;
    pmtID--;

    unsigned col = m_pmt.getPixelColumn(x);
    if (col == 0) return 0;
    col--;

    unsigned row = m_pmt.getPixelRow(y);
    if (row == 0) return 0;
    row--;

    col += (pmtID % m_numColumns) * m_pmt.getNumColumns();
    row += (pmtID / m_numColumns) * m_pmt.getNumRows();
    return row * m_numColumns * m_pmt.getNumColumns() + col + 1;
  }


  void TOPGeoPMTArray::generateDecoupledPMTs(double fraction)
  {
    if (fraction <= 0) return;

    for (unsigned i = 0; i < getSize(); i++) {
      if (gRandom->Rndm() < fraction) setDecoupledPMT(i + 1);
    }

  }

  bool TOPGeoPMTArray::isPMTDecoupled(unsigned pmtID) const
  {
    for (const auto& pmt : m_decoupledPMTs) {
      if (pmtID == pmt) return true;
    }
    return false;
  }


  bool TOPGeoPMTArray::isConsistent() const
  {
    if (m_numRows == 0) return false;
    if (m_numColumns == 0) return false;
    if (m_dx <= 0) return false;
    if (m_dy <= 0) return false;
    if (m_material.empty()) return false;
    if (!m_pmt.isConsistent()) return false;
    return true;
  }

  void TOPGeoPMTArray::print(const std::string& title) const
  {
    TOPGeoBase::print(title);
    cout << " size: " << getNumColumns() << " X " << getNumRows();
    cout << ", dimensions: " << getSizeX() << " X " << getSizeY() << " X " << getSizeZ();
    cout << " " << s_unitName << endl;
    cout << " cell: " << getDx() << " X " << getDy() << " " << s_unitName << endl;
    cout << " material: " << getMaterial() << endl;
    cout << " air gap (decoupled PMT's): " << getAirGap() << " " << s_unitName << endl;
    cout << " optically decoupled PMT's:";
    if (m_decoupledPMTs.empty()) {
      cout << " None";
    } else {
      for (const auto& pmt : m_decoupledPMTs) cout << " " << pmt;
    }
    cout << endl;
    cout << endl;
    m_pmt.print();
  }

} // end Belle2 namespace

/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/dbobjects/TOPGeoPMT.h>
#include <math.h>
#include <iostream>

using namespace std;

namespace Belle2 {

  unsigned TOPGeoPMT::getPixelColumn(double x) const
  {
    x *= s_unit;
    if (fabs(x) >= m_sensSizeX / 2) return 0;
    return m_numColumns - int((x + m_sensSizeX / 2) / m_sensSizeX * m_numColumns);
  }


  unsigned TOPGeoPMT::getPixelRow(double y) const
  {
    y *= s_unit;
    if (fabs(y) >= m_sensSizeY / 2) return 0;
    return int((y + m_sensSizeY / 2) / m_sensSizeY * m_numRows) + 1;
  }

  unsigned TOPGeoPMT::getPixelID(double x, double y) const
  {
    auto col = getPixelColumn(x);
    if (col == 0) return 0;
    auto row = getPixelRow(y);
    if (row == 0) return 0;

    return (row - 1) * m_numColumns + col;
  }


  double TOPGeoPMT::getX(unsigned col) const
  {
    return (int)(m_numColumns + 1 - 2 * col) / 2.0 * getDx();
  }

  double TOPGeoPMT::getY(unsigned row) const
  {
    return (int)(2 * row - m_numRows - 1) / 2.0 * getDy();
  }


  bool TOPGeoPMT::isConsistent() const
  {
    if (m_sizeX <= 0) return false;
    if (m_sizeY <= 0) return false;
    if (m_sizeZ <= 0) return false;
    if (m_wallThickness <= 0) return false;

    if (m_sensSizeX <= 0) return false;
    if (m_sensSizeY <= 0) return false;
    if (m_sensThickness <= 0) return false;
    if (m_numColumns == 0) return false;
    if (m_numRows == 0) return false;
    if (m_sensMaterial.empty()) return false;

    if (m_winThickness <= 0) return false;
    if (m_winMaterial.empty()) return false;

    if (m_botThickness <= 0) return false;
    if (m_botMaterial.empty()) return false;

    if (m_reflEdgeWidth <= 0) return false;
    if (m_reflEdgeThickness <= 0) return false;

    return true;
  }


  void TOPGeoPMT::print(const std::string& title) const
  {
    TOPGeoBase::print(title);

    cout << " outer dimensions: " << getSizeX() << " X " << getSizeY()
         << " X " << getSizeZ() << " " << s_unitName << endl;
    cout << " wall thickness:   " << getWallThickness() << " " << s_unitName << endl;
    cout << " window thickness: " << getWinThickness() << " " << s_unitName << endl;
    cout << " bottom thickness: " << getBotThickness() << " " << s_unitName << endl;
    cout << " sensitive area:   " <<  getSensSizeX() << " X " << getSensSizeY()
         << " " << s_unitName;
    cout << ", thickness: " << getSensThickness() << " " << s_unitName << endl;
    cout << " number of pixels: " << getNumColumns() << " X " << getNumRows() << endl;
    cout << " casing material:    " << getWallMaterial() << endl;
    cout << " inner material:     " << getFillMaterial() << endl;
    cout << " window material:    " << getWinMaterial() << endl;
    cout << " bottom material:    " << getBotMaterial() << endl;
    cout << " sensitive material: " << getSensMaterial() << endl;
    cout << " reflective edge: " << endl;
    cout << "  - width: " << getReflEdgeWidth() << " " << s_unitName;
    cout << ", thickness: " << getReflEdgeThickness() << " " << s_unitName << endl;

    printSurface(m_reflEdgeSurface);
  }

} // end Belle2 namespace

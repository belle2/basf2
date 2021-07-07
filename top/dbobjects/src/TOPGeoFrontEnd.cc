/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/dbobjects/TOPGeoFrontEnd.h>
#include <iostream>

using namespace std;

namespace Belle2 {

  bool TOPGeoFrontEnd::isConsistent() const
  {
    if (m_FBWidth <= 0) return false;
    if (m_FBHeight <= 0) return false;
    if (m_FBThickness <= 0) return false;
    if (m_FBGap < 0) return false;
    if (m_FBMaterial.empty()) return false;

    if (m_HVWidth <= 0) return false;
    if (m_HVLength <= 0) return false;
    if (m_HVThickness <= 0) return false;
    if (m_HVGap < 0) return false;
    if (m_HVMaterial.empty()) return false;

    if (m_BSWidth <= 0) return false;
    if (m_BSHeight <= 0) return false;
    if (m_BSLength <= 0) return false;
    if (m_BSGap < 0) return false;
    if (m_BSMaterial.empty()) return false;
    if (m_spacerWidth <= 0) return false;
    if (m_spacerMaterial.empty()) return false;

    return true;
  }


  void TOPGeoFrontEnd::print(const std::string& title) const
  {
    TOPGeoBase::printUnderlined(title);

    cout << " Front board: " << getFrontBoardWidth() << " X " <<
         getFrontBoardHeight() << " X " << getFrontBoardThickness() << " " << s_unitName;
    cout << ", gap = " << getFrontBoardGap() << " " << s_unitName;
    cout << ", y = " << getFrontBoardY() << " " << s_unitName;
    cout << ", material = " << getFrontBoardMaterial() << endl;

    cout << " HV board: " << getHVBoardWidth() << " X " <<
         getHVBoardLength() << " X " << getHVBoardThickness() << " " << s_unitName;
    cout << ", gap = " << getHVBoardGap() << " " << s_unitName;
    cout << ", y = " << getHVBoardY() << " " << s_unitName;
    cout << ", material = " << getHVBoardMaterial() << endl;

    cout << " Board stack: " << getBoardStackWidth() << " X " <<
         getBoardStackHeight() << " X " << getBoardStackLength() << " " << s_unitName;
    cout << ", gap = " << getBoardStackGap() << " " << s_unitName;
    cout << ", y = " << getBoardStackY() << " " << s_unitName;
    cout << ", material = " << getBoardStackMaterial() << endl;

    cout << "              spacers: width = " << getSpacerWidth() << " " << s_unitName;
    cout << ", material = " << getSpacerMaterial() << endl;

  }

} // end Belle2 namespace

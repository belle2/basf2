/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/dbobjects/TOPGeoBoardStack.h>
#include <framework/gearbox/Unit.h>
#include <math.h>
#include <iostream>

using namespace std;
using namespace Belle2;

bool TOPGeoBoardStack::isConsistent() const
{
  if (m_FBWidth <= 0) return false;
  if (m_FBHeight <= 0) return false;
  if (m_FBThickness <= 0) return false;
  if (m_FBGap < 0) return false;
  if (m_FBMaterial.empty()) return false;

  if (m_BSWidth <= 0) return false;
  if (m_BSHeight <= 0) return false;
  if (m_BSLength <= 0) return false;
  if (m_BSGap < 0) return false;
  if (m_BSMaterial.empty()) return false;
  if (m_spacerWidth <= 0) return false;
  if (m_spacerMaterial.empty()) return false;

  return true;
}


void TOPGeoBoardStack::print(const std::string& title) const
{
  TOPGeoBase::print(title);

  cout << " Front board: " << getFrontBoardWidth() << " X " <<
       getFrontBoardHeight() << " X " << getFrontBoardThickness() << " " << s_unitName;
  cout << ", gap = " << getFrontBoardGap() << " " << s_unitName;
  cout << ", y = " << getFrontBoardY() << " " << s_unitName;
  cout << ", material = " << getFrontBoardMaterial() << endl;

  cout << " Board stack: " << getBoardStackWidth() << " X " <<
       getBoardStackHeight() << " X " << getBoardStackLength() << " " << s_unitName;
  cout << ", gap = " << getBoardStackGap() << " " << s_unitName;
  cout << ", y = " << getBoardStackY() << " " << s_unitName;
  cout << ", material = " << getBoardStackMaterial() << endl;

  cout << "              spacer: width = " << getSpacerWidth() << " " << s_unitName;
  cout << ", material = " << getSpacerMaterial() << endl;

}


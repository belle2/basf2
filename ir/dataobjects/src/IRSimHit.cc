/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <ir/dataobjects/IRSimHit.h>

using namespace std;
using namespace Belle2;

IRSimHit::~IRSimHit()
{
}

/** The method to set.*/
void IRSimHit::setposIn(ROOT::Math::XYZVector posIn)
{
  m_posIn = posIn;
}
void IRSimHit::setmomIn(ROOT::Math::XYZVector momIn)
{
  m_momIn = momIn;
}
void IRSimHit::setposOut(ROOT::Math::XYZVector posOut)
{
  m_posOut = posOut;
}
void IRSimHit::setmomOut(ROOT::Math::XYZVector momOut)
{
  m_momOut = momOut;
}
void IRSimHit::setPDGcode(int PDGcode)
{
  m_PDGcode = PDGcode;
}
void IRSimHit::setdepEnergy(float depEnergy)
{
  m_depEnergy = depEnergy;
}
void IRSimHit::setVolname(const std::string& Volname)
{
  m_Volname = Volname;
}

/** The method to get.*/
ROOT::Math::XYZVector IRSimHit::getposIn() { return m_posIn; }
ROOT::Math::XYZVector IRSimHit::getmomIn() { return m_momIn; }
ROOT::Math::XYZVector IRSimHit::getposOut() { return m_posOut; }
ROOT::Math::XYZVector IRSimHit::getmomOut() { return m_momOut; }
int IRSimHit::getPDGcode() { return m_PDGcode; }
float IRSimHit::getdepEnergy() { return m_depEnergy; }
std::string IRSimHit::getVolname() { return m_Volname; }


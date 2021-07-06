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
void IRSimHit::setposIn(TVector3 posIn)
{
  m_posIn = posIn;
}
void IRSimHit::setmomIn(TVector3 momIn)
{
  m_momIn = momIn;
}
void IRSimHit::setposOut(TVector3 posOut)
{
  m_posOut = posOut;
}
void IRSimHit::setmomOut(TVector3 momOut)
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
TVector3 IRSimHit::getposIn() { return m_posIn; }
TVector3 IRSimHit::getmomIn() { return m_momIn; }
TVector3 IRSimHit::getposOut() { return m_posOut; }
TVector3 IRSimHit::getmomOut() { return m_momOut; }
int IRSimHit::getPDGcode() { return m_PDGcode; }
float IRSimHit::getdepEnergy() { return m_depEnergy; }
std::string IRSimHit::getVolname() { return m_Volname; }


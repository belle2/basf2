/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao, Martin Heck                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
void IRSimHit::setVolname(std::string Volname)
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


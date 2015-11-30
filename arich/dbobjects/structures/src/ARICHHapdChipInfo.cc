/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Manca Mrvar                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <arich/dbobjects/structures/ARICHHapdChipInfo.h>

using namespace Belle2;
using namespace std;

ClassImp(ARICHHapdChipInfo);


TGraph* ARICHHapdChipInfo::getBombardmentCurrent(unsigned int i) const
{
  if (i < m_bombardmentCurrent.size()) return m_bombardmentCurrent[i];
  else return NULL;
}


void ARICHHapdChipInfo::setBombardmentCurrent(std::vector<TGraph*> bcurrent)
{
  for (unsigned int i = 0; i < bcurrent.size(); i++) m_bombardmentCurrent.push_back(bcurrent[i]);
}


TGraph* ARICHHapdChipInfo::getAvalancheCurrent(unsigned int i) const
{
  if (i < m_avalancheCurrent.size()) return m_avalancheCurrent[i];
  else return NULL;
}


void ARICHHapdChipInfo::setAvalancheCurrent(std::vector<TGraph*> acurrent)
{
  for (unsigned int i = 0; i < acurrent.size(); i++) m_avalancheCurrent.push_back(acurrent[i]);
}


int ARICHHapdChipInfo::getDeadChannel(unsigned int i) const
{
  if (i < m_deadChannel.size()) return m_deadChannel[i];
  else return -1;
}


int ARICHHapdChipInfo::getBadChannel(unsigned int i) const
{
  if (i < m_badChannel.size()) return m_badChannel[i];
  else return -1;
}

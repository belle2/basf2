/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <arich/dbobjects/ARICHHapdChipInfo.h>

using namespace Belle2;
using namespace std;

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


int ARICHHapdChipInfo::getCutChannel(unsigned int i) const
{
  if (i < m_cutChannel.size()) return m_cutChannel[i];
  else return -1;
}


int ARICHHapdChipInfo::getBadChannel(unsigned int i) const
{
  if (i < m_badChannel.size()) return m_badChannel[i];
  else return -1;
}

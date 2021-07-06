/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <arich/dbobjects/ARICHHapdQA.h>

using namespace Belle2;
using namespace std;

TGraph* ARICHHapdQA::getHapdLeakCurrent(unsigned int i) const
{
  if (i < m_leakCurrent.size()) return m_leakCurrent[i];
  else return NULL;
}


void ARICHHapdQA::setHapdLeakCurrent(std::vector<TGraph*> leakCurrent)
{
  for (unsigned int i = 0; i < leakCurrent.size(); i++) m_leakCurrent.push_back(leakCurrent[i]);
}


TGraph* ARICHHapdQA::getHapdNoise(unsigned int i) const
{
  if (i < m_noise.size()) return m_noise[i];
  else return NULL;
}


TH1S* ARICHHapdQA::getHapdHitCount(unsigned int i) const
{
  if (i < m_hitCount.size()) return m_hitCount[i];
  else return NULL;
}


void ARICHHapdQA::setHapdHitCount(std::vector<TH1S*> hitCount)
{
  for (unsigned int i = 0; i < hitCount.size(); i++)  m_hitCount.push_back(hitCount[i]);
}



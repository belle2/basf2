/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Manca Mrvar                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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



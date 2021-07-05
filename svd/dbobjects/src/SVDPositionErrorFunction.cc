/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include<svd/dbobjects/SVDPositionErrorFunction.h>

using namespace Belle2;

std::vector <SVDPositionErrorFunction::posErrFunction> SVDPositionErrorFunction::m_implementations;


SVDPositionErrorFunction::SVDPositionErrorFunction(const SVDPositionErrorFunction& a) : TObject(a)
{
  for (int i = 0; i < maxClusterSize; i++) {
    m_a1[i] = a.m_a1[i];
    m_a2[i] = a.m_a2[i];
    m_b1[i] = a.m_b1[i];
    m_c1[i] = a.m_c1[i];
  }

  m_current = a.m_current;

}

SVDPositionErrorFunction& SVDPositionErrorFunction::operator=(const SVDPositionErrorFunction& a)
{
  if (this == &a)
    return *this;

  for (int i = 0; i < maxClusterSize; i++) {
    m_a1[i] = a.m_a1[i];
    m_a2[i] = a.m_a2[i];
    m_b1[i] = a.m_b1[i];
    m_c1[i] = a.m_c1[i];
  }

  m_current = a.m_current;

  return *this;
}

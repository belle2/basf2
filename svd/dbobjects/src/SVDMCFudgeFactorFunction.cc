/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include<svd/dbobjects/SVDMCFudgeFactorFunction.h>

using namespace Belle2;

std::vector <SVDMCFudgeFactorFunction::fudgeFactorFunction> SVDMCFudgeFactorFunction::m_implementations;


SVDMCFudgeFactorFunction::SVDMCFudgeFactorFunction(const SVDMCFudgeFactorFunction& a) : TObject(a)
{
  m_chebyCoeffs = a.m_chebyCoeffs;

  m_x = a.m_x;
  m_y = a.m_y;

  m_current = a.m_current;

}

SVDMCFudgeFactorFunction& SVDMCFudgeFactorFunction::operator=(const SVDMCFudgeFactorFunction& a)
{
  if (this == &a)
    return *this;

  m_chebyCoeffs = a.m_chebyCoeffs;

  m_x = a.m_x;
  m_y = a.m_y;

  m_current = a.m_current;

  return *this;
}

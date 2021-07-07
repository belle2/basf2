/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include<svd/dbobjects/SVDHitTimeSelectionFunction.h>

using namespace Belle2;

std::vector <SVDHitTimeSelectionFunction::selFunction>
SVDHitTimeSelectionFunction::m_implementations;

/*
SVDHitTimeSelectionFunction::SVDHitTimeSelectionFunction(const SVDHitTimeSelectionFunction& a):TObject()
{
  m_current = a.m_current;


  //first version:
  m_tMin = a.m_tMin;

  //second version
  m_deltaT = a.m_deltaT;

  //third
  m_nSigma = a.m_nSigma;


}
*/


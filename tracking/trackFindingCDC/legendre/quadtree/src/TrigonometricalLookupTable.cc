/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/legendre/quadtree/TrigonometricalLookupTable.h>

#include <utility>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

TrigonometricalLookupTable& TrigonometricalLookupTable::Instance()
{
  static TrigonometricalLookupTable trigonometricalLookupTable;
  return trigonometricalLookupTable;
}

TrigonometricalLookupTable::TrigonometricalLookupTable():
  m_lookup_created(false), m_nbinsTheta(pow(2, 16))
{

}

void TrigonometricalLookupTable::initialize()
{
  if (not m_lookup_created) {

    m_lookup_theta.resize(m_nbinsTheta + 1);

    for (unsigned long i = 0; i <= m_nbinsTheta; ++i) {
      m_lookup_theta[i] = std::make_pair(computeSin(i), computeCos(i));
    }

    m_lookup_created = true;
  }
}

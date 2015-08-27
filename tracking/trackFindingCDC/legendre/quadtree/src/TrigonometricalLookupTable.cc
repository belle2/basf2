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


using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

TrigonometricalLookupTable* TrigonometricalLookupTable::s_trigonometricalLookupTable = nullptr;

TrigonometricalLookupTable& TrigonometricalLookupTable::Instance()
{
  // todo: memory leak
  if (!s_trigonometricalLookupTable) s_trigonometricalLookupTable = new TrigonometricalLookupTable();
  return *s_trigonometricalLookupTable;
}


TrigonometricalLookupTable::TrigonometricalLookupTable():
  m_lookup_created(false), m_nbinsTheta(pow(2, 15))
{

}

TrigonometricalLookupTable::~TrigonometricalLookupTable()
{

  clearTable();

}


void TrigonometricalLookupTable::clearTable()
{

  if (m_lookup_created) {
    delete[] m_sin_theta;
    delete[] m_cos_theta;
    m_lookup_created = false;
  }

}



void TrigonometricalLookupTable::initialize(bool forced)
{
  if ((not m_lookup_created) || (forced)) {

    if (forced && m_lookup_created) {
      clearTable();
    }

    float bin_width = s_PI / m_nbinsTheta;
    m_sin_theta = new float[m_nbinsTheta + 1];
    m_cos_theta = new float[m_nbinsTheta + 1];

    for (int i = 0; i <= m_nbinsTheta; ++i) {
      m_sin_theta[i] = sin(i * bin_width);
      m_cos_theta[i] = cos(i * bin_width);
    }

    m_lookup_created = true;
  }

}

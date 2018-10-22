#include<svd/dbobjects/SVDHitTimeSelectionFunction.h>
#include<framework/logging/Logger.h>

using namespace Belle2;

std::vector <SVDHitTimeSelectionFunction::selFunction>
SVDHitTimeSelectionFunction::m_implementations;

SVDHitTimeSelectionFunction::SVDHitTimeSelectionFunction(const SVDHitTimeSelectionFunction& a)
{
  //first version:
  m_tMin = a.m_tMin;

  m_current = a.m_current;
}



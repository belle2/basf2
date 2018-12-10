#include<svd/dbobjects/SVDHitTimeSelectionFunction.h>
#include<framework/logging/Logger.h>

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


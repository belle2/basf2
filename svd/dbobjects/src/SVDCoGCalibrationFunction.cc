#include<svd/dbobjects/SVDCoGCalibrationFunction.h>
#include<framework/logging/Logger.h>

using namespace Belle2;

std::vector <SVDCoGCalibrationFunction::cogFunction>
SVDCoGCalibrationFunction::m_implementations;

SVDCoGCalibrationFunction::SVDCoGCalibrationFunction(const SVDCoGCalibrationFunction& a)
{

  m_current = a.m_current;

  m_implementations.at(m_current) = (a.m_implementations).at(m_current);
  B2INFO("CI SONO");

  for (int tb = 0; tb < 4; tb++) {
    m_shift[tb] = a.m_shift[tb];
    m_scale[tb] = a.m_scale[tb];
  }

};


#include<svd/dbobjects/SVDCoGCalibrationFunction.h>
#include<framework/logging/Logger.h>

using namespace Belle2;

std::vector <SVDCoGCalibrationFunction::cogFunction>
SVDCoGCalibrationFunction::m_implementations;

SVDCoGCalibrationFunction::SVDCoGCalibrationFunction(const SVDCoGCalibrationFunction& a)
{
  for (int i = 0; i < nTriggerBins; i++) {
    m_bias[i] = a.m_bias[i];
    m_scale[i] = a.m_scale[i];
  }
  m_current = a.m_current;
}



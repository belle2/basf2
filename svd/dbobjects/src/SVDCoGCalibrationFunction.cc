/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include<svd/dbobjects/SVDCoGCalibrationFunction.h>

using namespace Belle2;

std::vector <SVDCoGCalibrationFunction::cogFunction> SVDCoGCalibrationFunction::m_implementations;

std::vector <SVDCoGCalibrationFunction::cogFunctionErr> SVDCoGCalibrationFunction::m_implementationsErr;

SVDCoGCalibrationFunction::SVDCoGCalibrationFunction(const SVDCoGCalibrationFunction& a) : TObject(a)
{
  for (int i = 0; i < nTriggerBins; i++) {
    m_bias[i] = a.m_bias[i];
    m_scale[i] = a.m_scale[i];
  }
  m_current = a.m_current;
  for (int i = 0; i < m_nPar; i++)
    m_par[i] = a.m_par[i];
}

SVDCoGCalibrationFunction& SVDCoGCalibrationFunction::operator=(const SVDCoGCalibrationFunction& a)
{
  if (this == &a)
    return *this;
  for (int i = 0; i < nTriggerBins; i++) {
    m_bias[i] = a.m_bias[i];
    m_scale[i] = a.m_scale[i];
  }
  m_current = a.m_current;
  for (int i = 0; i < m_nPar; i++)
    m_par[i] = a.m_par[i];
  return *this;
}

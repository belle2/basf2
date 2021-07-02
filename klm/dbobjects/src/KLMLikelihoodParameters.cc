/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/dbobjects/KLMLikelihoodParameters.h>

/* Belle 2 headers. */
#include <framework/logging/Logger.h>

using namespace Belle2;

const std::vector<double>& KLMLikelihoodParameters::getLongitudinalPDF(const int id) const
{
  std::map<int, std::vector<double>>::const_iterator it = m_LongitudinalPDF.find(id);
  if (it != m_LongitudinalPDF.end()) {
    return it->second;
  } else {
    B2FATAL("Longitudinal for KLMLikelihoodParameters not found!");
  }
}

const std::vector<double>& KLMLikelihoodParameters::getTransversePDF(const int id) const
{
  std::map<int, std::vector<double>>::const_iterator it = m_TransversePDF.find(id);
  if (it != m_TransversePDF.end()) {
    return it->second;
  } else {
    B2FATAL("Transverse PDF for KLMLikelihoodParameters not found!");
  }
}

double KLMLikelihoodParameters::getTransverseThreshold(const int id) const
{
  std::map<int, double>::const_iterator it = m_TransverseThreshold.find(id);
  if (it != m_TransverseThreshold.end()) {
    return it->second;
  } else {
    B2FATAL("Transverse threshold for KLMLikelihoodParameters not found!");
  }
}

double KLMLikelihoodParameters::getTransverseScaleX(const int id) const
{
  std::map<int, double>::const_iterator it = m_TransverseScaleX.find(id);
  if (it != m_TransverseScaleX.end()) {
    return it->second;
  } else {
    B2FATAL("Horizontal transverse scale for KLMLikelihoodParameters not found!");
  }
}

double KLMLikelihoodParameters::getTransverseScaleY(const int id) const
{
  std::map<int, double>::const_iterator it = m_TransverseScaleY.find(id);
  if (it != m_TransverseScaleY.end()) {
    return it->second;
  } else {
    B2FATAL("Vertical transverse scale for KLMLikelihoodParameters not found!");
  }
}

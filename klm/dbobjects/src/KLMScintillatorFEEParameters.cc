/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin, Giacomo De Pietro                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/dbobjects/KLMScintillatorFEEParameters.h>

using namespace Belle2;

KLMScintillatorFEEParameters::KLMScintillatorFEEParameters()
{
}

KLMScintillatorFEEParameters::~KLMScintillatorFEEParameters()
{
}

void KLMScintillatorFEEParameters::setFEEData(uint16_t strip, KLMScintillatorFEEData* data)
{
  std::map<uint16_t, KLMScintillatorFEEData>::iterator it;
  it = m_FEEParameters.find(strip);
  if (it == m_FEEParameters.end())
    m_FEEParameters.insert(std::pair<uint16_t, KLMScintillatorFEEData>(strip, *data));
  else
    it->second = *data;
}

const KLMScintillatorFEEData* KLMScintillatorFEEParameters::getFEEData(uint16_t strip) const
{
  std::map<uint16_t, KLMScintillatorFEEData>::const_iterator it;
  it = m_FEEParameters.find(strip);
  if (it == m_FEEParameters.end())
    return nullptr;
  return &(it->second);
}

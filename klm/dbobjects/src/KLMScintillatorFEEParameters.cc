/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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

void KLMScintillatorFEEParameters::setFEEData(
  KLMChannelNumber strip, KLMScintillatorFEEData* data)
{
  std::map<KLMChannelNumber, KLMScintillatorFEEData>::iterator it;
  it = m_FEEParameters.find(strip);
  if (it == m_FEEParameters.end())
    m_FEEParameters.insert(std::pair<KLMChannelNumber, KLMScintillatorFEEData>(strip, *data));
  else
    it->second = *data;
}

const KLMScintillatorFEEData* KLMScintillatorFEEParameters::getFEEData(
  KLMChannelNumber strip) const
{
  std::map<KLMChannelNumber, KLMScintillatorFEEData>::const_iterator it;
  it = m_FEEParameters.find(strip);
  if (it == m_FEEParameters.end())
    return nullptr;
  return &(it->second);
}

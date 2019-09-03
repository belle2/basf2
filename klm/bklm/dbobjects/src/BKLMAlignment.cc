/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <framework/logging/Logger.h>
#include <klm/bklm/dbobjects/BKLMAlignment.h>

using namespace Belle2;

BKLMAlignment::BKLMAlignment()
{
}

BKLMAlignment::~BKLMAlignment()
{
}

void BKLMAlignment::setModuleAlignment(uint16_t module,
                                       KLMAlignmentData* dat)
{
  std::map<uint16_t, KLMAlignmentData>::iterator it;
  it = m_ModuleAlignment.find(module);
  if (it == m_ModuleAlignment.end()) {
    m_ModuleAlignment.insert(
      std::pair<uint16_t, KLMAlignmentData>(module, *dat));
  } else {
    it->second = *dat;
  }
}

const KLMAlignmentData* BKLMAlignment::getModuleAlignment(
  uint16_t module) const
{
  std::map<uint16_t, KLMAlignmentData>::const_iterator it;
  it = m_ModuleAlignment.find(module);
  if (it == m_ModuleAlignment.end())
    return nullptr;
  return &(it->second);
}

double BKLMAlignment::getGlobalParam(unsigned short element,
                                     unsigned short param) const
{
  const KLMAlignmentData* alignmentData = getModuleAlignment(element);
  if (alignmentData == nullptr)
    return 0;
  return alignmentData->getParameter(
           static_cast<enum KLMAlignmentData::ParameterNumbers>(param));
}

void BKLMAlignment::setGlobalParam(double value, unsigned short element,
                                   unsigned short param)
{
  KLMAlignmentData* alignmentData =
    const_cast<KLMAlignmentData*>(getModuleAlignment(element));
  if (alignmentData == nullptr)
    return;
  alignmentData->setParameter(
    static_cast<enum KLMAlignmentData::ParameterNumbers>(param), value);
}

/* TODO: this function is not implemented. */
std::vector< std::pair<unsigned short, unsigned short> >
BKLMAlignment::listGlobalParams()
{
  return {};
}

/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/dbobjects/bklm/BKLMAlignment.h>

using namespace Belle2;

BKLMAlignment::BKLMAlignment()
{
}

BKLMAlignment::~BKLMAlignment()
{
}

void BKLMAlignment::setModuleAlignment(KLMModuleNumber module,
                                       KLMAlignmentData* dat)
{
  std::map<KLMModuleNumber, KLMAlignmentData>::iterator it;
  it = m_ModuleAlignment.find(module);
  if (it == m_ModuleAlignment.end()) {
    m_ModuleAlignment.insert(
      std::pair<KLMModuleNumber, KLMAlignmentData>(module, *dat));
  } else {
    it->second = *dat;
  }
}

const KLMAlignmentData* BKLMAlignment::getModuleAlignment(
  KLMModuleNumber module) const
{
  std::map<KLMModuleNumber, KLMAlignmentData>::const_iterator it;
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
  /*
   * Create alignment data if it does not exist.
   * This is necessary for errors and corrections.
   */
  if (alignmentData == nullptr) {
    KLMAlignmentData newAlignmentData(0, 0, 0, 0, 0, 0);
    newAlignmentData.setParameter(
      static_cast<enum KLMAlignmentData::ParameterNumbers>(param), value);
    m_ModuleAlignment.insert(
      std::pair<KLMModuleNumber, KLMAlignmentData>(element, newAlignmentData));
  } else {
    alignmentData->setParameter(
      static_cast<enum KLMAlignmentData::ParameterNumbers>(param), value);
  }
}

/* TODO: this function is not implemented. */
std::vector< std::pair<unsigned short, unsigned short> >
BKLMAlignment::listGlobalParams()
{
  return {};
}

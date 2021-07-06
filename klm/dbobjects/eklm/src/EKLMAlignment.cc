/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/dbobjects/eklm/EKLMAlignment.h>

using namespace Belle2;

EKLMAlignment::EKLMAlignment()
{
}

EKLMAlignment::~EKLMAlignment()
{
}

void EKLMAlignment::setModuleAlignment(KLMModuleNumber module,
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

const KLMAlignmentData* EKLMAlignment::getModuleAlignment(
  KLMModuleNumber module) const
{
  std::map<KLMModuleNumber, KLMAlignmentData>::const_iterator it;
  it = m_ModuleAlignment.find(module);
  if (it == m_ModuleAlignment.end())
    return nullptr;
  return &(it->second);
}

double EKLMAlignment::getGlobalParam(unsigned short element,
                                     unsigned short param) const
{
  const KLMAlignmentData* alignmentData = getModuleAlignment(element);
  if (alignmentData == nullptr)
    return 0;
  return alignmentData->getParameter(
           static_cast<enum KLMAlignmentData::ParameterNumbers>(param));
}

void EKLMAlignment::setGlobalParam(double value, unsigned short element,
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
EKLMAlignment::listGlobalParams()
{
  return {};
}

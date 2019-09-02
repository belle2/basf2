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
#include <klm/dataobjects/KLMAlignableElement.h>

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
  const KLMAlignmentData* alignmentData;
  KLMAlignableElement id(element);
  alignmentData = getModuleAlignment(id.getModuleNumber());
  if (alignmentData == nullptr)
    return 0;
  switch (param) {
    case KLMAlignmentData::c_DeltaU:
      return alignmentData->getDeltaU();
    case KLMAlignmentData::c_DeltaV:
      return alignmentData->getDeltaV();
    case KLMAlignmentData::c_DeltaW:
      return alignmentData->getDeltaW();
    case KLMAlignmentData::c_DeltaAlpha:
      return alignmentData->getDeltaAlpha();
    case KLMAlignmentData::c_DeltaBeta:
      return alignmentData->getDeltaBeta();
    case KLMAlignmentData::c_DeltaGamma:
      return alignmentData->getDeltaGamma();
  }
  B2FATAL("Attempt to get BKLM alignment parameter with incorrect number " <<
          param);
  return 0;
}

void BKLMAlignment::setGlobalParam(double value, unsigned short element,
                                   unsigned short param)
{
  KLMAlignmentData* alignmentData;
  KLMAlignableElement id(element);
  alignmentData = const_cast<KLMAlignmentData*>(
                    getModuleAlignment(id.getModuleNumber()));
  if (alignmentData == nullptr)
    return;
  switch (param) {
    case KLMAlignmentData::c_DeltaU:
      alignmentData->setDeltaU(value);
      return;
    case KLMAlignmentData::c_DeltaV:
      alignmentData->setDeltaV(value);
      return;
    case KLMAlignmentData::c_DeltaW:
      alignmentData->setDeltaW(value);
      return;
    case KLMAlignmentData::c_DeltaAlpha:
      alignmentData->setDeltaAlpha(value);
      return;
    case KLMAlignmentData::c_DeltaBeta:
      alignmentData->setDeltaBeta(value);
      return;
    case KLMAlignmentData::c_DeltaGamma:
      alignmentData->setDeltaGamma(value);
      return;
  }
  B2FATAL("Attempt to set BKLM alignment parameter with incorrect number " <<
          param);
}

/* TODO: this function is not implemented. */
std::vector< std::pair<unsigned short, unsigned short> >
BKLMAlignment::listGlobalParams()
{
  return {};
}

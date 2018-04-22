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
#include <eklm/dbobjects/EKLMAlignmentData.h>

using namespace Belle2;

EKLMAlignmentData::EKLMAlignmentData()
{
  m_Dx = 0;
  m_Dy = 0;
  m_Dalpha = 0;
}

EKLMAlignmentData::EKLMAlignmentData(float dx, float dy, float dalpha)
{
  m_Dx = dx;
  m_Dy = dy;
  m_Dalpha = dalpha;
}

EKLMAlignmentData::~EKLMAlignmentData()
{
}

void EKLMAlignmentData::setDx(float dx)
{
  m_Dx = dx;
}

float EKLMAlignmentData::getDx() const
{
  return m_Dx;
}

void EKLMAlignmentData::setDy(float dy)
{
  m_Dy = dy;
}

float EKLMAlignmentData::getDy() const
{
  return m_Dy;
}

void EKLMAlignmentData::setDalpha(float dalpha)
{
  m_Dalpha = dalpha;
}

float EKLMAlignmentData::getDalpha() const
{
  return m_Dalpha;
}


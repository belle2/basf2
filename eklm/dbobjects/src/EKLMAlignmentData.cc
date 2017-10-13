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

EKLMAlignmentData::EKLMAlignmentData(double dx, double dy, double dalpha)
{
  m_Dx = dx;
  m_Dy = dy;
  m_Dalpha = dalpha;
}

EKLMAlignmentData::~EKLMAlignmentData()
{
}

void EKLMAlignmentData::setDx(double dx)
{
  m_Dx = dx;
}

double EKLMAlignmentData::getDx() const
{
  return m_Dx;
}

void EKLMAlignmentData::setDy(double dy)
{
  m_Dy = dy;
}

double EKLMAlignmentData::getDy() const
{
  return m_Dy;
}

void EKLMAlignmentData::setDalpha(double dalpha)
{
  m_Dalpha = dalpha;
}

double EKLMAlignmentData::getDalpha() const
{
  return m_Dalpha;
}


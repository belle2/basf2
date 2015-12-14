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

ClassImp(Belle2::EKLMAlignmentData);

EKLMAlignmentData::EKLMAlignmentData()
{
  m_dx = 0;
  m_dy = 0;
  m_dalpha = 0;
}

EKLMAlignmentData::EKLMAlignmentData(double dx, double dy, double dalpha)
{
  m_dx = dx;
  m_dy = dy;
  m_dalpha = dalpha;
}

EKLMAlignmentData::~EKLMAlignmentData()
{
}

void EKLMAlignmentData::setDx(double dx)
{
  m_dx = dx;
}

double EKLMAlignmentData::getDx()
{
  return m_dx;
}

void EKLMAlignmentData::setDy(double dy)
{
  m_dy = dy;
}

double EKLMAlignmentData::getDy()
{
  return m_dy;
}

void EKLMAlignmentData::setDalpha(double dalpha)
{
  m_dalpha = dalpha;
}

double EKLMAlignmentData::getDalpha()
{
  return m_dalpha;
}


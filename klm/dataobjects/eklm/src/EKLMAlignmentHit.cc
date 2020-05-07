/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/dataobjects/eklm/EKLMAlignmentHit.h>

using namespace Belle2;

EKLMAlignmentHit::EKLMAlignmentHit()
{
  m_DigitIdentifier = 0;
}

EKLMAlignmentHit::EKLMAlignmentHit(int digitIdentifier)
{
  m_DigitIdentifier = digitIdentifier;
}

EKLMAlignmentHit::~EKLMAlignmentHit()
{
}

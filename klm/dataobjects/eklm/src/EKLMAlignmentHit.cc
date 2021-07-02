/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/dataobjects/eklm/EKLMAlignmentHit.h>

using namespace Belle2;

EKLMAlignmentHit::EKLMAlignmentHit() :
  m_DigitIdentifier(0)
{
}

EKLMAlignmentHit::EKLMAlignmentHit(int digitIdentifier) :
  m_DigitIdentifier(digitIdentifier)
{
}

EKLMAlignmentHit::~EKLMAlignmentHit()
{
}

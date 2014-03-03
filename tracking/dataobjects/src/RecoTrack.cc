/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/dataobjects/RecoTrack.h>

using namespace Belle2;

ClassImp(RecoTrack);

void RecoTrack::resetHitIndices(const short pseudoCharge,
                                const Const::EDetector detector)
{
  if ((detector == Const::EDetector::CDC or
       detector == Const::EDetector::invalidDetector) and
      pseudoCharge >= 0)
    m_cdcHitIndicesPositive.clear();

  if ((detector == Const::EDetector::CDC or
       detector == Const::EDetector::invalidDetector) and
      pseudoCharge <= 0)
    m_cdcHitIndicesNegative.clear();

  if ((detector == Const::EDetector::SVD or
       detector == Const::EDetector::invalidDetector) and
      pseudoCharge >= 0)
    m_svdHitIndicesPositive.clear();

  if ((detector == Const::EDetector::SVD or
       detector == Const::EDetector::invalidDetector) and
      pseudoCharge <= 0)
    m_svdHitIndicesNegative.clear();

  if ((detector == Const::EDetector::PXD or
       detector == Const::EDetector::invalidDetector) and
      pseudoCharge >= 0)
    m_pxdHitIndicesPositive.clear();

  if ((detector == Const::EDetector::PXD or
       detector == Const::EDetector::invalidDetector) and
      pseudoCharge <= 0)
    m_pxdHitIndicesNegative.clear();
}

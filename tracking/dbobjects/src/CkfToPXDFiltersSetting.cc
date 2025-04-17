/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/dbobjects/CkfToPXDFiltersSetting.h>
#include <limits>

using namespace Belle2;

CkfToPXDFiltersSetting::CkfToPXDFiltersSetting() : TObject()
  , m_paramPtThresholdTrackToHitCut(std::numeric_limits<double>::quiet_NaN())
  , m_paramPhiInterceptToHitCut(std::numeric_limits<double>::quiet_NaN())
  , m_paramEtaInterceptToHitCut(std::numeric_limits<double>::quiet_NaN())
  , m_paramPhiRecoTrackToHitCut(std::numeric_limits<double>::quiet_NaN())
  , m_paramEtaRecoTrackToHitCut(std::numeric_limits<double>::quiet_NaN())
  , m_paramPhiHitHitCut(std::numeric_limits<double>::quiet_NaN())
  , m_paramEtaHitHitCut(std::numeric_limits<double>::quiet_NaN())
  , m_paramPhiOverlapHitHitCut(std::numeric_limits<double>::quiet_NaN())
  , m_paramEtaOverlapHitHitCut(std::numeric_limits<double>::quiet_NaN())
{}


/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/dbobjects/ckf_ToPXD_FiltersSetting.h>
#include <limits>

using namespace Belle2;

ckf_ToPXD_FiltersSetting::ckf_ToPXD_FiltersSetting() : TObject()
  , m_param_PtThresholdTrackToHitCut(std::numeric_limits<double>::quiet_NaN())
  , m_param_PhiInterceptToHitCut(std::numeric_limits<double>::quiet_NaN())
  , m_param_EtaInterceptToHitCut(std::numeric_limits<double>::quiet_NaN())
  , m_param_PhiRecoTrackToHitCut(std::numeric_limits<double>::quiet_NaN())
  , m_param_EtaRecoTrackToHitCut(std::numeric_limits<double>::quiet_NaN())
  , m_param_PhiHitHitCut(std::numeric_limits<double>::quiet_NaN())
  , m_param_EtaHitHitCut(std::numeric_limits<double>::quiet_NaN())
  , m_param_PhiOverlapHitHitCut(std::numeric_limits<double>::quiet_NaN())
  , m_param_EtaOverlapHitHitCut(std::numeric_limits<double>::quiet_NaN())
{}


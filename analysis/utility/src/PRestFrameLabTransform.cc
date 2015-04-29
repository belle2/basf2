/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/utility/PRestFrameLabTransform.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

TLorentzRotation PRestFrameLabTransform::m_restframe2lab;
TLorentzRotation PRestFrameLabTransform::m_lab2restframe;

TLorentzVector PRestFrameLabTransform::labToRestFrame(const TLorentzVector& vector)
{
  static PRestFrameLabTransform T;
  return T.rotateLabToRestFrame() * vector;
}

TLorentzVector PRestFrameLabTransform::restFrameToLab(const TLorentzVector& vector)
{
  static PRestFrameLabTransform T;
  return T.rotateRestFrameToLab() * vector;
}
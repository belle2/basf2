/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/ckf/svd/entities/CKFToSVDState.h>
#include <tracking/dataobjects/RecoTrack.h>

using namespace Belle2;

CKFToSVDState::CKFToSVDState(const RecoTrack* seed) : CKFState(seed)
{
  setMeasuredStateOnPlane(seed->getMeasuredStateOnPlaneFromFirstHit());
}
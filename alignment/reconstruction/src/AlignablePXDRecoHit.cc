/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012, 2015 - Belle II Collaboration                       *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <alignment/reconstruction/AlignablePXDRecoHit.h>
#include <alignment/Manager.h>
#include <alignment/Hierarchy.h>
#include <alignment/dbobjects/VXDAlignment.h>

#include <alignment/GlobalDerivatives.h>


using namespace std;
using namespace Belle2;
using namespace alignment;

std::pair<std::vector<int>, TMatrixD> AlignablePXDRecoHit::globalDerivatives(const genfit::StateOnPlane* sop)
{
  auto globals = GlobalCalibrationManager::getInstance().getAlignmentHierarchy().getGlobalDerivatives<VXDAlignment>(getPlaneId(),
                 sop);
  // TODO: Move Lorentz outside
  //auto globalsLorentz = GlobalCalibrationManager::getInstance().getLorentzShiftHierarchy().getGlobalDerivatives<VXDAlignment>(getPlaneId(),
  //               sop);
  return GlobalDerivatives(globals);
}

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

#include <alignment/Hierarchy.h>
#include <alignment/dbobjects/VXDAlignment.h>

using namespace std;
using namespace Belle2;
using namespace alignment;

std::pair<std::vector<int>, TMatrixD> AlignablePXDRecoHit::globalDerivatives(const genfit::StateOnPlane* sop)
{
  return GlobalDerivatives::passGlobals(
           HierarchyManager::getInstance().getAlignmentHierarchy().getGlobalDerivatives<VXDAlignment>(getPlaneId(), sop));
}

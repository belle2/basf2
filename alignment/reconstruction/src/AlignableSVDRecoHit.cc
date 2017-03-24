/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012, 2015 - Belle II Collaboration                       *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <alignment/reconstruction/AlignableSVDRecoHit.h>

#include <alignment/Hierarchy.h>
#include <alignment/dbobjects/VXDAlignment.h>

using namespace std;
using namespace Belle2;

std::pair<std::vector<int>, TMatrixD> AlignableSVDRecoHit::globalDerivatives(const genfit::StateOnPlane* sop)
{
  return alignment::HierarchyManager::getInstance().getAlignmentHierarchy().getGlobalDerivatives<VXDAlignment>(getPlaneId(), sop);
}
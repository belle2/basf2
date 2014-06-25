/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/FacetFilterTree.h"

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

FacetFilterTree::FacetFilterTree() :
  FacetFilterTreeBase("facet", "Variables to select correct facets and the mc truth.")
{
}

FacetFilterTree::~FacetFilterTree()
{
}

bool FacetFilterTree::setValues(const CellWeight& mcWeight, const CellWeight& prWeight, const CDCRecoFacet& facet)
{
  //setValue < NAMED("mcWeight") > (mcWeight);
  //setValue < NAMED("prWeight") > (prWeight);

  bool mcDecision = not isNotACell(mcWeight);
  bool prDecision = not isNotACell(prWeight);

  setValue < NAMED("mcDecision") > (mcDecision);
  setValue < NAMED("prDecision") > (prDecision);

  const ParameterLine2D& startToMiddle = facet.getStartToMiddleLine();
  const ParameterLine2D& startToEnd    = facet.getStartToEndLine();
  const ParameterLine2D& middleToEnd   = facet.getMiddleToEndLine();

  Float_t smToSECosine = startToMiddle.tangential().cosWith(startToEnd.tangential());
  Float_t smToMECosine = startToMiddle.tangential().cosWith(middleToEnd.tangential());
  Float_t seToMECosine = startToEnd.tangential().cosWith(middleToEnd.tangential());

  setValue < NAMED("smToSECosine") > (smToSECosine);
  setValue < NAMED("smToMECosine") > (smToMECosine);
  setValue < NAMED("seToMECosine") > (seToMECosine);

  return true;
}

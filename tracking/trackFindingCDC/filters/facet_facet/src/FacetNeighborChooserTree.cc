/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include "../include/FacetNeighborChooserTree.h"

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;



FacetNeighborChooserTree::FacetNeighborChooserTree() :
  FacetNeighborChooserTreeBase("facet_facet", "Variables to select correct facet neighbors and the mc truth.")
{
}



FacetNeighborChooserTree::~FacetNeighborChooserTree()
{
}



bool FacetNeighborChooserTree::setValues(
  const CellWeight& mcWeight,
  const CellWeight& prWeight,
  const CDCRecoFacet& facet,
  const CDCRecoFacet& neighborFacet
)
{
  //setValue < NAMED("mcWeight") > (mcWeight);
  //setValue < NAMED("prWeight") > (prWeight);

  bool mcDecision = not isNotACell(mcWeight);
  bool prDecision = not isNotACell(prWeight);

  setValue < NAMED("mcDecision") > (mcDecision);
  setValue < NAMED("prDecision") > (prDecision);

  //the compatibility of the short legs or all?
  //start end to continuation middle end
  //start middle to continuation start end

  const ParameterLine2D& firstStartToMiddle = facet.getStartToMiddleLine();
  const ParameterLine2D& firstStartToEnd    = facet.getStartToEndLine();

  const ParameterLine2D& secondStartToEnd   = neighborFacet.getStartToEndLine();
  const ParameterLine2D& secondMiddleToEnd  = neighborFacet.getMiddleToEndLine();

  Float_t smToNeighborSECosine = firstStartToMiddle.tangential().cosWith(secondStartToEnd.tangential());
  Float_t seToNeighborMECosine = firstStartToEnd.tangential().cosWith(secondMiddleToEnd.tangential());

  setValue < NAMED("smToNeighborSECosine") > (smToNeighborSECosine);
  setValue < NAMED("seToNeighborMECosine") > (seToNeighborMECosine);

  return true;
}

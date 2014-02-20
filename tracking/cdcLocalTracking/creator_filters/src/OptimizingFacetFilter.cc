/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/OptimizingFacetFilter.h"

#include <cmath>
#include <framework/logging/Logger.h>

#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>
#include <tracking/cdcLocalTracking/typedefs/BasicConstants.h>


using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

OptimizingFacetFilter::OptimizingFacetFilter()
{

  m_output_csv.open("facet_creator_filter.csv");

  m_output_csv << "SMToSECosine,"
               << "SEToMECosine,"
               << "SMToMECosine,"
               << "MCTruth"
               << endl;
}

OptimizingFacetFilter::~OptimizingFacetFilter()
{

  m_output_csv.close();

}

CellState OptimizingFacetFilter::isGoodFacet(const CDCRecoFacet& facet) const
{

  CellState mcCellWeight = m_mcFacetFilter.isGoodFacet(facet);
  //bool mcDecision = (not isNotACell(mcCellWeight));

  //CDCRecoFacet reversedFacet = facet.reversed();
  //CellState mcReversedCellWeight = m_mcFacetFilter.isGoodFacet(reversedFacet);

  bool mcDecision = not isNotACell(mcCellWeight);

  facet.adjustLines();

  const ParameterLine2D& startToMiddle = facet.getStartToMiddleLine();
  const ParameterLine2D& startToEnd    = facet.getStartToEndLine();
  const ParameterLine2D& middleToEnd   = facet.getMiddleToEndLine();

  m_output_csv << startToMiddle.tangential().cosWith(startToEnd.tangential()) << ","
               << startToEnd.tangential().cosWith(middleToEnd.tangential()) << ","
               << startToMiddle.tangential().cosWith(middleToEnd.tangential()) << ","
               << mcDecision
               << endl;

  //return mcCellWeight;

  CellState simpleCellWeight = m_simpleFacetFilter.isGoodFacet(facet);
  return simpleCellWeight;


}

void OptimizingFacetFilter::clear() const {;}

void OptimizingFacetFilter::initialize()
{
}


void OptimizingFacetFilter::terminate()
{
}

/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bastian Kronenbitter, Thomas Hauth, Viktor Trusov,       *
 *               Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/AxialTrackCreatorHitLegendre.h>

#include <tracking/trackFindingCDC/legendre/quadtree/QuadTreeCandidateFinder.h>
#include <tracking/trackFindingCDC/legendre/quadtree/QuadTreeNodeProcessor.h>
#include <tracking/trackFindingCDC/legendre/quadtree/AxialHitQuadTreeProcessor.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.h>

using namespace Belle2;
using namespace TrackFindingCDC;

AxialTrackCreatorHitLegendre::AxialTrackCreatorHitLegendre() = default;

AxialTrackCreatorHitLegendre::AxialTrackCreatorHitLegendre(LegendreFindingPass pass)
  : m_pass(pass)
{
}

std::string AxialTrackCreatorHitLegendre::getDescription()
{
  return "Generates axial tracks from hits using several increasingly relaxed legendre space search over phi0 and curvature.";
}

void AxialTrackCreatorHitLegendre::exposeParameters(ModuleParamList* moduleParamList __attribute__((unused)),
                                                    const std::string& prefix __attribute__((unused)))
{
}

void AxialTrackCreatorHitLegendre::initialize()
{
  B2ASSERT("Maximal level of QuadTree search is setted to be greater than lookuptable grid level! ",
           m_param_maxLevel <= PrecisionUtil::getLookupGridLevel());

  Super::initialize();
}

void AxialTrackCreatorHitLegendre::apply(const std::vector<const CDCWireHit*>& axialWireHits,
                                         std::vector<CDCTrack>& tracks)
{
  // Prepare vector of unused hits provide it to the qt processor
  // Also reset the mask flag and select only the untaken hits
  std::vector<const CDCWireHit*> unusedAxialWireHits;
  for (const CDCWireHit* wireHit : axialWireHits) {
    (*wireHit)->setMaskedFlag(false);
    if ((*wireHit)->hasTakenFlag()) continue;
    unusedAxialWireHits.push_back(wireHit);
  }

  // Create object which holds and generates parameters
  QuadTreeParameters quadTreeParameters(m_param_maxLevel, m_pass);

  //Create quadtree processot
  std::unique_ptr<AxialHitQuadTreeProcessor> qtProcessor{quadTreeParameters.constructQTProcessor()};

  qtProcessor->provideItemsSet(unusedAxialWireHits);
  //  qtProcessor.seedQuadTree(4, symmetricalKappa);

  // Create object which contains interface between quadtree processor and track processor (module)
  QuadTreeNodeProcessor quadTreeNodeProcessor(*qtProcessor, quadTreeParameters.getPrecisionFunction());

  // Object which operates with AxialHitQuadTreeProcessor and QuadTreeNodeProcessor and starts quadtree search
  QuadTreeCandidateFinder quadTreeCandidateFinder;

  // Interface
  AxialHitQuadTreeProcessor::CandidateProcessorLambda lambdaInterface =
    quadTreeNodeProcessor.getLambdaInterface(axialWireHits, tracks);

  // Start candidate finding
  quadTreeCandidateFinder.doTreeTrackFinding(lambdaInterface, quadTreeParameters, *qtProcessor);
}

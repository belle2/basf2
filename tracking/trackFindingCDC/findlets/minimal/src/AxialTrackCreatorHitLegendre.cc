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

#include <tracking/trackFindingCDC/legendre/quadtree/QuadTreeNodeProcessor.h>
#include <tracking/trackFindingCDC/legendre/quadtreetools/QuadTreeParameters.h>

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

  // Interface
  AxialHitQuadTreeProcessor::CandidateProcessorLambda lambdaInterface =
    quadTreeNodeProcessor.getLambdaInterface(axialWireHits, tracks);

  // Start candidate finding
  this->doTreeTrackFinding(lambdaInterface, quadTreeParameters, *qtProcessor);
}

void AxialTrackCreatorHitLegendre::doTreeTrackFinding(
  AxialHitQuadTreeProcessor::CandidateProcessorLambda& lmdInterface,
  QuadTreeParameters& parameters,
  AxialHitQuadTreeProcessor& qtProcessor)
{
  // radius of the CDC
  double rCDC = 113.;

  // Curvature for high pt particles that leave the CDC
  double curlCurv = 2. / rCDC;

  // Create sectorisation level
  if (parameters.getPass() != LegendreFindingPass::FullRange) {
    qtProcessor.seedQuadTree(4);
  }

  if (parameters.getPass() != LegendreFindingPass::FullRange) {
    qtProcessor.fillSeededTree(lmdInterface, 50, curlCurv);
  } else {
    qtProcessor.fillGivenTree(lmdInterface, 50, curlCurv);
  }
  // qtProcessor.fillGivenTree(lmdInterface, 50, curlCurv);

  // find curlers with diameter higher than half of radius of CDC (see calculations above)
  if (parameters.getPass() != LegendreFindingPass::FullRange) {
    qtProcessor.fillSeededTree(lmdInterface, 70, 2 * curlCurv);
  } else {
    qtProcessor.fillGivenTree(lmdInterface, 70, 2 * curlCurv);
  }
  // qtProcessor.fillGivenTree(lmdInterface, 70, 2 * curlCurv);

  // Start relaxation loop
  int limit = parameters.getInitialHitsLimit();
  double rThreshold = parameters.getCurvThreshold();
  do {
    if (parameters.getPass() != LegendreFindingPass::FullRange) {
      qtProcessor.fillSeededTree(lmdInterface, limit, rThreshold);
    } else {
      qtProcessor.fillGivenTree(lmdInterface, limit, rThreshold);
    }
    // qtProcessor.fillGivenTree(lmdInterface, limit, rThreshold);

    limit = limit * m_param_stepScale;

    if (parameters.getPass() != LegendreFindingPass::NonCurlers) {
      rThreshold *= 2.;
      if (rThreshold > 0.15) rThreshold = 0.15;
    }

  } while (limit >= m_param_threshold);
}

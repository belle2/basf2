/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bastian Kronenbitter, Thomas Hauth, Viktor Trusov,       *
 *               Nils Braun, Dmitrii Neverov                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/AxialTrackCreatorHitLegendre.h>

#include <tracking/trackFindingCDC/legendre/quadtree/OffOriginExtension.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.templateDetails.h>

using namespace Belle2;
using namespace TrackFindingCDC;

using EPass = AxialTrackCreatorHitLegendre::EPass;
std::unique_ptr<AxialHitQuadTreeProcessor> AxialTrackCreatorHitLegendre::constructQTProcessor(EPass pass)
{
  using XYSpans = AxialHitQuadTreeProcessor::XYSpans;
  using PrecisionFunction = PrecisionUtil::PrecisionFunction;
  const int maxTheta = std::pow(2, PrecisionUtil::getLookupGridLevel());

  if (pass == EPass::NonCurlers) {
    int maxLevel = 12;
    int seedLevel = 4;
    XYSpans xySpans({{0, maxTheta}, { -0.02, 0.14}});
    PrecisionFunction precisionFunction = &PrecisionUtil::getOriginCurvPrecision;

    return std::make_unique<AxialHitQuadTreeProcessor>(maxLevel, seedLevel, xySpans, precisionFunction);

  } else if (pass == EPass::NonCurlersWithIncreasingThreshold) {
    int maxLevel = 10;
    int seedLevel = 4;
    XYSpans xySpans({{0, maxTheta}, { -0.02, 0.14}});
    PrecisionFunction precisionFunction = &PrecisionUtil::getNonOriginCurvPrecision;

    return std::make_unique<AxialHitQuadTreeProcessor>(maxLevel, seedLevel, xySpans, precisionFunction);

  } else if (pass == EPass::FullRange) {
    int maxLevel = 10;
    int seedLevel = 1;
    XYSpans xySpans({{0, maxTheta}, {0.00, 0.30}});
    PrecisionFunction precisionFunction = &PrecisionUtil::getNonOriginCurvPrecision;

    return std::make_unique<AxialHitQuadTreeProcessor>(maxLevel, seedLevel, xySpans, precisionFunction);

  } else if (pass == EPass::Straight) {
    int maxLevel = 10;
    int seedLevel = 4;
    XYSpans xySpans({{0, maxTheta}, { -0.02, 0.02}});
//       PrecisionFunction precisionFunction = &PrecisionUtil::getBasicCurvPrecision; //That is 0.3 / pow(2, 16)
    PrecisionFunction precisionFunction = [this](double curv __attribute__((unused))) {return m_param_precision ;};

    return std::make_unique<AxialHitQuadTreeProcessor>(maxLevel, seedLevel, xySpans, precisionFunction);

  }
  B2FATAL("Invalid pass");
}


AxialTrackCreatorHitLegendre::AxialTrackCreatorHitLegendre() = default;

AxialTrackCreatorHitLegendre::AxialTrackCreatorHitLegendre(EPass pass)
  : m_pass(pass)
{
}

std::string AxialTrackCreatorHitLegendre::getDescription()
{
  return "Generates axial tracks from hits using several increasingly relaxed legendre space search over phi0 and curvature.";
}

void AxialTrackCreatorHitLegendre::exposeParameters(ModuleParamList* moduleParamList,
                                                    const std::string& prefix)
{
  moduleParamList->addParameter(prefixed(prefix, "minNHits"),
                                m_param_minNHits,
                                "Parameter to define minimal threshold of number of hits.",
                                m_param_minNHits);
  if (m_pass == EPass::Straight) {
    moduleParamList->addParameter(prefixed(prefix, "precision"),
                                  m_param_precision,
                                  "Parameter to define precision of quadtree search.",
                                  m_param_precision);
  }
}

void AxialTrackCreatorHitLegendre::initialize()
{
  Super::initialize();
}

void AxialTrackCreatorHitLegendre::apply(const std::vector<const CDCWireHit*>& axialWireHits,
                                         std::vector<CDCTrack>& tracks)
{
  // Prepare vector of unused hits to provide to the qt processor
  // Also reset the mask flag and select only the untaken hits
  std::vector<const CDCWireHit*> unusedAxialWireHits;
  for (const CDCWireHit* wireHit : axialWireHits) {
    (*wireHit)->setMaskedFlag(false);
    if ((*wireHit)->hasTakenFlag()) continue;
    unusedAxialWireHits.push_back(wireHit);
  }

  // Create quadtree processor
  std::unique_ptr<AxialHitQuadTreeProcessor> qtProcessor = constructQTProcessor(m_pass);
  qtProcessor->seed(unusedAxialWireHits);
//   qtProcessor->drawHits(unusedAxialWireHits, 9);

  // Create object which contains interface between quadtree processor and track processor (module)
  BaseCandidateReceiver* receiver;
  if (not(m_pass == EPass::Straight)) {
    receiver = new OffOriginExtension(unusedAxialWireHits);
  } else {
    receiver = new BaseCandidateReceiver(unusedAxialWireHits);
  }

  // Start candidate finding
  this->executeRelaxation(std::ref(*receiver), *qtProcessor);

  const std::vector<CDCTrack>& newTracks = receiver->getTracks();
  tracks.insert(tracks.end(), newTracks.begin(), newTracks.end());
  delete receiver;
}

void AxialTrackCreatorHitLegendre::executeRelaxation(const CandidateReceiver& candidateReceiver,
                                                     AxialHitQuadTreeProcessor& qtProcessor)
{
  // radius of the CDC
  const double rCDC = 113.;

  // Curvature for high pt particles that leave the CDC
  const double curlCurv = 2. / rCDC;

  // find leavers
  qtProcessor.fill(candidateReceiver, 50, curlCurv);

  // find curlers with diameter higher than half of radius of CDC
  qtProcessor.fill(candidateReceiver, 70, 2 * curlCurv);

  // Start relaxation loop
  int minNHits = m_pass == EPass::FullRange ? 30 : 50;
  double maxCurv = m_pass == EPass::FullRange ? 0.15 : 0.07;
  do {
    qtProcessor.fill(candidateReceiver, minNHits, maxCurv);

    minNHits = minNHits * m_param_stepScale;

    if (m_pass != EPass::NonCurlers) {
      maxCurv *= 2.;
      if (maxCurv > 0.15) maxCurv = 0.15;
    }

  } while (minNHits >= m_param_minNHits);
}

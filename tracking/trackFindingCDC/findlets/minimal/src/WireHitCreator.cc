/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/WireHitCreator.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/eventdata/utils/FlightTimeEstimator.h>

#include <tracking/trackFindingCDC/findlets/minimal/EPreferredDirection.h>

#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <cdc/translators/RealisticTDCCountTranslator.h>
#include <cdc/translators/LinearGlobalADCCountTranslator.h>
#include <cdc/geometry/CDCGeometryPar.h>

#include <cdc/dataobjects/CDCHit.h>

#include <framework/datastore/StoreArray.h>
#include <framework/core/ModuleParamList.templateDetails.h>

#include <mdst/dataobjects/MCParticle.h>

using namespace Belle2;
using namespace TrackFindingCDC;

WireHitCreator::WireHitCreator() = default;
WireHitCreator::~WireHitCreator() = default;

std::string WireHitCreator::getDescription()
{
  return "Combines the geometrical information and the raw hit information into wire hits, "
         "which can be used from all modules after that";
}

void WireHitCreator::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(prefixed(prefix, "wirePosition"),
                                m_param_wirePosition,
                                "Set of geometry parameters to be used in the track finding. "
                                "Either 'base', 'misaligned' or 'aligned'.",
                                m_param_wirePosition);

  moduleParamList->addParameter(prefixed(prefix, "ignoreWireSag"),
                                m_param_ignoreWireSag,
                                "Assume a wire sag coefficient of zero "
                                "such that the wires appear to be straight for "
                                "the track finders",
                                m_param_ignoreWireSag);

  moduleParamList->addParameter(prefixed(prefix, "flightTimeEstimation"),
                                m_param_flightTimeEstimation,
                                "Option which flight direction should be assumed for "
                                "an initial time of flight estimation. Options are: "
                                "'none' (no TOF correction), "
                                "'outwards', "
                                "'downwards'.",
                                m_param_flightTimeEstimation);

  moduleParamList->addParameter(prefixed(prefix, "triggerPoint"),
                                m_param_triggerPoint,
                                "Point relative to which the flight times are calculated",
                                m_param_triggerPoint);

  moduleParamList->addParameter(prefixed(prefix, "useSuperLayers"),
                                m_param_useSuperLayers,
                                "List of super layers to be used.",
                                m_param_useSuperLayers);

  moduleParamList->addParameter(prefixed(prefix, "maxDriftTimes"),
                                m_param_maxDriftTimes,
                                "Maximum drift time (nsec) allowed per super layer (in order of super layer#0-8). Default payload-based cut is applied if this value is negative",
                                m_param_maxDriftTimes);

  moduleParamList->addParameter(prefixed(prefix, "useLayers"),
                                m_param_useLayers,
                                "List of layers to be used. "
                                "If a layer number is given in 'ignoreLayers', too, this will result on a B2FATAL. "
                                "Either use 'useLayers' and provide a set of layers to be used, "
                                "or use 'ignoreLayers' and provide a list of layers to be ignored, but not both at the same time.",
                                m_param_useLayers);

  moduleParamList->addParameter(prefixed(prefix, "ignoreLayers"),
                                m_param_ignoreLayers,
                                "List of layers to be ignored. "
                                "If a layer number is given in 'useLayers', too, this will result on a B2FATAL. "
                                "Either use 'useLayers' and provide a set of layers to be used, "
                                "or use 'ignoreLayers' and provide a list of layers to be ignored, but not both at the same time.",
                                m_param_ignoreLayers);

  moduleParamList->addParameter(prefixed(prefix, "useSecondHits"),
                                m_param_useSecondHits,
                                "Use the second hit information in the track finding.",
                                m_param_useSecondHits);

  moduleParamList->addParameter(prefixed(prefix, "useBadWires"),
                                m_param_useBadWires,
                                "Also create the hits that are on bad wires.",
                                m_param_useBadWires);

  moduleParamList->addParameter(prefixed(prefix, "useDegreeSector"),
                                m_param_useDegreeSector,
                                "To angles in degrees for which hits should be created - mostly for debugging",
                                m_param_useDegreeSector);

  moduleParamList->addParameter(prefixed(prefix, "useMCParticleIds"),
                                m_param_useMCParticleIds,
                                "Ids of the MC particles to use. Default does not look at the MCParticles - most for debugging",
                                m_param_useMCParticleIds);
}

void WireHitCreator::initialize()
{
  StoreArray<CDCHit> hits;
  hits.isRequired();

  // Create the wires and layers once during initialisation
  CDCWireTopology::getInstance();

  if (m_param_wirePosition == "base") {
    m_wirePosition = EWirePosition::c_Base;
  } else if (m_param_wirePosition == "misaligned") {
    m_wirePosition = EWirePosition::c_Misaligned;
  } else if (m_param_wirePosition == "aligned") {
    m_wirePosition = EWirePosition::c_Aligned;
  } else {
    B2ERROR("Received unknown wirePosition " << m_param_wirePosition);
  }

  m_tdcCountTranslator.reset(new CDC::RealisticTDCCountTranslator);
  m_adcCountTranslator.reset(new CDC::LinearGlobalADCCountTranslator);

  if (m_param_flightTimeEstimation != std::string("")) {
    try {
      m_flightTimeEstimation = getPreferredDirection(m_param_flightTimeEstimation);
    } catch (std::invalid_argument& e) {
      B2ERROR("Unexpected 'flightTimeEstimation' parameter : '" << m_param_flightTimeEstimation);
    }
  }

  m_triggerPoint = Vector3D(std::get<0>(m_param_triggerPoint),
                            std::get<1>(m_param_triggerPoint),
                            std::get<2>(m_param_triggerPoint));

  if (m_flightTimeEstimation == EPreferredDirection::c_Symmetric) {
    B2ERROR("Unexpected 'flightTimeEstimation' parameter : '" << m_param_flightTimeEstimation);
  } else if (m_flightTimeEstimation == EPreferredDirection::c_Downwards) {
    FlightTimeEstimator::instance(std::make_unique<CosmicRayFlightTimeEstimator>(m_triggerPoint));
  } else if (m_flightTimeEstimation == EPreferredDirection::c_Outwards) {
    FlightTimeEstimator::instance(std::make_unique<BeamEventFlightTimeEstimator>());
  }

  int nSL = m_param_maxDriftTimes.size();
  for (int iSL = 0; iSL < nSL; ++iSL) {
    m_maxDriftTimes.at(iSL) = m_param_maxDriftTimes.at(iSL);
  }

  if (not m_param_useSuperLayers.empty()) {
    for (const ISuperLayer iSL : m_param_useSuperLayers) {
      m_useSuperLayers.at(iSL) = true;
    }
  } else {
    m_useSuperLayers.fill(true);
  }

  // Check for common value in the two vectors for using / ignoring layers
  for (const uint useLayer : m_param_useLayers) {
    for (const uint ingoreLayer : m_param_ignoreLayers) {
      if (useLayer == ingoreLayer) {
        B2FATAL("You chose to use and ignore CDC layer " << useLayer << " at the same time. "
                "Please decide to either use or to ignore the layer.");
      }
    }
  }

  // fill all layers that should be used
  if (not m_param_useLayers.empty()) {
    for (const uint layer : m_param_useLayers) {
      m_useLayers.at(layer) = true;
    }
  } else {
    m_useLayers.fill(true);
  }
  // set layers that should be ignored to false
  if (not m_param_ignoreLayers.empty()) {
    for (const uint layer : m_param_ignoreLayers) {
      m_useLayers.at(layer) = false;
    }
  }

  if (std::isfinite(std::get<0>(m_param_useDegreeSector))) {
    m_useSector[0] = Vector2D::Phi(std::get<0>(m_param_useDegreeSector) * Unit::deg);
    m_useSector[1] = Vector2D::Phi(std::get<1>(m_param_useDegreeSector) * Unit::deg);
  }

  Super::initialize();
}

void WireHitCreator::beginRun()
{
  Super::beginRun();
  CDCWireTopology& wireTopology = CDCWireTopology::getInstance();
  wireTopology.reinitialize(m_wirePosition, m_param_ignoreWireSag);
}

void WireHitCreator::apply(std::vector<CDCWireHit>& outputWireHits)
{
  // Wire hits have been created before
  if (not outputWireHits.empty()) return;

  const CDCWireTopology& wireTopology = CDCWireTopology::getInstance();
  CDC::TDCCountTranslatorBase& tdcCountTranslator = *m_tdcCountTranslator;
  CDC::ADCCountTranslatorBase& adcCountTranslator = *m_adcCountTranslator;
  CDC::CDCGeometryPar& geometryPar = CDC::CDCGeometryPar::Instance();

  // Create the wire hits into a vector
  StoreArray<CDCHit> hits;
  std::size_t nHits = hits.getEntries();
  if (nHits == 0) {
    B2WARNING("Event with no hits");
    outputWireHits.clear();
  }

  std::map<int, size_t> nHitsByMCParticleId;

  outputWireHits.reserve(nHits);

  // make sure that DB object for time cut is valid:
  if (not m_DBCDClayerTimeCut.isValid()) {
    B2FATAL("CDClayerTimeCut DB object is invalid");
  }

  for (const CDCHit& hit : hits) {

    if (hit.getICLayer() < geometryPar.getOffsetOfFirstLayer() or
        hit.getISuperLayer() < geometryPar.getOffsetOfFirstSuperLayer()) {
      continue;
    }

    // ignore this hit if it contains the information of a 2nd hit
    if (!m_param_useSecondHits && hit.is2ndHit()) {
      continue;
    }

    // Only use some MCParticles if request - mostly for debug
    if (not m_param_useMCParticleIds.empty()) {
      MCParticle* mcParticle = hit.getRelated<MCParticle>();
      int mcParticleId = mcParticle->getArrayIndex();
      if (mcParticle) {
        nHitsByMCParticleId[mcParticleId]++;
      }
      bool useMCParticleId = std::count(m_param_useMCParticleIds.begin(),
                                        m_param_useMCParticleIds.end(),
                                        mcParticleId);
      if (not useMCParticleId) continue;
    }

    WireID wireID(hit.getID());
    if (not wireTopology.isValidWireID(wireID)) {
      B2WARNING("Skip invalid wire id " << hit.getID());
      continue;
    }

    // ignore hit if it is on a bad wire
    if (not m_param_useBadWires and geometryPar.isBadWire(wireID)) {
      continue;
    }

    // Exclude hit with large drift time
    // In the following, the tof correction is off; the propagation-delay corr. is off (default of the translator); the event time corr. is on when it is available.
    const double approxDriftTime = tdcCountTranslator.getDriftTime(hit.getTDCCount(), wireID, 0, 0, hit.getADCCount());
    ISuperLayer iSL = wireID.getISuperLayer();

    // If input module parameter is set to positive value, use it. Otherwise use DB
    if (m_maxDriftTimes.at(iSL) > 0) {
      if (approxDriftTime > m_maxDriftTimes.at(iSL)) continue;
    } else {
      if (m_DBCDClayerTimeCut->getLayerTimeCut(iSL) > 0 && approxDriftTime > m_DBCDClayerTimeCut->getLayerTimeCut(iSL)) continue;
    }

    if (not m_useSuperLayers[iSL]) continue;
    unsigned short layer = wireID.getICLayer();
    if (not m_useLayers[layer]) continue;

    const CDCWire& wire = wireTopology.getWire(wireID);

    const Vector2D& pos2D = wire.getRefPos2D();

    // Check whether the position is outside the selected sector
    if (pos2D.isBetween(m_useSector[1], m_useSector[0])) continue;

    // Only use some MCParticles if request - mostly for debug
    if (not m_param_useMCParticleIds.empty()) {
      MCParticle* mcParticle = hit.getRelated<MCParticle>();
      int mcParticleId = mcParticle->getArrayIndex();
      if (mcParticle) {
        nHitsByMCParticleId[mcParticleId]++;
      }
      bool useMCParticleId = std::count(m_param_useMCParticleIds.begin(),
                                        m_param_useMCParticleIds.end(),
                                        mcParticleId);
      if (not useMCParticleId) continue;
    }



    // Consider the particle as incoming in the top part of the CDC for a downwards flight direction
    bool isIncoming = m_flightTimeEstimation == EPreferredDirection::c_Downwards and pos2D.y() > 0;
    const double alpha = isIncoming ?  M_PI : 0;
    const double beta = 1;
    const double flightTimeEstimate =
      FlightTimeEstimator::instance().getFlightTime2D(pos2D, alpha, beta);

    const double driftTime =  tdcCountTranslator.getDriftTime(hit.getTDCCount(),
                                                              wire.getWireID(),
                                                              flightTimeEstimate,
                                                              wire.getRefZ(),
                                                              hit.getADCCount());
    const bool left = false;
    const bool right = true;
    const double theta = M_PI / 2;

    const double leftRefDriftLength =
      tdcCountTranslator.getDriftLength(hit.getTDCCount(),
                                        wire.getWireID(),
                                        flightTimeEstimate,
                                        left,
                                        wire.getRefZ(),
                                        alpha,
                                        theta);

    const double rightRefDriftLength =
      tdcCountTranslator.getDriftLength(hit.getTDCCount(),
                                        wire.getWireID(),
                                        flightTimeEstimate,
                                        right,
                                        wire.getRefZ(),
                                        alpha,
                                        theta);

    const double refDriftLength =
      (leftRefDriftLength + rightRefDriftLength) / 2.0;

    const double leftRefDriftLengthVariance =
      tdcCountTranslator.getDriftLengthResolution(refDriftLength,
                                                  wire.getWireID(),
                                                  left,
                                                  wire.getRefZ(),
                                                  alpha,
                                                  theta);

    const double rightRefDriftLengthVariance =
      tdcCountTranslator.getDriftLengthResolution(refDriftLength,
                                                  wire.getWireID(),
                                                  right,
                                                  wire.getRefZ(),
                                                  alpha,
                                                  theta);

    const double refDriftLengthVariance =
      (leftRefDriftLengthVariance + rightRefDriftLengthVariance) / 2.0;

    const double leftRefChargeDeposit =
      adcCountTranslator.getCharge(hit.getADCCount(),
                                   wire.getWireID(),
                                   left,
                                   wire.getRefZ(),
                                   theta);

    const double rightRefChargeDeposit =
      adcCountTranslator.getCharge(hit.getADCCount(),
                                   wire.getWireID(),
                                   right,
                                   wire.getRefZ(),
                                   theta);

    const double refChargeDeposit =
      (leftRefChargeDeposit + rightRefChargeDeposit) / 2.0;

    outputWireHits.emplace_back(&hit, refDriftLength, refDriftLengthVariance, refChargeDeposit, driftTime);
  }

  std::sort(outputWireHits.begin(), outputWireHits.end());

  if (not m_param_useMCParticleIds.empty()) {
    for (const std::pair<int, size_t> nHitsForMCParticleId : nHitsByMCParticleId) {
      B2DEBUG(25,
              "MC particle " << nHitsForMCParticleId.first << " #hits "
              << nHitsForMCParticleId.second);
    }
  }
}

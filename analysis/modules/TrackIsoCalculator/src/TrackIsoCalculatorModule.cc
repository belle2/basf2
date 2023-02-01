/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/modules/TrackIsoCalculator/TrackIsoCalculatorModule.h>
#include <analysis/DecayDescriptor/DecayDescriptorParticle.h>

#include <cmath>
#include <iomanip>
#include <boost/algorithm/string.hpp>


using namespace Belle2;

REG_MODULE(TrackIsoCalculator);

TrackIsoCalculatorModule::TrackIsoCalculatorModule() : Module()
{
  // Set module properties
  setDescription(
    R"DOC(Calculate track isolation variables on the charged stable particles, or selected charged daughters, of the input ParticleList.)DOC");

  // Parameter definitions
  addParam("decayString",
           m_decayString,
           "The name of the input charged stable particle list, e.g. ``mu+:all``, or a composite particle w/ charged stable daughters for which distances are to be calculated, e.g. ``D0 -> ^K- pi+``. Note that in the latter case we allow only one daughter to be selected in the decay string per module instance.");
  addParam("particleListReference",
           m_pListReferenceName,
           "The name of the input ParticleList of reference tracks. Must be a charged stable particle as defined in Const::chargedStableSet.");
  addParam("detectorName",
           m_detName,
           "The name of the detector at whose (cylindrical) surface(s) we extrapolate each helix's polar and azimuthal angle. Allowed values: {CDC, TOP, ARICH, ECL, KLM}.");
  addParam("useHighestProbMassForExt",
           m_useHighestProbMassForExt,
           "If this option is set, the helix extrapolation for the target and reference particles will use the track fit result for the most probable mass hypothesis, namely, the one that gives the highest chi2Prob of the fit.",
           bool(false));
  addParam("payloadName",
           m_payloadName,
           "The name of the database payload object with the PID detector weights.",
           std::string("PIDDetectorWeights"));
  addParam("excludePIDDetWeights",
           m_excludePIDDetWeights,
           "If set to true, will not use the PID detector weights for the score definition.",
           bool(false));
}

TrackIsoCalculatorModule::~TrackIsoCalculatorModule()
{
}

void TrackIsoCalculatorModule::initialize()
{
  m_event_metadata.isRequired();

  if (!m_excludePIDDetWeights) {
    m_DBWeights = std::make_unique<DBObjPtr<PIDDetectorWeights>>(m_payloadName);
  }

  bool valid = m_decaydescriptor.init(m_decayString);
  if (!valid) {
    B2ERROR("Decay string " << m_decayString << " is invalid.");
  }

  const DecayDescriptorParticle* mother = m_decaydescriptor.getMother();

  m_pListTarget.isRequired(mother->getFullName());
  m_nSelectedDaughters = m_decaydescriptor.getSelectionNames().size();

  if (m_nSelectedDaughters > 1) {
    B2ERROR("More than one daughter is selected in the decay string " << m_decayString << ".");
  }

  m_pListReference.isRequired(m_pListReferenceName);

  B2INFO("Calculating track-based isolation variables at "
         << m_detName
         << " surface(s) for the decay string: "
         << m_decayString
         << " using the reference ParticleList: "
         << m_pListReferenceName
         << ".");

  if (!onlySelectedStdChargedInDecay()) {
    B2ERROR("Selected ParticleList in decay string:"
            << m_decayString
            << " and/or reference ParticleList: "
            << m_pListReferenceName
            << " is not that of a valid particle in Const::chargedStableSet!");
  }

  if (m_useHighestProbMassForExt) {
    B2INFO("Will use track fit result for the most probable mass hypothesis in helix extrapolation.");
  }

  // Ensure we use uppercase detector labels.
  boost::to_upper(m_detName);

  // The total number of layers of this detector.
  m_nLayers = m_detToLayers[m_detName].size();

  // Define the name(s) of the variables for this detector to be stored as extraInfo.
  for (const auto& iLayer : m_detToLayers[m_detName]) {

    auto iDetLayer = m_detName + std::to_string(iLayer);

    auto distVarName = "distToClosestTrkAt" + iDetLayer + "_VS_" + m_pListReferenceName;
    if (m_useHighestProbMassForExt) {
      distVarName += "__useHighestProbMassForExt";
    }
    auto refPartIdxVarName =  "idxOfClosestPartAt" + iDetLayer + "In_" + m_pListReferenceName;

    m_detLayerToDistVariable.insert(std::make_pair(iDetLayer, distVarName));
    m_detLayerToRefPartIdxVariable.insert(std::make_pair(iDetLayer, refPartIdxVarName));

  }

  // Isolation score variable per detector.
  m_isoScoreVariable = "trkIsoScore" + m_detName + "_VS_" + m_pListReferenceName;
  if (m_useHighestProbMassForExt) {
    m_isoScoreVariable += "__useHighestProbMassForExt";
  }

  // PID weight variable per detector.
  m_detPIDWeightName = "detPIDWeight_" + m_detName;

  // Weighted sum of (scaled) inverse distances variable per detector.
  m_weightedSumInvDistsVariable = "weightedSumInvDists" + m_detName + "_VS_" + m_pListReferenceName;
  if (m_useHighestProbMassForExt) {
    m_weightedSumInvDistsVariable += "__useHighestProbMassForExt";
  }

}

void TrackIsoCalculatorModule::event()
{
  B2DEBUG(11, "Start processing EVENT: " << m_event_metadata->getEvent());

  const auto nMotherParticles = m_pListTarget->getListSize();

  // Fill transient container of all selected charged particles in the decay
  // for which the distance to nearest neighbour is to be calculated.
  // If the input ParticleList is that of standard charged particles, just copy
  // the whole list over, otherwise loop over the selected charged daughters.
  std::unordered_map<unsigned int, const Particle*> targetParticles;
  targetParticles.reserve(nMotherParticles);

  // Loop over the layer of the input detector.
  for (const auto& iLayer : m_detToLayers[m_detName]) {

    auto iDetLayer = m_detName + std::to_string(iLayer);

    for (unsigned int iPart(0); iPart < nMotherParticles; ++iPart) {

      auto iParticle = m_pListTarget->getParticle(iPart);

      if (m_nSelectedDaughters) {
        for (auto* iDaughter : m_decaydescriptor.getSelectionParticles(iParticle)) {
          // Check if the distance for this target particle has been set already,
          // e.g. by a previous instance of this module.
          if (iDaughter->hasExtraInfo(m_detLayerToDistVariable[iDetLayer])) {
            continue;
          }
          targetParticles.insert({iDaughter->getMdstArrayIndex(), iDaughter});
        }
      } else {
        if (iParticle->hasExtraInfo(m_detLayerToDistVariable[iDetLayer])) {
          continue;
        }
        targetParticles.insert({iParticle->getMdstArrayIndex(), iParticle});
      }

    }
  }

  const auto nParticlesTarget = targetParticles.size();
  const auto nParticlesReference = m_pListReference->getListSize();

  // Loop over the layer of the input detector.
  for (const auto& iLayer : m_detToLayers[m_detName]) {

    auto iDetLayer = m_detName + std::to_string(iLayer);

    B2DEBUG(11, "\n"
            << "Detector surface: " << iDetLayer << "\n"
            << "nMotherParticles: " << nMotherParticles << "\n"
            << "nParticlesTarget: " << nParticlesTarget << "\n"
            << "nParticlesReference: " << nParticlesReference);

    double dummyDist(-1.0);

    // Store the pair-wise distances in a map,
    // where the keys are pairs of mdst indexes.
    std::map<std::pair<unsigned int, unsigned int>, double> particleMdstIdxPairsToDist;

    // Loop over input particle list
    for (const auto& targetParticle : targetParticles) {

      auto iMdstIdx = targetParticle.first;
      auto iParticle = targetParticle.second;

      for (unsigned int jPart(0); jPart < nParticlesReference; ++jPart) {

        auto jParticle = m_pListReference->getParticle(jPart);
        auto jMdstIdx = jParticle->getMdstArrayIndex();

        auto partMdstIdxPair = std::make_pair(iMdstIdx, jMdstIdx);

        // Set dummy distance if same particle.
        if (iMdstIdx == jMdstIdx) {
          particleMdstIdxPairsToDist[partMdstIdxPair] = dummyDist;
          continue;
        }
        // If:
        //
        // - the mass hypothesis of the best fit is used, OR
        // - the mass hypothesis of the 'default' fit of the two particles is the same,
        //
        // avoid re-doing the calculation if a pair with the flipped mdst indexes in the map already exists.
        if (m_useHighestProbMassForExt || (iParticle->getPDGCodeUsedForFit() == jParticle->getPDGCodeUsedForFit())) {
          if (particleMdstIdxPairsToDist.count({jMdstIdx, iMdstIdx})) {
            particleMdstIdxPairsToDist[partMdstIdxPair] = particleMdstIdxPairsToDist[ {jMdstIdx, iMdstIdx}];
            continue;
          }
        }
        // Calculate the pair-wise distance.
        particleMdstIdxPairsToDist[partMdstIdxPair] = this->getDistAtDetSurface(iParticle, jParticle, iDetLayer);
      }

    }

    // For each particle in the input list, find the minimum among all distances to the reference particles.
    for (const auto& targetParticle : targetParticles) {

      auto iMdstIdx = targetParticle.first;
      auto iParticle = targetParticle.second;

      // Save the distances and the mdst indexes of the reference particles.
      std::vector<std::pair<double, unsigned int>> iDistancesAndRefMdstIdxs;
      for (const auto& [mdstIdxs, dist] : particleMdstIdxPairsToDist) {
        if (mdstIdxs.first == iMdstIdx) {
          if (!std::isnan(dist) && dist >= 0) {
            iDistancesAndRefMdstIdxs.push_back(std::make_pair(dist, mdstIdxs.second));
          }
        }
      }

      if (!iDistancesAndRefMdstIdxs.size()) {
        B2DEBUG(12, "The container of distances is empty. Perhaps the target and reference lists contain the same exact particles?");
        continue;
      }

      const auto minDist = *std::min_element(std::begin(iDistancesAndRefMdstIdxs), std::end(iDistancesAndRefMdstIdxs),
      [](const auto & l, const auto & r) {return l.first < r.first;});

      auto jParticle = m_pListReference->getParticleWithMdstIdx(minDist.second);

      B2DEBUG(11, "\n"
              << "Particle w/ mdstIndex[" << iMdstIdx << "] (PDG = "
              << iParticle->getPDGCode() << "). Closest charged particle w/ mdstIndex["
              << minDist.second
              << "] (PDG = " << jParticle->getPDGCode()
              << ") at " << iDetLayer
              << " surface is found at D = " << minDist.first
              << " [cm]\n"
              << "Storing extraInfo variables:\n"
              << m_detLayerToDistVariable[iDetLayer]
              << "\n"
              << m_detLayerToRefPartIdxVariable[iDetLayer]);

      if (!iParticle->hasExtraInfo(m_detLayerToDistVariable[iDetLayer])) {
        m_particles[iParticle->getArrayIndex()]->addExtraInfo(m_detLayerToDistVariable[iDetLayer], minDist.first);
      }
      m_particles[iParticle->getArrayIndex()]->writeExtraInfo(m_detLayerToRefPartIdxVariable[iDetLayer], minDist.second);

    } // end loop over input particle list.

  } // end loop over detector layers.

  // Store the isolation score per target particle for the given detector.
  for (const auto& targetParticle : targetParticles) {

    auto iMdstIdx = targetParticle.first;
    auto iParticle = targetParticle.second;

    // Get the sum of inverse minmimum distances (scaled by the threshold distance) over the layers of this detector,
    // weighted by the PID detector separation score (if requested).
    auto weightedSumInvDists = this->getWeightedSumInvDists(iParticle);
    m_particles[iParticle->getArrayIndex()]->writeExtraInfo(m_weightedSumInvDistsVariable, weightedSumInvDists);

    auto score = this->getIsoScore(iParticle);
    m_particles[iParticle->getArrayIndex()]->writeExtraInfo(m_isoScoreVariable, score);

    B2DEBUG(11, "\n"
            << "Particle w/ mdstIndex[" << iMdstIdx << "] (PDG = " << iParticle->getPDGCode() << ").\n"
            << "Isolation score in the " << m_detName << ": s = " << score
            << "\n"
            << "Storing extraInfo variable:\n"
            << m_isoScoreVariable);

  }

  B2DEBUG(11, "Finished processing EVENT: " << m_event_metadata->getEvent());

}


double TrackIsoCalculatorModule::getDetectorWeight(const Particle* iParticle) const
{

  double detWeight(-1.0);

  if (!iParticle->hasExtraInfo(m_detPIDWeightName)) {

    if (!m_excludePIDDetWeights) {

      auto hypo = Const::ChargedStable(std::abs(iParticle->getPDGCode()));
      auto p = iParticle->getP();
      auto theta = std::get<double>(Variable::Manager::Instance().getVariable("theta")->function(iParticle));
      auto det = this->getDetEnum(m_detName);

      detWeight = (*m_DBWeights.get())->getWeight(hypo, det, p, theta);
      // If w > 0, the detector has detrimental impact on PID:
      // the value is reset to zero to prevent the detector from contributing to the score.
      // NB: NaN should stay NaN.
      detWeight = (detWeight < 0 || std::isnan(detWeight)) ? detWeight : 0.0;
    }

    m_particles[iParticle->getArrayIndex()]->addExtraInfo(m_detPIDWeightName, detWeight);

  } else {

    detWeight = iParticle->getExtraInfo(m_detPIDWeightName);

  }

  return detWeight;

}


double TrackIsoCalculatorModule::getWeightedSumInvDists(const Particle* iParticle) const
{

  auto det = this->getDetEnum(m_detName);

  double detWeight = this->getDetectorWeight(iParticle);

  double sumInvDists(0.);
  for (const auto& iLayer : m_detToLayers.at(m_detName)) {

    auto iDetLayer = m_detName + std::to_string(iLayer);
    auto distVar = m_detLayerToDistVariable.at(iDetLayer);
    auto threshDist = this->getDistThreshold(det, iLayer);

    if (iParticle->hasExtraInfo(distVar)) {
      sumInvDists += threshDist / iParticle->getExtraInfo(distVar);
    }

  }

  return detWeight * sumInvDists;

}


double TrackIsoCalculatorModule::getIsoScore(const Particle* iParticle) const
{

  auto det = this->getDetEnum(m_detName);

  double detWeight = this->getDetectorWeight(iParticle);

  unsigned int n(0);
  for (const auto& iLayer : m_detToLayers.at(m_detName)) {

    auto iDetLayer = m_detName + std::to_string(iLayer);
    auto distVar = m_detLayerToDistVariable.at(iDetLayer);
    auto threshDist = this->getDistThreshold(det, iLayer);

    if (iParticle->hasExtraInfo(distVar)) {
      if (iParticle->getExtraInfo(distVar) < threshDist) {
        n++;
      }
    }

  }

  if (!n) {
    B2DEBUG(12, "\nNo close-enough neighbours to this particle in the " << m_detName << " were found.");
  }

  if (n > m_nLayers) {
    B2FATAL("\nTotal layers in " << m_detName << ": N=" << m_nLayers
            << "\n"
            << "Layers w/ a close-enough particle: n=" << n
            << "\n"
            << "n > N ?? Abort...");
  }

  return 1. - (-detWeight * (n / m_nLayers));

}


double TrackIsoCalculatorModule::getDistAtDetSurface(const Particle* iParticle,
                                                     const Particle* jParticle,
                                                     const std::string& detLayerName) const
{

  // Radius and z boundaries of the cylinder describing this detector's surface.
  const auto rho = m_detLayerToSurfBoundaries.at(detLayerName).m_rho;
  const auto zfwd = m_detLayerToSurfBoundaries.at(detLayerName).m_zfwd;
  const auto zbwd = m_detLayerToSurfBoundaries.at(detLayerName).m_zbwd;
  // Polar angle boundaries between barrel and endcaps.
  const auto th_fwd = m_detLayerToSurfBoundaries.at(detLayerName).m_th_fwd;
  const auto th_fwd_brl = m_detLayerToSurfBoundaries.at(detLayerName).m_th_fwd_brl;
  const auto th_bwd_brl = m_detLayerToSurfBoundaries.at(detLayerName).m_th_bwd_brl;
  const auto th_bwd = m_detLayerToSurfBoundaries.at(detLayerName).m_th_bwd;

  std::string nameExtTheta = "helixExtTheta(" + std::to_string(rho) + "," + std::to_string(zfwd) + "," + std::to_string(zbwd) + ")";
  if (m_useHighestProbMassForExt) {
    nameExtTheta.replace(nameExtTheta.size() - 1, 1, ", 1)");
  }
  const auto iExtTheta = std::get<double>(Variable::Manager::Instance().getVariable(nameExtTheta)->function(iParticle));
  const auto jExtTheta = std::get<double>(Variable::Manager::Instance().getVariable(nameExtTheta)->function(jParticle));

  std::string nameExtPhi = "helixExtPhi(" + std::to_string(rho) + "," + std::to_string(zfwd) + "," + std::to_string(zbwd) + ")";
  if (m_useHighestProbMassForExt) {
    nameExtPhi.replace(nameExtPhi.size() - 1, 1, ", 1)");
  }
  const auto iExtPhi = std::get<double>(Variable::Manager::Instance().getVariable(nameExtPhi)->function(iParticle));
  const auto jExtPhi = std::get<double>(Variable::Manager::Instance().getVariable(nameExtPhi)->function(jParticle));

  const auto iExtInBarrel = (iExtTheta >= th_fwd_brl && iExtTheta < th_bwd_brl);
  const auto jExtInBarrel = (jExtTheta >= th_fwd_brl && jExtTheta < th_bwd_brl);

  const auto iExtInFWDEndcap = (iExtTheta >= th_fwd && iExtTheta < th_fwd_brl);
  const auto jExtInFWDEndcap = (jExtTheta >= th_fwd && jExtTheta < th_fwd_brl);

  const auto iExtInBWDEndcap = (iExtTheta >= th_bwd_brl && iExtTheta < th_bwd);
  const auto jExtInBWDEndcap = (jExtTheta >= th_bwd_brl && jExtTheta < th_bwd);

  if (m_detName == "CDC" || m_detName == "TOP") {

    // If any of the two extrapolated tracks is not in the barrel region of the CDC/TOP, the distance is undefined.
    if (!iExtInBarrel || !jExtInBarrel) {
      return std::numeric_limits<double>::quiet_NaN();
    }

    // For CDC and TOP, we calculate the distance between the points where the two input
    // extrapolated track helices cross the input detector's cylindrical surface
    // on the (rho, phi) plane. Namely, this is the cord length of the arc
    // that subtends deltaPhi.
    auto diffPhi = jExtPhi - iExtPhi;
    if (std::abs(diffPhi) > M_PI) {
      diffPhi = (diffPhi > M_PI) ? diffPhi - 2 * M_PI :  2 * M_PI + diffPhi;
    }

    return 2.0 * rho * sin(std::abs(diffPhi) / 2.0);

  } else {

    if (m_detName == "ARICH") {
      // If any of the two tracks is not in the ARICH theta acceptance, the distance is undefined.
      if (!iExtInFWDEndcap || !jExtInFWDEndcap) {
        return std::numeric_limits<double>::quiet_NaN();
      }
    }
    if (m_detName == "ECL" || m_detName == "KLM") {

      // For ECL and KLM, we require track pairs to be both in the barrel,
      // both in the FWD endcap, or both in the FWD endcap. Otherwise, the distance is undefined.
      if (
        !(iExtInBarrel && jExtInBarrel) &&
        !(iExtInFWDEndcap && jExtInFWDEndcap) &&
        !(iExtInBWDEndcap && jExtInBWDEndcap)
      ) {
        return std::numeric_limits<double>::quiet_NaN();
      }
    }

    // Ok, we know theta and phi.
    // Let's extract (spherical) R by using the transformation:
    //
    // 1. rho = r * sin(theta)
    // 2. phi = phi
    // 3. z = r * cos(theta)
    //
    // The formula to be inverted depends on where each extrapolated track's theta is found:
    // if in barrel, use 1. (rho is known), if in fwd/bwd endcap, use 3. (zfwd/zbwd is known).

    const auto iExtR = (iExtTheta >= th_fwd_brl && iExtTheta < th_bwd_brl) ? rho / sin(iExtTheta) : ((iExtTheta >= th_fwd
                       && iExtTheta < th_fwd_brl) ? zfwd / cos(iExtTheta) : zbwd / cos(iExtTheta));
    const auto jExtR = (jExtTheta >= th_fwd_brl && jExtTheta < th_bwd_brl) ? rho / sin(jExtTheta) : ((jExtTheta >= th_fwd
                       && jExtTheta < th_fwd_brl) ? zfwd / cos(jExtTheta) : zbwd / cos(jExtTheta));

    return sqrt((iExtR * iExtR) + (jExtR * jExtR) - 2 * iExtR * jExtR * (sin(iExtTheta) * sin(jExtTheta) * cos(iExtPhi - jExtPhi)
                + cos(iExtTheta) * cos(jExtTheta)));

  }

  return std::numeric_limits<double>::quiet_NaN();

}

bool TrackIsoCalculatorModule::onlySelectedStdChargedInDecay()
{

  bool checkPList = false;

  if (!m_nSelectedDaughters) {
    checkPList = Const::chargedStableSet.find(abs(m_decaydescriptor.getMother()->getPDGCode())) != Const::invalidParticle;
  } else {
    for (int pdgcode : m_decaydescriptor.getSelectionPDGCodes()) {
      checkPList = Const::chargedStableSet.find(abs(pdgcode)) != Const::invalidParticle;
      if (!checkPList) {
        break;
      }
    }
  }

  DecayDescriptor dd;
  bool checkPListReference = false;
  if (dd.init(m_pListReferenceName)) {
    checkPListReference = Const::chargedStableSet.find(abs(dd.getMother()->getPDGCode())) != Const::invalidParticle;
  }

  return (checkPList and checkPListReference);

};

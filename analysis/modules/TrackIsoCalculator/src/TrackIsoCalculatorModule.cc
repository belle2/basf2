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
  addParam("detectorSurface",
           m_detSurface,
           "The name of the detector at whose (cylindrical) surface(s) we extrapolate each helix's polar and azimuthal angle. Allowed values: {CDC, TOP, ARICH, ECL, KLM}.");
  addParam("useHighestProbMassForExt",
           m_useHighestProbMassForExt,
           "If this option is set, the helix extrapolation for the target and reference particles will use the track fit result for the most probable mass hypothesis, namely, the one that gives the highest chi2Prob of the fit.",
           bool(false));
}

TrackIsoCalculatorModule::~TrackIsoCalculatorModule()
{
}

void TrackIsoCalculatorModule::initialize()
{
  m_event_metadata.isRequired();

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

  B2INFO("Calculating track-based isolation variables at " << m_detSurface << " surface for the decay string "
         << m_decayString << " using the reference ParticleList " << m_pListReferenceName << ".");

  if (!onlySelectedStdChargedInDecay()) {
    B2ERROR("Selected ParticleList in decay string " << m_decayString << " and/or reference ParticleList " << m_pListReferenceName <<
            " is not that of a valid particle in Const::chargedStableSet!");
  }

  if (m_useHighestProbMassForExt) {
    B2INFO("Will use track fit result for the most probable mass hypothesis in helix extrapolation.");
  }

  // Define the name of the variable to be stored as extraInfo.
  m_extraInfoName = "distToClosestTrkAt" + m_detSurface + "_VS_" + m_pListReferenceName;
  if (m_useHighestProbMassForExt) {
    m_extraInfoName += "__useHighestProbMassForExt";
  }

  m_isSurfaceInDet.insert({"CDC", m_detSurface.find("CDC") != std::string::npos});
  m_isSurfaceInDet.insert({"TOP", m_detSurface.find("TOP") != std::string::npos});
  m_isSurfaceInDet.insert({"ARICH", m_detSurface.find("ARICH") != std::string::npos});
  m_isSurfaceInDet.insert({"ECL", m_detSurface.find("ECL") != std::string::npos});
  m_isSurfaceInDet.insert({"KLM", m_detSurface.find("KLM") != std::string::npos});
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

  for (unsigned int iPart(0); iPart < nMotherParticles; ++iPart) {

    auto iParticle = m_pListTarget->getParticle(iPart);

    if (m_nSelectedDaughters) {
      for (auto* iDaughter : m_decaydescriptor.getSelectionParticles(iParticle)) {
        // Check if the distance for this target particle has been set already,
        // e.g. by a previous instance of this module.
        if (iDaughter->hasExtraInfo(m_extraInfoName)) {
          continue;
        }
        targetParticles.insert({iDaughter->getMdstArrayIndex(), iDaughter});
      }
    } else {
      if (iParticle->hasExtraInfo(m_extraInfoName)) {
        continue;
      }
      targetParticles.insert({iParticle->getMdstArrayIndex(), iParticle});
    }
  }

  const auto nParticlesTarget = targetParticles.size();
  const auto nParticlesReference = m_pListReference->getListSize();

  B2DEBUG(11, "Detector surface: " << m_detSurface << "\n"
          << "nMotherParticles: " << nMotherParticles << "\n"
          << "nParticlesTarget: " << nParticlesTarget << "\n"
          << "nParticlesReference: " << nParticlesReference);

  double dummyDist(-1.0);

  // Store the pair-wise distances in a map,
  // where the keys are pairs of mdst indexes.
  std::map<std::pair<unsigned int, unsigned int>, double> particleMdstIdxPairsToDist;

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
      particleMdstIdxPairsToDist[partMdstIdxPair] = this->getDistAtDetSurface(iParticle, jParticle);
    }
  }

  // For each particle in the input list, find the minimum among all distances to the reference particles.
  for (const auto& targetParticle : targetParticles) {
    auto iMdstIdx = targetParticle.first;
    auto iParticle = targetParticle.second;

    std::vector<double> iDistances;
    for (const auto& [mdstIdxs, dist] : particleMdstIdxPairsToDist) {
      if (mdstIdxs.first == iMdstIdx) {
        if (!std::isnan(dist) && dist >= 0) {
          iDistances.push_back(dist);
        }
      }
    }

    if (!iDistances.size()) {
      continue;
    }

    const auto minDist = *std::min_element(std::begin(iDistances), std::end(iDistances));

    B2DEBUG(10, "Particle w/ mdstIndex[" << iMdstIdx << "] (PDG = "
            << iParticle->getPDGCode() << "). Closest charged partner at "
            << m_detSurface << " surface is found at D = " << minDist << " [cm]"
            << "\n"
            << "Storing ExtraInfo w/ name: " << m_extraInfoName);

    m_particles[iParticle->getArrayIndex()]->addExtraInfo(m_extraInfoName, minDist);

  }

  B2DEBUG(11, "Finished processing EVENT: " << m_event_metadata->getEvent());
}

void TrackIsoCalculatorModule::terminate()
{
}

double TrackIsoCalculatorModule::getDistAtDetSurface(const Particle* iParticle, const Particle* jParticle)
{

  // Radius and z boundaries of the cylinder describing this detector's surface.
  const auto rho = m_detSurfBoundaries[m_detSurface].m_rho;
  const auto zfwd = m_detSurfBoundaries[m_detSurface].m_zfwd;
  const auto zbwd = m_detSurfBoundaries[m_detSurface].m_zbwd;
  // Polar angle boundaries between barrel and endcaps.
  const auto th_fwd = m_detSurfBoundaries[m_detSurface].m_th_fwd;
  const auto th_fwd_brl = m_detSurfBoundaries[m_detSurface].m_th_fwd_brl;
  const auto th_bwd_brl = m_detSurfBoundaries[m_detSurface].m_th_bwd_brl;
  const auto th_bwd = m_detSurfBoundaries[m_detSurface].m_th_bwd;

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

  if (m_isSurfaceInDet["CDC"] || m_isSurfaceInDet["TOP"]) {

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

    if (m_isSurfaceInDet["ARICH"]) {
      // If any of the two tracks is not in the ARICH theta acceptance, the distance is undefined.
      if (!iExtInFWDEndcap || !jExtInFWDEndcap) {
        return std::numeric_limits<double>::quiet_NaN();
      }
    }
    if (m_isSurfaceInDet["ECL"] || m_isSurfaceInDet["KLM"]) {

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

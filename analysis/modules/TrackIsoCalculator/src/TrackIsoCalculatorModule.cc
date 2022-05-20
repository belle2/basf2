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
  setDescription(R"DOC(Calculate track isolation variables on the input ParticleList.)DOC");

  // Parameter definitions
  addParam("particleList",
           m_decayString,
           "A decay string that has a full ParticleList name as mother particle. If no daughters selected, must be a charged stable particle as defined in Const::chargedStableSet.");
  addParam("particleListReference",
           m_pListReferenceName,
           "The name of the input ParticleList of reference tracks. By default, the :all list of the same particle type with m_pListName is used. "
           "Must be a charged stable particle as defined in Const::chargedStableSet.",
           std::string(""));
  addParam("detectorInnerSurface",
           m_detInnerSurface,
           "The name of the detector at whose innermost layer we extrapolate each helix's polar and azimuthal angle. Allowed values: {CDC, PID(=TOP/ARICH), ECL, KLM}.");
  addParam("use2DRhoPhiDist",
           m_use2DRhoPhiDist,
           "If true, will calculate the pair-wise track distance as the cord length on the (rho, phi) projection.",
           bool(false));
}

TrackIsoCalculatorModule::~TrackIsoCalculatorModule()
{
}

void TrackIsoCalculatorModule::initialize()
{

  m_event_metadata.isRequired();
  bool valid = m_decaydescriptor.init(m_decayString);
  if (!valid)
    B2ERROR("ParticleLoaderModule::initialize Invalid input DecayString: " << m_decayString);
  const DecayDescriptorParticle* mother = m_decaydescriptor.getMother();
  //int nProducts = m_decaydescriptor.getNDaughters();
  m_targetListName = mother->getFullName();
  m_targetList.isRequired(m_targetListName);
  m_nSelectedDaughters = m_decaydescriptor.getSelectionNames().size();

  m_pListReference.isRequired(m_pListReferenceName);

  B2INFO("TrackIsoCalculator module will calculate isolation variables for the ParticleList: " << m_targetListName << ", "
         << "Reference ParticleList: " << m_pListReferenceName << ".");

  m_extraInfoName = (!m_use2DRhoPhiDist) ? ("dist3DToClosestTrkAt" + m_detInnerSurface + "Surface") : ("dist2DRhoPhiToClosestTrkAt" +
                    m_detInnerSurface + "Surface");

  if (!isStdChargedList()) {
    B2FATAL("ParticleList: " << m_targetListName << " and/or ParticleList: " << m_pListReferenceName <<
            " is not that of a valid particle in Const::chargedStableSet! Aborting...");
  }

}

void TrackIsoCalculatorModule::event()
{

  const auto nParticles = m_targetList->getListSize();

  const auto nParticlesReference = m_pListReference->getListSize();
  const auto nTargetParticles = (m_nSelectedDaughters == 0) ? m_targetList->getListSize() : m_targetList->getListSize() *
                                m_nSelectedDaughters;
  B2DEBUG(11, "EVENT: " << m_event_metadata->getEvent() << "\n" << "nParticles: " << nParticles << "\n"
          << "nParticlesReference: " << nParticlesReference);
  // Fill temporary vector of particles in case of selected daughters:
  std::vector<const Particle*> targetParticles;
  if (m_nSelectedDaughters > 0) {
    for (unsigned int iPart(0); iPart < m_targetList->getListSize(); ++iPart) {
      auto* iParticle = m_targetList->getParticle(iPart);
      auto daughters = m_decaydescriptor.getSelectionParticles(iParticle);
      for (auto* iDaughter : daughters) {
        targetParticles.push_back(iDaughter);
      }
    }
  }
  // Store the pair-wise distances in a 2D array.
  // Size is given by the length of the reference particle list.
  std::vector<double> defaultDistances(nParticlesReference, 1e9);
  // Size is given by the length of the particle list. Each vector (= defaultDistances) has nParticlesReference components.
  // Thus, total size is given by nParticles times nParticlesReference.
  std::vector<std::vector<double>> pairwiseDistances(nTargetParticles, defaultDistances);

  B2DEBUG(11, "Array of pair-wise distances between tracks in particle list. Initial values:");
  this->printDistancesArr(pairwiseDistances, nTargetParticles, nParticlesReference);

  for (unsigned int iPart(0); iPart < nTargetParticles; ++iPart) {
    const Particle* iParticle = (m_nSelectedDaughters > 0) ? targetParticles[iPart] : m_targetList->getParticle(iPart);
    for (unsigned int jPart(0); jPart < nParticlesReference; ++jPart) {
      Particle* jParticle = m_pListReference->getParticle(jPart);

      if (iParticle->getMdstArrayIndex() == jParticle->getMdstArrayIndex())
        continue;
      if (pairwiseDistances[iPart][jPart] != 1e9)
        continue;

      // Calculate the pair-wise distance.
      double ijDist = (!m_use2DRhoPhiDist) ? this->get3DDistAtDetSurface(iParticle,
                      jParticle) : this->get2DRhoPhiDistAsChordLength(iParticle, jParticle);

      pairwiseDistances[iPart][jPart] = ijDist;
      if (m_nSelectedDaughters == 0) {
        int jPart_in_inputList = m_targetList->getIndex(jParticle);
        if (jPart_in_inputList != -1)
          pairwiseDistances[jPart_in_inputList][iPart] = ijDist;
      }
    }

  }

  B2DEBUG(11, "Array of pair-wise distances between tracks in particle list. Final values:");
  this->printDistancesArr(pairwiseDistances, nParticles, nParticlesReference);

  // For each particle index, find the index of the particle w/ minimal distance in the corresponding row of the 2D array.
  for (unsigned int iPart(0); iPart < nTargetParticles; ++iPart) {

    auto minDist = std::min_element(std::begin(pairwiseDistances[iPart]), std::end(pairwiseDistances[iPart]));
    auto jPart = std::distance(std::begin(pairwiseDistances[iPart]), minDist);

    const Particle* iParticle = (m_nSelectedDaughters > 0) ? targetParticles[iPart] : m_targetList->getParticle(iPart);
    B2DEBUG(10, m_extraInfoName << " = " << *minDist << " [cm] - Particle[" << iPart << "]'s closest partner at innermost " <<
            m_detInnerSurface << " surface is Particle[" << jPart << "]");

    if (!iParticle->hasExtraInfo(m_extraInfoName)) {
      B2DEBUG(10, "\tStoring extraInfo for Particle[" << iPart << "]...");
      m_particles[iParticle->getArrayIndex()]->writeExtraInfo(m_extraInfoName, *minDist);

    }
  }

}

void TrackIsoCalculatorModule::terminate()
{
}

double TrackIsoCalculatorModule::get3DDistAtDetSurface(const Particle* iParticle, const Particle* jParticle)
{

  // Radius and z boundaries of the cylinder describing this detector's inner surface.
  const auto rho = m_detSurfBoundaries[m_detInnerSurface].m_rho;
  const auto zfwd = m_detSurfBoundaries[m_detInnerSurface].m_zfwd;
  const auto zbwd = m_detSurfBoundaries[m_detInnerSurface].m_zbwd;

  std::string nameExtTheta = "helixExtTheta(" + std::to_string(rho) + "," + std::to_string(zfwd) + "," + std::to_string(zbwd) + ")";
  const auto iExtTheta = std::get<double>(Variable::Manager::Instance().getVariable(nameExtTheta)->function(iParticle));
  const auto jExtTheta = std::get<double>(Variable::Manager::Instance().getVariable(nameExtTheta)->function(jParticle));

  std::string nameExtPhi = "helixExtPhi(" + std::to_string(rho) + "," + std::to_string(zfwd) + "," + std::to_string(zbwd) + ")";
  const auto iExtPhi = std::get<double>(Variable::Manager::Instance().getVariable(nameExtPhi)->function(iParticle));
  const auto jExtPhi = std::get<double>(Variable::Manager::Instance().getVariable(nameExtPhi)->function(jParticle));

  // Ok, we know theta and phi.
  // Let's extract (spherical) R by using the transformation:
  //
  // 1. rho = r * sin(theta)
  // 2. phi = phi
  // 3. z = r * cos(theta)
  //
  // The formula to be inverted depends on where each extrapolated track's theta is found:
  // if in barrel, use 1. (rho is known), if in fwd/bwd endcap, use 3. (zfwd/zbwd is known).

  const auto th_fwd = m_detSurfBoundaries[m_detInnerSurface].m_th_fwd;
  const auto th_fwd_brl = m_detSurfBoundaries[m_detInnerSurface].m_th_fwd_brl;
  const auto th_bwd_brl = m_detSurfBoundaries[m_detInnerSurface].m_th_bwd_brl;

  const auto iExtR = (iExtTheta >= th_fwd_brl && iExtTheta < th_bwd_brl) ? rho / sin(iExtTheta) : ((iExtTheta >= th_fwd
                     && iExtTheta < th_fwd_brl) ? zfwd / cos(iExtTheta) : zbwd / cos(iExtTheta));
  const auto jExtR = (jExtTheta >= th_fwd_brl && jExtTheta < th_bwd_brl) ? rho / sin(jExtTheta) : ((jExtTheta >= th_fwd
                     && jExtTheta < th_fwd_brl) ? zfwd / cos(jExtTheta) : zbwd / cos(jExtTheta));

  // Now convert r, theta, phi to cartesian coordinates.
  // 1. x = r * sin(theta) * cos(phi)
  // 2. y = r * sin(theta) * sin(phi)
  // 3. z = r * cos(theta)

  const auto iExtX = iExtR * sin(iExtTheta) * cos(iExtPhi);
  const auto iExtY = iExtR * sin(iExtTheta) * sin(iExtPhi);
  const auto iExtZ = iExtR * cos(iExtTheta);

  const auto jExtX = jExtR * sin(jExtTheta) * cos(jExtPhi);
  const auto jExtY = jExtR * sin(jExtTheta) * sin(jExtPhi);
  const auto jExtZ = jExtR * cos(jExtTheta);

  // Get 3D distance via Pythagoras.
  return sqrt((iExtX - jExtX) * (iExtX - jExtX) + (iExtY - jExtY) * (iExtY - jExtY) + (iExtZ - jExtZ) * (iExtZ - jExtZ));
}


double TrackIsoCalculatorModule::get2DRhoPhiDistAsChordLength(const Particle* iParticle, const Particle* jParticle)
{

  // Radius and z boundaries of the cylinder describing this detector's inner surface.
  const auto rho = m_detSurfBoundaries[m_detInnerSurface].m_rho;
  const auto zfwd = m_detSurfBoundaries[m_detInnerSurface].m_zfwd;
  const auto zbwd = m_detSurfBoundaries[m_detInnerSurface].m_zbwd;

  std::string nameExtPhi = "helixExtPhi(" + std::to_string(rho) + "," + std::to_string(zfwd) + "," + std::to_string(zbwd) + ")";
  const auto iExtPhi = std::get<double>(Variable::Manager::Instance().getVariable(nameExtPhi)->function(iParticle));
  const auto jExtPhi = std::get<double>(Variable::Manager::Instance().getVariable(nameExtPhi)->function(jParticle));

  auto diffPhi = jExtPhi - iExtPhi;
  if (std::abs(diffPhi) > M_PI) {
    diffPhi = (diffPhi > M_PI) ? diffPhi - 2 * M_PI :  2 * M_PI + diffPhi;
  }

  return 2 * rho * sin(std::abs(diffPhi) / 2.0);
}

bool TrackIsoCalculatorModule::isStdChargedList()
{

  bool checkPList = false;
  unsigned short nSelectedDaughters = m_decaydescriptor.getSelectionNames().size();
  if (nSelectedDaughters == 0)
    checkPList = Const::chargedStableSet.find(abs(m_decaydescriptor.getMother()->getPDGCode())) != Const::invalidParticle;
  else
    checkPList = true;
  DecayDescriptor dd;
  bool checkPListReference = false;
  if (dd.init(m_pListReferenceName))
    checkPListReference = Const::chargedStableSet.find(abs(dd.getMother()->getPDGCode())) != Const::invalidParticle;

  return (checkPList and checkPListReference);
};

void TrackIsoCalculatorModule::printDistancesArr(const std::vector<std::vector<double>>& arr, int size_x, int size_y)
{

  auto logConfig = this->getLogConfig();

  if (logConfig.getLogLevel() == LogConfig::c_Debug && logConfig.getDebugLevel() >= 11) {
    std::cout << "" << std::endl;
    for (int i(0); i < size_x; ++i) {
      for (int j(0); j < size_y; ++j) {
        std::cout << std::setw(7) << arr[i][j] << " ";
      }
      std::cout << "\n";
    }
    std::cout << "" << std::endl;
  }
}

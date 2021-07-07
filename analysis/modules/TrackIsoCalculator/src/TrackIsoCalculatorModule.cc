/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/modules/TrackIsoCalculator/TrackIsoCalculatorModule.h>

#include <cmath>
#include <iomanip>



using namespace Belle2;

REG_MODULE(TrackIsoCalculator)

TrackIsoCalculatorModule::TrackIsoCalculatorModule() : Module()
{
  // Set module properties
  setDescription(R"DOC(Calculate track isolation variables on the input ParticleList.)DOC");

  // Parameter definitions
  addParam("particleList",
           m_pListName,
           "The name of the input ParticleList. Must be a charged stable particle as defined in Const::chargedStableSet.");
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
  m_pList.isRequired(m_pListName);

  B2INFO("TrackIsoCalculator module will calculate isolation variables for the ParticleList: " << m_pListName << ".");

  m_extraInfoName = (!m_use2DRhoPhiDist) ? ("dist3DToClosestTrkAt" + m_detInnerSurface + "Surface") : ("dist2DRhoPhiToClosestTrkAt" +
                    m_detInnerSurface + "Surface");
}

void TrackIsoCalculatorModule::event()
{

  if (!isStdChargedList()) {
    B2FATAL("PDG: " << m_pList->getPDGCode() << " of ParticleList: " << m_pList->getParticleListName() <<
            " is not that of a valid particle in Const::chargedStableSet! Aborting...");
  }

  const auto nParticles = m_pList->getListSize();

  B2DEBUG(11, "EVENT: " << m_event_metadata->getEvent() << "\n" << "nParticles: " << nParticles);

  // Store the pair-wise distances in a 2D array.
  // Size is given by the length of the particle list.
  std::vector<double> defaultDistances(nParticles, 1e9);
  std::vector<std::vector<double>> pairwiseDistances(nParticles, defaultDistances);

  B2DEBUG(11, "Array of pair-wise distances between tracks in particle list. Initial values:");
  this->printDistancesArr(pairwiseDistances, nParticles);

  for (unsigned int iPart(0); iPart < nParticles; ++iPart) {

    Particle* iParticle = m_pList->getParticle(iPart);

    for (unsigned int jPart(iPart + 1); jPart < nParticles; ++jPart) {

      Particle* jParticle = m_pList->getParticle(jPart);

      // Calculate the pair-wise distance.
      double ijDist = (!m_use2DRhoPhiDist) ? this->get3DDistAtDetSurface(iParticle,
                      jParticle) : this->get2DRhoPhiDistAsChordLength(iParticle, jParticle);

      pairwiseDistances[iPart][jPart] = ijDist;
      pairwiseDistances[jPart][iPart] = ijDist;

    }

  }

  B2DEBUG(11, "Array of pair-wise distances between tracks in particle list. Final values:");
  this->printDistancesArr(pairwiseDistances, nParticles);

  // For each particle index, find the index of the particle w/ minimal distance in the corresponding row of the 2D array.
  for (unsigned int iPart(0); iPart < nParticles; ++iPart) {

    auto minDist = std::min_element(std::begin(pairwiseDistances[iPart]), std::end(pairwiseDistances[iPart]));
    auto jPart = std::distance(std::begin(pairwiseDistances[iPart]), minDist);

    Particle* iParticle = m_pList->getParticle(iPart);

    B2DEBUG(10, m_extraInfoName << " = " << *minDist << " [cm] - Particle[" << iPart << "]'s closest partner at innermost " <<
            m_detInnerSurface << " surface is Particle[" << jPart << "]");

    if (!iParticle->hasExtraInfo(m_extraInfoName)) {
      B2DEBUG(10, "\tStoring extraInfo for Particle[" << iPart << "]...");
      iParticle->writeExtraInfo(m_extraInfoName, *minDist);
    }
  }

}

void TrackIsoCalculatorModule::terminate()
{
}

double TrackIsoCalculatorModule::get3DDistAtDetSurface(Particle* iParticle, Particle* jParticle)
{

  // Radius and z boundaries of the cylinder describing this detector's inner surface.
  const auto rho = m_detSurfBoundaries[m_detInnerSurface].m_rho;
  const auto zfwd = m_detSurfBoundaries[m_detInnerSurface].m_zfwd;
  const auto zbwd = m_detSurfBoundaries[m_detInnerSurface].m_zbwd;

  std::string nameExtTheta = "helixExtTheta(" + std::to_string(rho) + "," + std::to_string(zfwd) + "," + std::to_string(zbwd) + ")";
  const auto iExtTheta = Variable::Manager::Instance().getVariable(nameExtTheta)->function(iParticle);
  const auto jExtTheta = Variable::Manager::Instance().getVariable(nameExtTheta)->function(jParticle);

  std::string nameExtPhi = "helixExtPhi(" + std::to_string(rho) + "," + std::to_string(zfwd) + "," + std::to_string(zbwd) + ")";
  const auto iExtPhi = Variable::Manager::Instance().getVariable(nameExtPhi)->function(iParticle);
  const auto jExtPhi = Variable::Manager::Instance().getVariable(nameExtPhi)->function(jParticle);

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


double TrackIsoCalculatorModule::get2DRhoPhiDistAsChordLength(Particle* iParticle, Particle* jParticle)
{

  // Radius and z boundaries of the cylinder describing this detector's inner surface.
  const auto rho = m_detSurfBoundaries[m_detInnerSurface].m_rho;
  const auto zfwd = m_detSurfBoundaries[m_detInnerSurface].m_zfwd;
  const auto zbwd = m_detSurfBoundaries[m_detInnerSurface].m_zbwd;

  std::string nameExtPhi = "helixExtPhi(" + std::to_string(rho) + "," + std::to_string(zfwd) + "," + std::to_string(zbwd) + ")";
  const auto iExtPhi = Variable::Manager::Instance().getVariable(nameExtPhi)->function(iParticle);
  const auto jExtPhi = Variable::Manager::Instance().getVariable(nameExtPhi)->function(jParticle);

  auto diffPhi = jExtPhi - iExtPhi;
  if (std::abs(diffPhi) > M_PI) {
    diffPhi = (diffPhi > M_PI) ? diffPhi - 2 * M_PI :  2 * M_PI + diffPhi;
  }

  return 2 * rho * sin(std::abs(diffPhi) / 2.0);
}

void TrackIsoCalculatorModule::printDistancesArr(const std::vector<std::vector<double>>& arr, int size)
{

  auto logConfig = this->getLogConfig();

  if (logConfig.getLogLevel() == LogConfig::c_Debug && logConfig.getDebugLevel() >= 11) {
    std::cout << "" << std::endl;
    for (int i(0); i < size; ++i) {
      for (int j(0); j < size; ++j) {
        std::cout << std::setw(7) << arr[i][j] << " ";
      }
      std::cout << "\n";
    }
    std::cout << "" << std::endl;
  }
}

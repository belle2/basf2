/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/modules/TrackIsoCalculator/TrackIsoCalculatorModule.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
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
           m_pListName,
           "The name of the input ParticleList. Must be a charged stable particle as defined in Const::chargedStableSet.");
  addParam("particleListReference",
           m_pListReferenceName,
           "The name of the input ParticleList of reference tracks. Must be a charged stable particle as defined in Const::chargedStableSet.");
  addParam("detectorSurface",
           m_detSurface,
           "The name of the detector at whose inner (cylindrical) surface we extrapolate each helix's polar and azimuthal angle. Allowed values: {CDC, TOP, ARICH, ECL, KLM}.");
}

TrackIsoCalculatorModule::~TrackIsoCalculatorModule()
{
}

void TrackIsoCalculatorModule::initialize()
{

  m_event_metadata.isRequired();
  m_pList.isRequired(m_pListName);
  m_pListReference.isRequired(m_pListReferenceName);

  B2INFO("TrackIsoCalculator module will calculate isolation variables for the ParticleList: " << m_pListName << ", "
         << "Reference ParticleList: " << m_pListReferenceName << ".");

  if (!isStdChargedList()) {
    B2FATAL("ParticleList: " << m_pListName << " and/or ParticleList: " << m_pListReferenceName <<
            " is not that of a valid particle in Const::chargedStableSet! Aborting...");
  }

  // Define the name of the variable to be stored as extraInfo.
  m_extraInfoName = "distToClosestTrkAt" + m_detSurface + "_VS_" + m_pListReferenceName;

  m_isSurfaceInDet.insert({"CDC", m_detSurface.find("CDC") != std::string::npos});
  m_isSurfaceInDet.insert({"TOP", m_detSurface.find("TOP") != std::string::npos});
  m_isSurfaceInDet.insert({"ARICH", m_detSurface.find("ARICH") != std::string::npos});
  m_isSurfaceInDet.insert({"ECL", m_detSurface.find("ECL") != std::string::npos});
  m_isSurfaceInDet.insert({"KLM", m_detSurface.find("KLM") != std::string::npos});
}

void TrackIsoCalculatorModule::event()
{

  const auto nParticles = m_pList->getListSize();
  const auto nParticlesReference = m_pListReference->getListSize();

  B2DEBUG(11, "EVENT: " << m_event_metadata->getEvent() << "\n"
          << "Detector surface: " << m_detSurface << "\n"
          << "nParticles: " << nParticles << "\n"
          << "nParticlesReference: " << nParticlesReference);

  // Store the pair-wise distances in a 2D array.
  // Size is given by the length of the reference particle list.
  std::vector<double> defaultDistances(nParticlesReference, -1.0);
  // Size is given by the length of the particle list. Each vector (= defaultDistances) has nParticlesReference components.
  // Thus, total size is given by nParticles times nParticlesReference.
  std::vector<std::vector<double>> pairwiseDistances(nParticles, defaultDistances);

  B2DEBUG(11, "Array of pair-wise distances between tracks in particle list. Initial values:");
  this->printDistancesArr(pairwiseDistances, nParticles, nParticlesReference);

  for (unsigned int iPart(0); iPart < nParticles; ++iPart) {
    Particle* iParticle = m_pList->getParticle(iPart);

    for (unsigned int jPart(0); jPart < nParticlesReference; ++jPart) {
      Particle* jParticle = m_pListReference->getParticle(jPart);

      // Skip if same particle.
      if (iParticle->getMdstArrayIndex() == jParticle->getMdstArrayIndex()) {
        continue;
      }
      // Skip if this pair already calculated.
      if (pairwiseDistances[iPart][jPart] != -1.0) {
        continue;
      }

      // Calculate the pair-wise distance.
      const auto ijDist = this->getDistAtDetSurface(iParticle, jParticle);

      pairwiseDistances[iPart][jPart] = ijDist;

      int jPart_in_inputList = m_pList->getIndex(jParticle);
      if (jPart_in_inputList != -1) {
        pairwiseDistances[jPart_in_inputList][iPart] = ijDist;
      }

    }

  }

  B2DEBUG(11, "Array of pair-wise distances between tracks in particle list. Final values:");
  this->printDistancesArr(pairwiseDistances, nParticles, nParticlesReference);

  // For each particle index, find the index of the particle w/ minimal distance in the corresponding row of the 2D array.
  for (unsigned int iPart(0); iPart < nParticles; ++iPart) {

    // Remove any NaNs and dummy distances from the row to avoid spoiling the search for the minimum value.
    std::vector<double> iRow;
    for (const auto& d : pairwiseDistances[iPart]) {
      if (std::isnan(d) || d < 0) {
        continue;
      }
      iRow.push_back(d);
    }

    if (!iRow.size()) {
      continue;
    }

    auto minDist = std::min_element(std::begin(iRow), std::end(iRow));
    auto jPart = std::distance(std::begin(iRow), minDist);

    Particle* iParticle = m_pList->getParticle(iPart);
    Particle* jParticle = m_pListReference->getParticle(jPart);

    B2DEBUG(10, "Particle[" << iPart << "]'s (PDG=" << iParticle->getPDGCode() << ") closest partner at innermost " <<
            m_detSurface << " surface is ReferenceParticle[" << jPart << "] (PDG=" << jParticle->getPDGCode() << ") at D = " <<
            *minDist << " [cm]");

    if (!iParticle->hasExtraInfo(m_extraInfoName)) {
      B2DEBUG(12, "Storing extraInfo(" << m_extraInfoName << ") for Particle[" << iPart << "]");
      iParticle->writeExtraInfo(m_extraInfoName, *minDist);
    }
  }

}

void TrackIsoCalculatorModule::terminate()
{
}

double TrackIsoCalculatorModule::getDistAtDetSurface(Particle* iParticle, Particle* jParticle)
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
  const auto iExtTheta = std::get<double>(Variable::Manager::Instance().getVariable(nameExtTheta)->function(iParticle));
  const auto jExtTheta = std::get<double>(Variable::Manager::Instance().getVariable(nameExtTheta)->function(jParticle));

  std::string nameExtPhi = "helixExtPhi(" + std::to_string(rho) + "," + std::to_string(zfwd) + "," + std::to_string(zbwd) + ")";
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
    // extraplolated track helices cross the input detector's cylindrical surface
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

bool TrackIsoCalculatorModule::isStdChargedList()
{
  DecayDescriptor dd;

  bool checkPList = false;
  if (dd.init(m_pListName))
    checkPList = Const::chargedStableSet.find(abs(dd.getMother()->getPDGCode())) != Const::invalidParticle;

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

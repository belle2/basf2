/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <skim/modules/ECLClusterRounderModule.h>

#include <cmath>

using namespace Belle2;

REG_MODULE(ECLClusterRounder);

class Module;

ECLClusterRounderModule::ECLClusterRounderModule() : Module()
{
  setDescription("This module loads all ECLClusters present in each event and "
                 "rounds their Double32_t member variables to the precision they"
                 "would have if they were read from an mdst file. Useful if you want"
                 "to perform analysis/skimming in the same steering file that also runs"
                 "event generation/simulation/reconstruction.");
}

void ECLClusterRounderModule::initialize() { }

void ECLClusterRounderModule::event()
{
  B2DEBUG(20, "Rounding all Double32_t ECLCluster members to mdst precision...");
  const double pi = 3.141592653589793238462643383279502884;
  for (int i = 0; i < m_eclclusters.getEntries(); i++) {
    ECLCluster* cluster = m_eclclusters[i];

    TMatrixDSym covMat = cluster->getCovarianceMatrix3x3();
    double matRounded[6] = {
      covMat(0, 0),
      roundToPrecision(covMat(1, 0), 0.0, 10.0, 12),
      covMat(1, 1),
      roundToPrecision(covMat(2, 0), 0.0, 10.0, 12),
      roundToPrecision(covMat(2, 1), 0.0, 10.0, 12),
      covMat(2, 2)
    };
    cluster->setCovarianceMatrix(matRounded);
    // Set diagonal entries seperately to avoid sqrt after rounding
    cluster->setUncertaintyEnergy(roundToPrecision(cluster->getUncertaintyEnergy(), 0.0, 0.3, 10));
    cluster->setUncertaintyPhi(roundToPrecision(cluster->getUncertaintyPhi(), 0.0, 0.05, 8));
    cluster->setUncertaintyTheta(roundToPrecision(cluster->getUncertaintyTheta(), 0.0, 0.05, 8));

    cluster->setdeltaL(roundToPrecision(cluster->getDeltaL(), -250.0, 250.0, 10));
    cluster->setMinTrkDistance(roundToPrecision(cluster->getMinTrkDistance(), 0.0, 250.0, 10));
    cluster->setAbsZernike40(roundToPrecision(cluster->getAbsZernike40(), 0.0, 1.7, 10));
    cluster->setAbsZernike51(roundToPrecision(cluster->getAbsZernike51(), 0.0, 1.2, 10));
    cluster->setZernikeMVA(roundToPrecision(cluster->getZernikeMVA(), 0.0, 1.0, 10));
    cluster->setE1oE9(roundToPrecision(cluster->getE1oE9(), 0.0, 1.0, 10));
    cluster->setE9oE21(roundToPrecision(cluster->getE9oE21(), 0.0, 1.0, 10));
    cluster->setSecondMoment(roundToPrecision(cluster->getSecondMoment(), 0.0, 40.0, 10));
    cluster->setLAT(roundToPrecision(cluster->getLAT(), 0.0, 1.0, 10));
    cluster->setNumberOfCrystals(roundToPrecision(cluster->getNumberOfCrystals(), 0.0, 200.0, 10));
    cluster->setTime(roundToPrecision(cluster->getTime(), -1000.0, 1000.0, 12));
    cluster->setDeltaTime99(roundToPrecision(cluster->getDeltaTime99(), 0.0, 1000.0, 12));
    cluster->setTheta(roundToPrecision(cluster->getTheta(), 0.0, pi, 16));
    cluster->setPhi(roundToPrecision(cluster->getPhi(), -pi, pi, 16));
    cluster->setR(roundToPrecision(cluster->getR(), 75.0, 300.0, 16));
    cluster->setPulseShapeDiscriminationMVA(roundToPrecision(cluster->getPulseShapeDiscriminationMVA(), 0.0, 1.0, 18));
    cluster->setNumberOfHadronDigits(roundToPrecision(cluster->getNumberOfHadronDigits(), 0.0, 255.0, 18));

    // Behavior of getEnergy depends on cluster hypothesis
    if (!cluster->hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons)) {
      cluster->addHypothesis(ECLCluster::EHypothesisBit::c_nPhotons);
      cluster->setLogEnergy(roundToPrecision(log(cluster->getEnergy(ECLCluster::EHypothesisBit::c_nPhotons)), -5.0, 3.0, 18));
      cluster->removeHypothesis(ECLCluster::EHypothesisBit::c_nPhotons);
    } else {
      cluster->setLogEnergy(roundToPrecision(log(cluster->getEnergy(ECLCluster::EHypothesisBit::c_nPhotons)), -5.0, 3.0, 18));
    }
    cluster->setLogEnergyRaw(roundToPrecision(log(cluster->getEnergyRaw()), -5.0, 3.0, 18));
    cluster->setLogEnergyHighestCrystal(roundToPrecision(log(cluster->getEnergyHighestCrystal()), -5.0, 3.0, 18));
  }
}

double ECLClusterRounderModule::roundToPrecision(Double32_t value, double min, double max, int nBits)
{
  if (min > max) {
    B2FATAL("max value must be bigger than min value for ECLCluster Double32_t rounding");
  }
  if (nBits <= 0) {
    B2FATAL("the number of mantissa bits for ECLCluster Double32_t rounding must be positive");
  }
  const double range = max - min;
  const double scale = (1 << nBits) / range;
  return min + std::lround((std::clamp(value, min, max) - min) * scale) / scale;
}
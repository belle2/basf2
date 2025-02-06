/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/dbobjects/ECLPhotonEnergyResolution.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

ECLPhotonEnergyResolution::ECLPhotonEnergyResolution()
{
  /**
   * Define out of range resolution value as -1
   */
  WeightInfo energyResolutionInfo = {
    {"RelativeEnergyResolution", -1.},
    {"RelativeEnergyResolutionUncertUp", -1.},
    {"RelativeEnergyResolutionUncertDown", -1.}
  };

  m_resolutionBinningTable.defineOutOfRangeWeight(energyResolutionInfo);
}

void ECLPhotonEnergyResolution::addRelativeEnergyResolution(std::vector<double> energyResolution, Binning binning)
{
  /**
   * Build energy resolution with uncertainties as info map
   */
  WeightInfo energyResolutionInfo = {
    {"RelativeEnergyResolution", energyResolution.at(0)},
    {"RelativeEnergyResolutionUncertUp", energyResolution.at(1)},
    {"RelativeEnergyResolutionUncertDown", energyResolution.at(2)}
  };

  /**
   * Extract energy, theta and phi bin values
   */
  ParticleWeightingBinLimits* energyBinValues = new ParticleWeightingBinLimits(binning.at(0).first, binning.at(0).second);
  ParticleWeightingBinLimits* thetaBinValues = new ParticleWeightingBinLimits(binning.at(1).first, binning.at(1).second);
  ParticleWeightingBinLimits* phiBinValues = new ParticleWeightingBinLimits(binning.at(2).first, binning.at(2).second);

  NDBin energyBinning = {{"Energy", energyBinValues}, {"Theta", thetaBinValues}, {"Phi", phiBinValues}};

  /**
   * Add resolution value to binning table
   */
  m_resolutionBinningTable.addEntry(energyResolutionInfo, energyBinning);
}

double ECLPhotonEnergyResolution::getRelativeEnergyResolution(double energy, double theta, double phi) const
{
  /**
   * Construct energy, theta, phi map corresponding to binning structure
   */
  std::map<std::string, double> binning = {{"Energy", energy}, {"Theta", theta}, {"Phi", phi}};

  /**
   * Extract energy resolution information
   */
  WeightInfo energyResolutionInfo = m_resolutionBinningTable.getInfo(binning);

  if (energyResolutionInfo.at("RelativeEnergyResolution") == -1.) {
    B2DEBUG(20, "Energy resolution was returned as -1 since provided energy was not within energy resolution binning.");
  }

  return energyResolutionInfo.at("RelativeEnergyResolution");
}

void ECLPhotonEnergyResolution::addThetaPhiResolution(std::vector<double> thetaResolution, std::vector<double> phiResolution,
                                                      Binning binning)
{
  /**
   * Build energy resolution with uncertainties as info map
   */
  WeightInfo resolutionInfo = {
    {"ThetaResolution", thetaResolution.at(0)},
    {"ThetaResolutionUncertUp", thetaResolution.at(1)},
    {"ThetaResolutionUncertDown", thetaResolution.at(2)},
    {"PhiResolution", thetaResolution.at(0)},
    {"PhiResolutionUncertUp", thetaResolution.at(1)},
    {"PhiResolutionUncertDown", thetaResolution.at(2)}
  };

  /**
   * Extract energy, theta and phi bin values
   */
  ParticleWeightingBinLimits* energyBinValues = new ParticleWeightingBinLimits(binning.at(0).first, binning.at(0).second);
  ParticleWeightingBinLimits* thetaBinValues = new ParticleWeightingBinLimits(binning.at(1).first, binning.at(1).second);
  ParticleWeightingBinLimits* phiBinValues = new ParticleWeightingBinLimits(binning.at(2).first, binning.at(2).second);

  NDBin energyBinning = {{"Energy", energyBinValues}, {"Theta", thetaBinValues}, {"Phi", phiBinValues}};

  /**
   * Add resolution value to binning table
   */
  m_resolutionBinningTable.addEntry(resolutionInfo, energyBinning);
}

double ECLPhotonEnergyResolution::getThetaPhiResolution(double energy, double theta, double phi, bool thetaOrPhi) const
{
  /**
   * Construct energy, theta, phi map corresponding to binning structure
   */
  std::map<std::string, double> binning = {{"Energy", energy}, {"Theta", theta}, {"Phi", phi}};

  /**
   * Extract resolution information
   */
  WeightInfo resolutionInfo = m_resolutionBinningTable.getInfo(binning);

  if (thetaOrPhi) {
    if (resolutionInfo.at("PhiResolution") == -1.) {
      B2DEBUG(20, "Phi resolution was returned as -1 since provided energy was not within theta resolution binning.");
    }
    return resolutionInfo.at("PhiResolution");
  } else {
    if (resolutionInfo.at("ThetaResolution") == -1.) {
      B2DEBUG(20, "Theta resolution was returned as -1 since provided energy was not within theta resolution binning.");
    }
    return resolutionInfo.at("ThetaResolution");
  }
}

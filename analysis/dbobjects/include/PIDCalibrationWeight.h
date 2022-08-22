/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include <boost/algorithm/string.hpp>

#include <TObject.h>

namespace Belle2 {

  /**
   * PID calibration weight matrix, 6 (particle type) x 6 (detectors).
   * The particle types are sorted by their invariant mass (e, mu, pi, K, p, d), which is inherited from Const::chargedStableSet.
   * Each vector<double> has 6 weights for different sub-detectors for a particle type.
   * The order is as follows, [SVD, CDC, TOP, ARICH, ECL. KLM] that is inherited from Const::PIDDetectors.
   */
  typedef std::vector<std::vector<double>> WeightMatrix;

  /**
   * Class for handling the PID calibration weight matrix
   */
  class PIDCalibrationWeight : public TObject {

  public:
    /// Constructor
    PIDCalibrationWeight() {};

    /// Constructor with the initial WeightMatrix
    PIDCalibrationWeight(WeightMatrix weightMatrix)
    {
      m_weightMatrix = weightMatrix;
    };

    /**
     * Set the WeightMatrix.
     * @param weightMatrix is the weight matrix to be set.
     */
    void setWeightMatrix(WeightMatrix weightMatrix)
    {
      m_weightMatrix = weightMatrix;
    };

    /**
     * Get the WeightMatrix.
     */
    WeightMatrix getWeightMatrix() const
    {
      return m_weightMatrix;
    };

    /**
     * Get the weight for the given combination of the PDG code and the detector name.
     * @param pdg is the PDG code
     * @param detector is the detector name
     * @return weight for the given combination of the PDG code and the detector name
     */
    double getWeight(int pdg, std::string detector) const
    {
      std::vector<double> weightVector = getWeights(pdg);

      Const::EDetector det;
      boost::to_lower(detector);
      if (detector == "svd") det = Const::SVD;
      else if (detector == "cdc") det = Const::CDC;
      else if (detector == "top") det = Const::TOP;
      else if (detector == "arich") det = Const::ARICH;
      else if (detector == "ecl") det = Const::ECL;
      else if (detector == "klm") det = Const::KLM;
      else B2FATAL("Unknown detector component: " << detector);
      int det_index = Const::PIDDetectors::c_set.getIndex(det);

      return weightVector[det_index];
    };

    /**
     * Get the weight for the given combination of the PDG code and the detector in Const::EDetector.
     * @param pdg is the PDG code
     * @param det is the detector in Const::EDetector
     * @return weight for the given set of the PDG code and the detector in Const::EDetector
     */
    double getWeight(int pdg, Const::EDetector det) const
    {
      std::vector<double> weightVector = getWeights(pdg);

      int det_index = Const::PIDDetectors::c_set.getIndex(det);
      return weightVector[det_index];
    };

    /**
     * Get the weights for the given PDG code
     * @param pdg is the PDG code
     * @return weights for the given PDG code
     */
    std::vector<double> getWeights(int pdg) const
    {
      int p_index = -1;
      for (const auto& pdgIter : Const::chargedStableSet) {
        if (pdgIter.getPDGCode() == pdg) {
          p_index = pdgIter.getIndex();
          break;
        }
      }
      if (p_index == -1)
        B2FATAL("Invalid particle: " << pdg);

      return m_weightMatrix[p_index];
    };

  private:

    /**
     * PID calibration weight matrix.
     */
    WeightMatrix m_weightMatrix;

    ClassDef(PIDCalibrationWeight, 1); /**< ClassDef as this is a TObject */

  };

} // Belle2 namespace



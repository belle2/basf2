/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TObject.h>

namespace Belle2 {

  /**
   * Class for handling the parameters for the neural-network PID.
   */
  class BellePi0EtaProbParameters: public TObject {

  public:
    /// Constructor
    BellePi0EtaProbParameters() {}

    /**
     * Construct with individual neural-network parameters
     */
    BellePi0EtaProbParameters(const std::vector<double>& pi0ProbFWD,
                              const std::vector<double>& pi0ProbBRL,
                              const std::vector<double>& pi0ProbBWD,
                              const std::vector<double>& etaProbFWD,
                              const std::vector<double>& etaProbBRL,
                              const std::vector<double>& etaProbBWD
                             ):
      m_pi0ProbFWD(pi0ProbFWD),
      m_pi0ProbBRL(pi0ProbBRL),
      m_pi0ProbBWD(pi0ProbBWD),
      m_etaProbFWD(etaProbFWD),
      m_etaProbBRL(etaProbBRL),
      m_etaProbBWD(etaProbBWD)
    {}

    /**
     * Get pi0 probability of forward end cap
     * @param index array index
     * @return double pi0 probability at given index in forward end cap
     */
    double getBelleFWDPi0Probability(int index) const {return m_pi0ProbFWD.at(index);}

    /**
     * Get pi0 probability of barrel
     * @param index array index
     * @return double pi0 probability at given index in barrel
     */
    double getBelleBRLPi0Probability(int index) const {return m_pi0ProbBRL.at(index);}

    /**
     * Get pi0 probability of backward end cap
     * @param index array index
     * @return double pi0 probability at given index in backward end cap
     */
    double getBelleBWDPi0Probability(int index) const {return m_pi0ProbBWD.at(index);}

    /**
     * Get eta probability of forward end cap
     * @param index array index
     * @return double eta probability at given index in forward end cap
     */
    double getBelleFWDEtaProbability(int index) const {return m_etaProbFWD.at(index);}

    /**
     * Get eta probability of barrel
     * @param index array index
     * @return double eta probability at given index in barrel
     */
    double getBelleBRLEtaProbability(int index) const {return m_etaProbBRL.at(index);}

    /**
     * Get eta probability of backward end cap
     * @param index array index
     * @return double eta probability at given index in backward end cap
     */
    double getBelleBWDEtaProbability(int index) const {return m_etaProbBWD.at(index);}

  private:

    std::vector<double> m_pi0ProbFWD; /**< vector of pi0 probabilities in forward end cap */
    std::vector<double> m_pi0ProbBRL; /**< vector of pi0 probabilities in barrel */
    std::vector<double> m_pi0ProbBWD; /**< vector of pi0 probabilities in backward end cap */
    std::vector<double> m_etaProbFWD; /**< vector of eta probabilities in forward end cap */
    std::vector<double> m_etaProbBRL; /**< vector of eta probabilities in barrel */
    std::vector<double> m_etaProbBWD; /**< vector of eta probabilities in backward end cap */

    ClassDef(BellePi0EtaProbParameters, 1); /**< ClassDef as this is a TObject */

  };

} // Belle2 namespace



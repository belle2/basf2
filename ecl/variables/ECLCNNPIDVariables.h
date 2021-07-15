/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

namespace Belle2 {
  class Particle;

  namespace Variable {

    /**
     * return cnn_pid_ecl_pion of CNNPIDECLPion
     */
    double CNNPIDECLPion(const Particle* part);

    /**
     * return cnn_pid_ecl_muon of CNNPIDECLMuon
     */
    double CNNPIDECLMuon(const Particle* part);

  }
} // Belle2 namespace

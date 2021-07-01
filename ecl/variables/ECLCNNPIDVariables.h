/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Abtin Narimani Charan                                    *
 * Email: abtin.narimani.charan@desy.de                                   *
 * This software is provided "as is" without any warranty.                *
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

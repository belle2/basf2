/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <ecl/variables/ECLCNNPIDVariables.h>

#include <analysis/VariableManager/Manager.h>
#include <analysis/dataobjects/Particle.h>

#include <framework/logging/Logger.h>

using namespace std;

namespace Belle2 {

  namespace Variable {

    // CNNPIDECLPion -------------------------------------------
    double CNNPIDECLPion(const Particle* part)
    {
      if (part->hasExtraInfo("cnn_pid_ecl_pion")) return part->getExtraInfo("cnn_pid_ecl_pion");
      else {
        B2WARNING("The ExtraInfo 'cnn_pid_ecl_pion' not found!");
        return std::numeric_limits<double>::quiet_NaN();
      }
    }

    // CNNPIDECLPion -------------------------------------------
    double CNNPIDECLMuon(const Particle* part)
    {
      if (part->hasExtraInfo("cnn_pid_ecl_muon")) return part->getExtraInfo("cnn_pid_ecl_muon");
      else {
        B2WARNING("The ExtraInfo 'cnn_pid_ecl_muon' not found!");
        return std::numeric_limits<double>::quiet_NaN();
      }
    }

    VARIABLE_GROUP("CNN PID ECL variable (cDST)");

    REGISTER_VARIABLE("cnn_pid_ecl_pion", CNNPIDECLPion,
                      R"DOC(CNN runs over extrapolated tracks and output probabilities of pion or muon like. The variable here is pion-like probablity.
Returns NaN if CNN was not run or if the ``cnn_pid_ecl_pion`` parameter was not set.)DOC");

    REGISTER_VARIABLE("cnn_pid_ecl_muon", CNNPIDECLMuon,
                      R"DOC(CNN runs over extrapolated tracks and output probabilities of pion or muon like. The variable here is muon-like probablity.
Returns NaN if CNN was not run or if the ``cnn_pid_ecl_muon`` parameter was not set.)DOC");

  }
}

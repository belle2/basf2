/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <ecl/variables/ECLCNNPIDVariables.h>
#include <ecl/dataobjects/ECLCNNPid.h>

#include <analysis/VariableManager/Manager.h>
#include <analysis/dataobjects/Particle.h>

#include <mdst/dataobjects/Track.h>

#include <framework/logging/Logger.h>

using namespace std;

namespace Belle2 {

  namespace Variable {

    // CNNPIDECLPion -------------------------------------------
    double CNNPIDECLMuon(const Particle* part)
    {
      auto track = part->getTrack();
      if (!track) return std::numeric_limits<double>::quiet_NaN();

      const auto eclCnnMuonRelationVector = track->getRelationsWith<ECLCNNPid>();
      if (eclCnnMuonRelationVector.size() == 0) return std::numeric_limits<double>::quiet_NaN();

      if (eclCnnMuonRelationVector.size() == 1) {
        return eclCnnMuonRelationVector.object(0)->getEclCnnMuon();
      } else {
        B2FATAL("Somehow found more than 1 ECL CNN muon probabilities matched to the extrapolated track. This should not be possible!");
        return std::numeric_limits<double>::quiet_NaN();
      }
    }

    VARIABLE_GROUP("CNN PID ECL variable (cDST)");

    REGISTER_VARIABLE("cnn_pid_ecl_muon", CNNPIDECLMuon,
                      R"DOC(CNN runs over extrapolated tracks and output probabilities of pion or muon like. The variable here is muon-like probablity.
Returns NaN if CNN was not run or if the ``cnn_pid_ecl_muon`` parameter was not set.)DOC");

  }
}

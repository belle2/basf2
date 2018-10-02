/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// analysis
#include <analysis/VariableManager/Manager.h>

// framework
#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreArray.h>

// dataobjects
#include <tracking/dataobjects/ExtHit.h>
#include <mdst/dataobjects/Track.h>
#include <analysis/dataobjects/Particle.h>
#include <top/dataobjects/TOPDigit.h>

using namespace std;

namespace Belle2 {
  namespace TOPDigitVariables {
  }
  namespace Variable {
    double getTOPModuleDigitCount(const Particle* particle)
    {
      auto trk = particle->getTrack();
      // auto tr = trk->getTrackFitResult(Const::ChargedStable(particle->getPDGCode()));
      auto extHits = trk->getRelationsWith<ExtHit>();
      int thisModuleID = 77; // default to sentinel value
      for (auto h : extHits) {
        if (h.getDetectorID() != 4) continue; // 4 == iTOP, see Const::EDetectors
        if (h.getStatus() != 0) continue; // 0 == EXT_ENTER
        // now find the module of this hit.
        thisModuleID = h.getCopyID(); // could be positive or negative
        break;
      }
      StoreArray<TOPDigit> topDigits;
      int count = 0;
      for (auto t : topDigits) {
        if (t.getModuleID() == thisModuleID) {
          count += 1;
        }
      }
      return count;
      // auto tr0_loglInfo = loglInfo(t.getRelationsWith("TOPLikelihoods"));
      // auto tr0_p = tr_p3.X(), tr_p3.Y(), tr_p3.Z();
    }


    //     def extHitInfo(extHitList):
    // for h in extHitList:
    //     if h.getDetectorID() != 4: # 4 == iTOP, see Const::EDetectors
    //         continue
    //     if h.getStatus() != 0: # 0 == EXT_ENTER
    //         continue
    //     # now find the module of this hit.
    //     thisModuleID = h.getCopyID()
    //     pos = h.getPosition()
    //     return thisModuleID, pos.X(), pos.Y(), pos.Z(), h.getTOF()
    // return (0, 0, 0, 0, 0)

    VARIABLE_GROUP("TOP Calibration");
    REGISTER_VARIABLE("topModuleDigitCount", getTOPModuleDigitCount,
                      "[calibration] Returns the number of TOPDigits in the module to which the track was extrapolated");
    // REGISTER_VARIABLE("topModuleTrackCount", getTOPModuleTrackCount,
    //                   "[calibration] Returns the number of particles (including this particle) that hit the same module");
    // REGISTER_VARIABLE("topModuleT0", getTOPModuleT0,
    //                   "[calibration] Returns the T0 for the module to which the particle was extrapolated");
    // REGISTER_VARIABLE("topModuleExtHit_x", getTOPModuleExtHit_x,
    //                   "[calibration] Returns the x component of the ExtHit");
    // REGISTER_VARIABLE("topModuleExtHit_y", getTOPModuleExtHit_y,
    //                   "[calibration] Returns the y component of the ExtHit");
    // REGISTER_VARIABLE("topModuleExtHit_z", getTOPModuleExtHit_z,
    //                   "[calibration] Returns the z component of the ExtHit");
  }
// Create an empty module which allows basf2 to easily find the library and load it from the steering file
  class EnableTOPDigitVariablesModule: public Module {}; // Register this module to create a .map lookup file.
  REG_MODULE(EnableTOPDigitVariables);
}

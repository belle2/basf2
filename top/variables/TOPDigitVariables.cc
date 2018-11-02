/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Jan Strube (jan.strube@desy.de)                           *
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

#include <algorithm> // for sort
using namespace std;

namespace Belle2 {
  namespace topDigitVariables {
  }
  namespace Variable {
    //! @returns the number of digits in the same module as the particle
    double topDigitCount(const Particle* particle)
    {
      auto trk = particle->getTrack();
      if (not trk) {
        return -1.0;
      }
      auto extHits = trk->getRelationsWith<ExtHit>();
      int thisModuleID = 77; // default to sentinel value
      for (auto h : extHits) {
        if (h.getDetectorID() != Const::EDetector::TOP) continue;
        if (h.getStatus() != EXT_ENTER) continue;
        // now find the module of this hit.
        thisModuleID = h.getCopyID(); // could be positive or negative
        break;
      }
      if (thisModuleID == 77) {
        return -1;
      }

      StoreArray<TOPDigit> topDigits;
      int count = 0;
      for (auto t : topDigits) {
        if (abs(t.getModuleID()) == abs(thisModuleID)) { // catch the case where one of the module IDs is negative
          count += 1;
        }
      }
      return count;
    }

    //! @returns the largest time between to subsequent digits in the same module as the particle
    double topDigitGapSize(const Particle* particle)
    {
      auto trk = particle->getTrack();
      if (not trk) {
        return -1.0;
      }
      auto extHits = trk->getRelationsWith<ExtHit>();
      int thisModuleID = 77; // default to sentinel value
      for (auto h : extHits) {
        if (h.getDetectorID() != 4) continue; // 4 == iTOP, see Const::EDetectors
        if (h.getStatus() != 0) continue; // 0 == EXT_ENTER
        // now find the module of this hit.
        thisModuleID = h.getCopyID(); // could be positive or negative
        break;
      }
      if (thisModuleID == 77) {
        return -1.0;
      }

      StoreArray<TOPDigit> topDigits;
      double maxGap = 0; // the largest time difference between two consecutive hits
      vector<double> digitTimes; // all digits in the module that the track entered
      for (auto t : topDigits) {
        if (abs(t.getModuleID()) != abs(thisModuleID)) continue;
        digitTimes.push_back(t.getTime());
      }
      if (digitTimes.size() == 0) {
        return -1.0;
      }
      sort(digitTimes.begin(), digitTimes.end());
      for (size_t i = 0; i < digitTimes.size() - 1; ++i) {
        double gap = digitTimes[i + 1] - digitTimes[i];
        if (gap > maxGap) {
          maxGap = gap;
        }
      }
      return maxGap;
    }

    // The number of reflected digits is defined as the number of digits after the gap
    //! @returns the number of reflected digits in the same module as the particle
    double topReflectedDigitCount(const Particle* particle)
    {
      auto trk = particle->getTrack();
      if (not trk) {
        return -1.0;
      }
      auto extHits = trk->getRelationsWith<ExtHit>();
      int thisModuleID = 77; // default to sentinel value
      for (auto h : extHits) {
        if (h.getDetectorID() != 4) continue; // 4 == iTOP, see Const::EDetectors
        if (h.getStatus() != 0) continue; // 0 == EXT_ENTER
        // now find the module of this hit.
        thisModuleID = h.getCopyID(); // could be positive or negative
        break;
      }
      if (thisModuleID == 77) {
        return -1.0;
      }

      StoreArray<TOPDigit> topDigits;
      double maxGap = 0; // the largest time difference between two consecutive hits
      size_t maxGapIndex = 0; // the index of the first hit *after* the gap
      vector<double> digitTimes; // all digits in the module that the track entered
      for (auto t : topDigits) {
        if (abs(t.getModuleID()) != abs(thisModuleID)) continue;
        digitTimes.push_back(t.getTime());
      }
      if (digitTimes.size() == 0) {
        return -1.0;
      }
      sort(digitTimes.begin(), digitTimes.end());
      for (size_t i = 0; i < digitTimes.size() - 1; ++i) {
        double gap = digitTimes[i + 1] - digitTimes[i];
        if (gap > maxGap) {
          maxGap = gap;
          maxGapIndex = i + 1;
        }
      }
      return digitTimes.size() - maxGapIndex;
    }

    VARIABLE_GROUP("TOP Calibration");
    REGISTER_VARIABLE("topDigitCount", topDigitCount,
                      "[calibration] Returns the number of TOPDigits in the module to which the track was extrapolated");
    REGISTER_VARIABLE("topReflectedDigitCount", topReflectedDigitCount,
                      "[calibration] Returns the number of reflected photons in the same module");
    REGISTER_VARIABLE("topDigitGapSize", topDigitGapSize,
                      "[calibration] Returns the largest time difference between two consecutive hits in the same module");
  }
// Create an empty module which allows basf2 to easily find the library and load it from the steering file
  class EnableTOPDigitVariablesModule: public Module {}; // Register this module to create a .map lookup file.
  REG_MODULE(EnableTOPDigitVariables);
}

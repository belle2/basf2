/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/calibration/ValidationTreeStruct.h>
#include <framework/logging/Logger.h>

namespace Belle2 {
  namespace TOP {

    void ValidationTreeStruct::clear()
    {
      expNo = 0;
      runNo = 0;
      numTracks = 0;
      commonT0 = 0;
      commonT0Err = 0;
      for (auto& x : moduleT0) x = 0;
      for (auto& x : moduleT0Err) x = 0;
      for (auto& x : numTBCalibrated) x = 0;
      for (auto& x : numT0Calibrated) x = 0;
      for (auto& x : numActive) x = 0;
      for (auto& x : numActiveCalibrated) x = 0;
      for (auto& x : thrEffi) x = 0;
      for (auto& x : asicShifts) x = 0;
      svdOffset = 0;
      svdSigma = 0;
      cdcOffset = 0;
      cdcSigma = 0;
      fillPatternOffset = 0;
      numMerged = 0;
    }

    void ValidationTreeStruct::merge(const ValidationTreeStruct& other)
    {
      if (numMerged == 0) {
        *this = other;
      } else {
        if (expNo != other.expNo or runNo != other.runNo) {
          B2ERROR("TOP::ValidationTreeStruct: experiment and run numbers must be equal - objects not merged");
          return;
        }
        numTracks += other.numTracks;
        add(commonT0, commonT0Err, other.commonT0, other.commonT0Err);
        for (unsigned i = 0; i < c_numModules; i++) {
          add(moduleT0[i], moduleT0Err[i], other.moduleT0[i], other.moduleT0Err[i]);
        }
      }
      numMerged++;
    }

    void ValidationTreeStruct::rescaleErrors(double scaleFactor)
    {
      commonT0Err *= scaleFactor;
      for (auto& x : moduleT0Err) x *= scaleFactor;
    }

  } // end namespace TOP
} // end namespace Belle2



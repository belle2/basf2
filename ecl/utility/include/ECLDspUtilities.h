/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sergei Gribanov, Mikhail Remnev                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vector>
#include <ecl/utility/ECLDspEmulator.h>

namespace Belle2 {
  class ECLDigit;
  class ECLDspData;

  namespace ECL {

    /**
     * This class contains static methods to make them
     * accessible from pyROOT.
     */
    class ECLDspUtilities {
    public:
      /**
       * @brief Convert ECLDspData from *.dat file to Root object.
       *
       * @param[in] raw_file Path to dsp??.dat file.
       * @return ECLDspData object
       */
      static ECLDspData* readEclDsp(const char* raw_file, int boardNumber);

      /**
       * @brief Convert ECLDspData from Root object to *.dat file.
       *
       * @param[in] raw_file Path to dsp??.dat file to be created.
       * @param[in] obj      Object to be written
       */
      static void writeEclDsp(const char* raw_file, ECLDspData* obj);

      /**
       * @brief Emulate shape fitting algorithm from ShaperDSP
       *        using algorithm from ecl/utility/src/ECLDspEmulator.cc
       *        See ecl/examples/eclShapeFitter.py for usage example.
       *
       * @param[in] cid      CellID, 1..8736
       * @param[in] adc[31]  Waveform data from ECLDsp dataobject
       * @param[in] ttrig    Trigger time from ECLTrig dataobject
       * @param[in] adjusted_timing Optional.
       *              Use adjusted formula to determine fit time.
       *              Implemented in ShaperDSP firmware since exp 14.
       *              If true, algorithm will determine time near 0 with higher
       *              precision, time of low-energy hits will be one of {-4,0,4}
       *              If false, time will be one of {-32, -16, 0}
       */
      static ECLShapeFit shapeFitter(int cid, std::vector<int> adc, int ttrig,
                                     bool adjusted_timing = true);

    private:
      /**
       * Private constructor since class only contains static methods,
       * no need to create an instance.
       */
      ECLDspUtilities() {}
    };
  }
}

